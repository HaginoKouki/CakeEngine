#include "JobSystem.h"

#include <Windows.h>
#include <objbase.h>
#include <format>
#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
// ログに出すときの分類の名前.
constexpr const char* kLogCategory = "JobSystem";
} // namespace
JobSystem::~JobSystem() {
	Shutdown();
}

void JobSystem::Initialize(uint32_t workerCount) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	// メインスレッドのスレッドIDを記録する.
	mainThreadId_ = std::this_thread::get_id();

	{
		std::lock_guard lock(mutex_); // カギを取る.
		stopping_ = false;
	} // カギを返す.

	// スレッドの本数を決めて起動する.
	const uint32_t count = (workerCount == 0 ? ComputeWorkerCount(std::thread::hardware_concurrency()) : workerCount);
	for (uint32_t i = 0; i < count; i++) {
		workers_.emplace_back(&JobSystem::WorkerMain, this, i);
	}
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, std::format("ワーカーを{}本で起動しました", count));

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void JobSystem::Shutdown() {
	// 眠っているワーカーを起こす.
	{
		std::lock_guard lock(mutex_); // カギを取る.
		stopping_ = true;             // 条件を変える.
		for (size_t i = 0; i < kJobPriorityCount; i++) {
			queues_[i].clear(); // キューを空にする.
		}
	} // カギを返す.
	workAvailable_.notify_all(); // 眠っている全員を起こす.

	// 全ワーカーを join し、解放する.
	for (auto& worker : workers_) {
		worker.join();
	}
	workers_.clear();

	{
		std::lock_guard lock(mutex_); // カギを取る.
		completions_.clear();         // 完了処理を空にする.
		live_.clear();                // 受け付けた作業の番号を空にする.
	} // カギを返す.
}

JobHandle JobSystem::Submit(JobPriority priority, std::function<void()> work, std::function<void()> complete) {
	// 優先度をキューの添字に変換する.
	const size_t index = static_cast<size_t>(priority);

	uint64_t id = 0; // 作業の番号を入れる変数.
	// 作業と完了処理をキューに移す.
	{
		std::lock_guard lock(mutex_);
		if (stopping_) {
			return JobHandle{};
		}
		id = nextId_++;
		live_.insert(id); // 受け付けた作業の番号を記録する.
		queues_[index].push_back(Job{id, std::move(work), std::move(complete)});
	} // カギを返す

	// 眠っているワーカーを1本だけ起こす.
	workAvailable_.notify_one();

	return JobHandle{id};
}

void JobSystem::PumpMainThread(std::chrono::microseconds budget) {
	// メインスレッド以外から呼ばれたら Debug ビルドで、実行中のプログラムを止める.
	assert(std::this_thread::get_id() == mainThreadId_ && "メインスレッドから呼ぶこと");

	const auto start = std::chrono::steady_clock::now(); // 開始時刻を記録する.
	while (true) {
		Completion completion; // 完了処理を入れる変数.
		{
			std::lock_guard lock(mutex_); // カギを取る.
			if (completions_.empty()) {   // 完了処理がなければループを抜ける.
				break;
			}
			completion = std::move(completions_.front()); // 完了処理を移す.
			completions_.pop_front();                     // キューから抜く.
		} // カギを返す.

		// カギの外で完了処理を実行する.
		completion.complete();

		{
			std::lock_guard lock(mutex_); // カギを取る.
			// 完了した作業の番号を live_ から消す.
			live_.erase(completion.id);
		}

		const auto now = std::chrono::steady_clock::now(); // 現在時刻を記録する.
		if (now - start >= budget) {                       // 使った時間が予算を超えたらループを抜ける.
			break;
		}
	}
}

void JobSystem::WorkerMain(uint32_t index) {
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// スレッドに名前を付ける.
	SetThreadDescription(GetCurrentThread(), std::format(L"Worker{:02}", index + 1).c_str());

	// このスレッドの名前を決める.
	DebugLog::SetCurrentThreadName(std::format("Worker{:02}", index + 1));
	// 起動したことをログに出す.
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, std::format("スレッドが起動しました"));

	while (true) {
		Job job; // 取り出した作業を入れる変数.
		{
			// カギを取る.
			std::unique_lock lock(mutex_);
			// stopping_ が true または、どれかのキューに作業がある状態になるまで眠る.
			workAvailable_.wait(lock, [this] { return stopping_ || HasQueuedLocked(); });

			// stopping_ が true ならループから抜ける.
			if (stopping_) {
				break;
			}

			// 作業を移し、キューから抜く.
			job = PopLocked();
		} // カギを返す.

		// カギの外で作業を実行する.
		RunJob(job);
	}

	// 停止したことをログに出す.
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, std::format("スレッドが停止しました"));

	if (SUCCEEDED(comResult)) {
		CoUninitialize();
	}
}

void JobSystem::RunJob(Job& job) {
	// カギの外で作業を実行する.
	try {
		job.work();
	} catch (const std::exception& e) {
		// 作業中に例外が投げられたらログに出す.
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::format("作業中に例外が投げられました: {}", e.what()));
	} catch (...) {
		// 作業中に例外が投げられたらログに出す.
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "作業中に不明な例外が投げられました");
	}

	{
		// カギを取る.
		std::lock_guard lock(mutex_);
		completions_.push_back(Completion{job.id, std::move(job.complete)}); // 完了処理をキューに移す.
	} // カギを返す.

	jobFinished_.notify_all(); // 眠っている WaitFor を起こす.
}

void JobSystem::WaitFor(JobHandle handle) {
	// handleが無効なら何もしない.
	if (!handle.IsValid()) {
		return;
	}
	// メインスレッド以外から呼ばれたら Debug ビルドで、実行中のプログラムを止める.
	assert(std::this_thread::get_id() == mainThreadId_ && "メインスレッドから呼ぶこと");

	std::unique_lock lock(mutex_);
	for (;;) {
		// live_ に handle.id が入っていなければ return する.
		if (!live_.contains(handle.id)) {
			return;
		}

		// completions_ から id == handle.id の Completion を探す.
		auto it = std::find_if(completions_.begin(), completions_.end(), [&handle](const Completion& c) { return c.id == handle.id; });
		if (it != completions_.end()) {
			Completion completion = std::move(*it);
			completions_.erase(it);

			// カギを返して完了処理を実行する.
			lock.unlock();
			completion.complete();

			// カギを取り返し、 live_ から handle.id を消す.
			lock.lock();
			live_.erase(handle.id);
			return;
		}

		// キューに作業があるなら.
		if (HasQueuedLocked()) {
			Job job = PopLocked();
			// カギを返して作業を実行する.
			lock.unlock();
			RunJob(job);
			lock.lock();
			continue;
		}

		// どれでもなければ.
		jobFinished_.wait(lock);
	}
}

bool JobSystem::HasQueuedLocked() const {
	// queues_ を順にみて、1つでも空でないキューがあれば true を返す.
	for (const auto& queue : queues_) {
		if (!queue.empty()) {
			return true;
		}
	}
	return false;
}
JobSystem::Job JobSystem::PopLocked() {
	// 優先度の高いキューから順にみて、空でなければ作業を取り出す.
	for (auto& queue : queues_) {
		if (!queue.empty()) {
			auto job = std::move(queue.front());
			queue.pop_front();
			return job;
		}
	}
	return {}; // 作業がないのに呼ばれた.
}

} // namespace Cake
