#include "JobSystem.h"

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

	const uint32_t count = (workerCount == 0 ? ComputeWorkerCount(std::thread::hardware_concurrency()) : workerCount);
	for (uint32_t i = 0; i < count; i++) {
		workers_.emplace_back(&JobSystem::WorkerMain, this, i);
	}
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, std::format("ワーカーを{}本で起動しました", count));

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void JobSystem::Shutdown() {
	for (auto& worker : workers_) {
		worker.join();
	}
	workers_.clear();
}

uint32_t JobSystem::ComputeWorkerCount(uint32_t hardwareConcurrency) {
	return hardwareConcurrency > 1 ? hardwareConcurrency - 1 : 1;
}

void JobSystem::WorkerMain(uint32_t index) {
	// このスレッドの名前を決める.
	DebugLog::SetCurrentThreadName(std::format("Worker{:02}", index + 1));
	// 起動したことをログに出す.
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, std::format("スレッドが起動しました"));
}

}
