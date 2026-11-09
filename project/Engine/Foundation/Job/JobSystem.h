#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Cake {

class JobSystem {
private:
	std::vector<std::thread> workers_;

	std::mutex mutex_;
	std::condition_variable workAvailable_; // 作業が積まれた、または止める知らせ.

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
	/// 引数が1以下なら1を返す。2以上なら hardwareConcurrency-1 を返す。
	/// </summary>
	/// <returns>ワーカーの本数（メインスレッドを除く）</returns>
	static uint32_t ComputeWorkerCount(uint32_t hardwareConcurrency);

private:
	void WorkerMain(uint32_t index);
};

} // namespace Cake
