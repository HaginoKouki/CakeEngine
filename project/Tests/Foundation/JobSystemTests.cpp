#include "externals/doctest/doctest.h"

#include <chrono> // std::chrono::milliseconds.
#include <atomic>
#include <mutex>
#include <vector>
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

TEST_CASE("JobSystem: 眠っているワーカーを起こして止める") {
	JobSystem jobSystem;
	jobSystem.Initialize(4);

	// ワーカーが眠りにつくまで待つ.
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// ▼ Shutdown を呼ぶ（止まったまま戻らなければ失敗）.
	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: 積んだ作業がすべて実行される") {
	JobSystem jobSystem;
	jobSystem.Initialize(4);

	std::atomic<int> counter = 0;

	// ▼ for で 100 回、counter を 1 増やす作業を Submit する.
	//    作業は [&counter] { ++counter; } のラムダ式.
	for (uint32_t i = 0; i < 100; i++) {
		jobSystem.Submit(JobPriority::Normal, [&counter] { ++counter; }, [] {});
	}

	// ▼ counter が 100 になるまで待つ.
	//    ずっと待ち続けないように、最大 5 秒（1 ミリ秒の sleep_for を 5000 回）までにする.
	for (uint32_t i = 0; i < 5000; i++) {
		if (counter == 100) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ CHECK で counter が 100 になったことを確かめる.
	CHECK(counter == 100);

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: 優先度の高い作業から実行する") {
	JobSystem jobSystem;
	jobSystem.Initialize(1); // ワーカーを1本にする.

	std::atomic<bool> gateStarted = false;  // 止めの作業が始まったか.
	std::atomic<bool> gateReleased = false; // 止めの作業に出す合図.

	// ▼ 止めの作業を Normal で積む。中身は次のとおり.
	//    gateStarted を true にする.
	//    gateReleased が true になるまで、1 ミリ秒ずつ眠りながら待つ.
	jobSystem.Submit(JobPriority::Normal, [&] {
		gateStarted = true;
		while (!gateReleased) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		} }, [] {});

	// ▼ gateStarted が true になるまで、1 ミリ秒ずつ眠りながら待つ（最大 5000 回）.
	for (size_t i = 0; i < 5000; i++) {
		if (gateStarted) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::mutex orderMutex;  // order を守る鍵.
	std::vector<int> order; // 実行された順番を記録する.

	// ▼ Low、Normal、High の順に作業を積む.
	//    Low の作業は、orderMutex の鍵を取ってから order に 2 を push_back する.
	//    Normal は 1、High は 0 を push_back する.
	jobSystem.Submit(JobPriority::Low, [&] {
		std::lock_guard lock(orderMutex);
		order.push_back(2); }, [] {});

	jobSystem.Submit(JobPriority::Normal, [&] {
		std::lock_guard lock(orderMutex);
		order.push_back(1); }, [] {});

	jobSystem.Submit(JobPriority::High, [&] {
	std::lock_guard lock(orderMutex);
	order.push_back(0); }, [] {});

	// ▼ gateReleased を true にする.
	gateReleased = true;

	// ▼ order の要素数が 3 になるまで、1 ミリ秒ずつ眠りながら待つ（最大 5000 回）.
	//    order を見るときも、orderMutex の鍵を取る.
	for (size_t i = 0; i < 5000; i++) {
		{
			std::lock_guard lock(orderMutex);
			if (order.size() == 3) {
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ order が {0, 1, 2}（High、Normal、Low の順）になっていることを CHECK で確かめる.
	//    CHECK(order == std::vector<int>{0, 1, 2}); と書ける.
	{
		std::lock_guard lock(orderMutex);
		CHECK(order == std::vector<int>{0, 1, 2});
	}

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: 完了処理はメインスレッドで実行される") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	std::atomic<bool> workDone = false; // 作業が終わったか.
	bool completed = false;             // 完了処理が呼ばれたか（メインスレッドだけが触るので atomic は要らない）.
	std::thread::id completedOn;        // 完了処理を実行したスレッドの番号.

	// ▼ 作業：workDone を true にする.
	//    完了処理：completed を true にし、completedOn に std::this_thread::get_id() を入れる.
	//    この2つを Submit する.
	std::function<void()> work = [&] { workDone = true; };
	std::function<void()> complete = [&] {
		completed = true;
		completedOn = std::this_thread::get_id();
	};
	jobSystem.Submit(JobPriority::Normal, std::move(work), std::move(complete));

	// ▼ workDone が true になるまで、1 ミリ秒ずつ眠りながら待つ（最大 5000 回）.
	for (size_t i = 0; i < 5000; i++) {
		if (workDone) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ この時点では、まだ完了処理は呼ばれていないことを CHECK_FALSE(completed) で確かめる.
	CHECK_FALSE(completed);

	// ▼ PumpMainThread を呼ぶ（budget は std::chrono::seconds(1) でよい）.
	jobSystem.PumpMainThread(std::chrono::seconds(1));

	// ▼ completed が true になったことと、
	//    completedOn がこのテストのスレッドの番号（std::this_thread::get_id()）と同じことを CHECK で確かめる.
	CHECK(completed);
	CHECK(completedOn == std::this_thread::get_id());

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: PumpMainThread は上限を超えても最低1つは実行する") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	std::atomic<int> workCount = 0; // 終わった作業の数.
	int completedCount = 0;         // 呼ばれた完了処理の数.

	// ▼ 作業「workCount を 1 増やす」、完了処理「completedCount を 1 増やす」を3回 Submit する.
	for (size_t i = 0; i < 3; i++) {
		jobSystem.Submit(JobPriority::Normal, [&] { ++workCount; }, [&] { ++completedCount; });
	}

	// ▼ workCount が 3 になるまで待つ（最大 5000 回）.
	for (size_t i = 0; i < 5000; i++) {
		if (workCount == 3) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ PumpMainThread(std::chrono::microseconds(0)) を呼ぶ。上限が 0 なので、1つだけ実行されるはず.
	// ▼ completedCount が 1 であることを CHECK で確かめる.
	jobSystem.PumpMainThread(std::chrono::microseconds(0));
	CHECK(completedCount == 1);

	// ▼ PumpMainThread(std::chrono::seconds(1)) を呼ぶ。残りの2つが実行されるはず.
	// ▼ completedCount が 3 であることを CHECK で確かめる.
	jobSystem.PumpMainThread(std::chrono::seconds(1));
	CHECK(completedCount == 3);

	jobSystem.Shutdown();
}
