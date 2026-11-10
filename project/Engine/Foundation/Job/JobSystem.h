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

namespace Cake {

// 作業の優先度。ワーカーは High → Normal → Low の順に取り出す.
enum class JobPriority : uint8_t {
	High,
	Normal,
	Low,
};
inline constexpr size_t kJobPriorityCount = 3; // 優先度の数（キューの本数）.



class JobSystem {
private:
	// 作業と、その完了処理の組.
	struct Job {
		std::function<void()> work;     // ワーカーで動く.
		std::function<void()> complete; // メインスレッドで動く.
	};

	// ワーカーのスレッド。メインスレッドは含まない.
	std::vector<std::thread> workers_;

	std::mutex mutex_;                      // 作業キューと完了処理キューを守るカギ.
	std::condition_variable workAvailable_; // 作業が積まれた、または止める知らせ.
	bool stopping_ = false;                 // 止める知らせ。mutex_ を取ってから触る.

	// Initialize を呼んだスレッド（メインスレッド）の番号.
	std::thread::id mainThreadId_;

	// 優先度ごとの、まだ実行していない作業。mutex_ を取ってから触る.
	std::array<std::deque<Job>, kJobPriorityCount> queues_;
	// 作業が終わって、メインスレッドで実行するのを待っている完了処理。mutex_ を取ってから触る.
	std::deque<std::function<void()>> completions_;

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

	// 作業と完了処理を積む。work はワーカーで、complete は PumpMainThread の中で実行される.
	void Submit(JobPriority priority, std::function<void()> work, std::function<void()> complete);

	// 溜まっている完了処理を実行する。メインスレッドから毎フレーム1回呼ぶ.
	// 使った時間が budget を超えたら、残りは次のフレームに回す。ただし最低1つは実行する.
	void PumpMainThread(std::chrono::microseconds budget);

	/// <summary>
	/// 引数が1以下なら1を返す。2以上なら hardwareConcurrency-1 を返す。
	/// </summary>
	/// <returns>ワーカーの本数（メインスレッドを除く）</returns>
	static uint32_t ComputeWorkerCount(uint32_t hardwareConcurrency);

private:

	// どれかのキューに作業があるか。mutex_ を取ってから呼ぶこと.
	bool HasQueuedLocked() const;
	// 優先度の高いキューから作業を1つ取り出す。mutex_ を取ってから、作業があるときだけ呼ぶこと.
	Job PopLocked();

	void WorkerMain(uint32_t index);
};

} // namespace Cake
