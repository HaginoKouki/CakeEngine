#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <cassert>
#include <chrono>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <deque>
#include <functional>
#include <utility>

#include <exception>
#include <memory>
#include <optional>
#include <string>

#include <unordered_set>
#include <algorithm>

namespace Cake {

// 作業の優先度。ワーカーは High → Normal → Low の順に取り出す.
enum class JobPriority : uint8_t {
	High,
	Normal,
	Low,
};
// 優先度の数（キューの本数）.
inline constexpr size_t kJobPriorityCount = 3;

// 作業の結果。成功なら value に戻り値が入り、失敗なら error に理由が入る.
template <typename T>
struct JobResult {
	std::optional<T> value;
	std::string error;

	// value に値が入っていれば true を返す.
	bool Succeeded() const { return value.has_value(); }
};

// Submit で積んだ作業を指す番号。WaitFor に渡す.
struct JobHandle {
	uint64_t id = 0; // 0 は「受け付けなかった」.

	bool IsValid() const { return id != 0; }
};

class JobSystem {
private:
	// 作業と、その完了処理の組.
	struct Job {
		uint64_t id = 0;                // 作業の番号.
		std::function<void()> work;     // ワーカーで動く.
		std::function<void()> complete; // メインスレッドで動く.
	};

	// 完了のキューに積むもの。どの作業の完了処理かを番号で持つ.
	struct Completion {
		uint64_t id = 0;
		std::function<void()> complete;
	};

private:
	// ワーカーのスレッド。メインスレッドは含まない.
	std::vector<std::thread> workers_;
	// Initialize を呼んだスレッド（メインスレッド）の番号.
	std::thread::id mainThreadId_;

	// 作業キューと完了処理キューを守るカギ.
	std::mutex mutex_;
	// 作業が積まれた、または止める知らせ.
	std::condition_variable workAvailable_;
	// 作業が1つ終わった知らせ。WaitFor が待つ.
	std::condition_variable jobFinished_;
	// Shutdown が呼ばれたら true になる.
	bool stopping_ = false;

	// 優先度ごとの、まだ実行していない作業。mutex_ を取ってから触る.
	std::array<std::deque<Job>, kJobPriorityCount> queues_;
	// 作業が終わって、メインスレッドで実行するのを待っている完了処理。mutex_ を取ってから触る.
	std::deque<Completion> completions_;

	// 受け付けてから、完了処理が終わるまでの作業の番号。mutex_ を取ってから触る.
	std::unordered_set<uint64_t> live_;
	// 次に付ける番号。mutex_ を取ってから触る.
	uint64_t nextId_ = 1;

public:
	JobSystem() = default;
	JobSystem(const JobSystem&) = delete;
	JobSystem& operator=(const JobSystem&) = delete;
	~JobSystem();

	// ワーカーを workCount 本起動する.
	// 0のときは ComputeWorkerCount によって決定される.
	void Initialize(uint32_t workerCount = 0);

	// ワーカーに止まるよう通知し、全員をjoinする.
	void Shutdown();

	/// <summary>
	/// 作業と完了処理をキューに積む.
	/// </summary>
	/// <param name="work">ワーカーで実行される作業</param>
	/// <param name="complete">PumpMainThread の中で実行される完了処理</param>
	JobHandle Submit(JobPriority priority, std::function<void()> work, std::function<void()> complete);
	/// <summary>
	/// 結果を返す作業をキューに積む。戻り値は complete の引数に入るため、完了処理で受け取れる.
	/// </summary>
	/// <param name="work">ワーカーで実行される作業</param>
	/// <param name="complete">引数の <code>JobResult&lt;T&gt;</code> には work の結果の JobResult が渡される</param>
	template <typename T>
	JobHandle Submit(JobPriority priority, std::function<T()> work, std::function<void(JobResult<T>)> complete);

	// handle の作業の完了処理が終わるまで待つ。メインスレッド専用.
	// 待つ間は、メインスレッドもキューの作業を手伝う。ほかの作業の完了処理は実行しない.
	// エディタの道具と、起動時に必要なアセット用。ゲームのフレーム中には使わない.
	void WaitFor(JobHandle handle);

	// 溜まっている完了処理を実行する。メインスレッドから毎フレーム1回呼ぶ.
	// 使った時間が budget を超えたら、残りは次のフレームに回す。ただし最低1つは実行する.
	void PumpMainThread(std::chrono::microseconds budget);

	/// <summary>
	/// 引数が1以下なら1を返す。2以上なら hardwareConcurrency-1 を返す。
	/// </summary>
	/// <returns>ワーカーの本数（メインスレッドを除く）</returns>
	static uint32_t ComputeWorkerCount(uint32_t hardwareConcurrency) { return hardwareConcurrency > 1 ? hardwareConcurrency - 1 : 1; }

private:
	// ワーカーのメイン関数。index は 0 から始まるワーカー番号.
	void WorkerMain(uint32_t index);

	// 作業を１つ実行し、完了処理を completions_ に積む.
	// カギを持たずに呼ぶこと.
	void RunJob(Job& job);

	// どれかのキューに作業があるか。mutex_ を取ってから呼ぶこと.
	bool HasQueuedLocked() const;
	// 優先度の高いキューから作業を1つ取り出す。mutex_ を取ってから、作業があるときだけ呼ぶこと.
	Job PopLocked();
};

template <typename T>
JobHandle JobSystem::Submit(JobPriority priority, std::function<T()> work, std::function<void(JobResult<T>)> complete) {
	// 結果の箱を作る.
	auto result = std::make_shared<JobResult<T>>();

	auto workLambda = [work = std::move(work), result]() {
		try {
			// 作業を実行し、戻り値を result->value に入れる.
			result->value = work();
		} catch (const std::exception& e) {
			result->error = e.what();
		} catch (...) {
			result->error = "不明な例外";
		}
	};
	auto completeLambda = [complete = std::move(complete), result]() {
		// 完了処理を呼ぶときに result を渡す.
		complete(std::move(*result));
	};

	// 作業を積む.
	return Submit(priority, std::move(workLambda), std::move(completeLambda));
}

} // namespace Cake
