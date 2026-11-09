#include "externals/doctest/doctest.h"

#include "Engine/Foundation/Job/JobSystem.h"

using namespace Cake;

TEST_CASE("JobSystem: ワーカーの本数の計算") {
	// ▼ ComputeWorkerCount(0) が 1 になることを CHECK で確かめる.
	// ▼ 同じように 1 → 1、2 → 1、8 → 7 を確かめる.
	CHECK(JobSystem::ComputeWorkerCount(0) == 1);
	CHECK(JobSystem::ComputeWorkerCount(1) == 1);
	CHECK(JobSystem::ComputeWorkerCount(2) == 1);
	CHECK(JobSystem::ComputeWorkerCount(8) == 7);
}

TEST_CASE("JobSystem: 起動して止める") {
	JobSystem jobSystem;
	// ▼ Initialize(4) を呼ぶ.
	jobSystem.Initialize(4);
	// ▼ Shutdown() を呼ぶ.
	jobSystem.Shutdown();
	// ▼ もう一度 Shutdown() を呼ぶ（落ちなければ成功）.
	jobSystem.Shutdown();
	// ここで jobSystem が破棄され、デストラクタからも Shutdown が呼ばれる.
}
