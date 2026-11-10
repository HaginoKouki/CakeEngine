#include "externals/doctest/doctest.h"

#include <chrono> // std::chrono::milliseconds.
#include <atomic>
#include <mutex>
#include <vector>
#include <stdexcept> // std::runtime_error.
#include <string>
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
	jobSystem.Submit(
		JobPriority::Normal,
		[&] {
			gateStarted = true;
			while (!gateReleased) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		},
		[] {}
	);

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

TEST_CASE("JobSystem: 作業の戻り値が完了処理に届く") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	bool completed = false;
	bool succeeded = false;
	int received = 0;

	// ▼ Submit<int> で、作業「42 を返す」と、
	//    完了処理「completed を true、succeeded に result.Succeeded()、received に *result.value を入れる」を積む.
	//    完了処理の引数は JobResult<int> result.
	jobSystem.Submit<int>(
		JobPriority::Normal,
		[]() -> int { return 42; },
		[&](JobResult<int> result) {
			completed = true;
			succeeded = result.Succeeded();
			if (succeeded) {
				received = *result.value;
			}
		}
	);

	// ▼ completed が true になるまで、PumpMainThread(std::chrono::milliseconds(1)) を呼んでは
	//    1 ミリ秒眠る、をくり返す（最大 5000 回）.
	for (uint32_t i = 0; i < 5000; i++) {
		if (completed) {
			break;
		}
		jobSystem.PumpMainThread(std::chrono::milliseconds(1));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ succeeded が true で、received が 42 であることを CHECK で確かめる.
	CHECK(succeeded);
	CHECK(received == 42);

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: 作業の例外は失敗として完了処理に届く") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	bool completed = false;
	bool succeeded = true;
	std::string error;

	// ▼ Submit<int> で、作業「throw std::runtime_error("読み込みに失敗");」と、
	//    完了処理「completed を true、succeeded と error に結果を入れる」を積む.
	//    作業のラムダ式は、戻り値の型を -> int と書いておく（[]() -> int { throw …; }）.
	//    return が無いので、書かないと戻り値の型が int だと決まらないため.
	jobSystem.Submit<int>(
		JobPriority::Normal,
		[]() -> int { throw std::runtime_error("読み込みに失敗"); },
		[&](JobResult<int> result) {
			completed = true;
			succeeded = result.Succeeded();
			error = result.error;
		}
	);

	// ▼ completed が true になるまで待つ（上と同じ）.
	for (uint32_t i = 0; i < 5000; i++) {
		if (completed) {
			break;
		}
		jobSystem.PumpMainThread(std::chrono::milliseconds(1));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ succeeded が false で、error が "読み込みに失敗" であることを確かめる.
	CHECK_FALSE(succeeded);
	CHECK(error == "読み込みに失敗");

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: 戻り値なしの作業が例外を投げても落ちない") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	bool completed = false;

	// ▼ 今ある Submit で、作業「throw std::runtime_error("失敗");」と、完了処理「completed を true にする」を積む.
	jobSystem.Submit(
		JobPriority::Normal,
		[]() { throw std::runtime_error("失敗"); },
		[&]() { completed = true; }
	);

	// ▼ completed が true になるまで待つ.
	for (uint32_t i = 0; i < 5000; i++) {
		if (completed) {
			break;
		}
		jobSystem.PumpMainThread(std::chrono::milliseconds(1));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// ▼ completed が true であることを確かめる（ここまで来れば、落ちなかったことになる）.
	CHECK(completed);

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: Submit は作業ごとに違う番号を返す") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	// ▼ Submit を2回呼び、返ってきた JobHandle を a と b に入れる（作業も完了処理も何もしないラムダ式でよい）.
	JobHandle a = jobSystem.Submit(JobPriority::Normal, [] {}, [] {});
	JobHandle b = jobSystem.Submit(JobPriority::Normal, [] {}, [] {});
	// ▼ a.IsValid() と b.IsValid() が true であることを確かめる.
	CHECK(a.IsValid());
	CHECK(b.IsValid());
	// ▼ a.id と b.id が違うことを確かめる（CHECK(a.id != b.id);）.
	CHECK(a.id != b.id);

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: WaitFor は完了処理が終わるまで待つ") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	bool completed = false;

	// ▼ 作業「10 ミリ秒眠る」、完了処理「completed を true にする」を Submit して、handle に入れる.
	JobHandle handle = jobSystem.Submit(
		JobPriority::Normal,
		[]() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); },
		[&]() { completed = true; }
	);
	// ▼ WaitFor(handle) を呼ぶ.
	jobSystem.WaitFor(handle);
	// ▼ completed が true であることを確かめる（PumpMainThread は呼ばない）.
	CHECK(completed);

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: ワーカーが埋まっていても WaitFor はメインスレッドで実行して戻る") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	// ▼ 優先度のテストと同じ「止めの作業」を積み、ワーカーが止めの作業を始めるまで待つ.
	std::atomic<bool> gateStarted = false;  // 止めの作業が始まったか.
	std::atomic<bool> gateReleased = false; // 止めの作業に出す合図.
	jobSystem.Submit(
		JobPriority::Normal,
		[&] {
			gateStarted = true;
			while (!gateReleased) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		},
		[] {}
	);
	for (size_t i = 0; i < 5000; i++) {
		if (gateStarted) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::thread::id ranOn; // 作業を実行したスレッドの番号.
	bool completed = false;

	// ▼ 作業「ranOn に std::this_thread::get_id() を入れる」、完了処理「completed を true にする」を Submit する.
	JobHandle handle = jobSystem.Submit(
		JobPriority::Normal,
		[&]() { ranOn = std::this_thread::get_id(); },
		[&]() { completed = true; }
	);
	// ▼ WaitFor を呼ぶ。ワーカーは止めの作業の中にいるので、メインスレッドが実行するはず.
	jobSystem.WaitFor(handle);
	// ▼ completed が true で、ranOn がこのテストのスレッドの番号と同じことを確かめる.
	CHECK(completed);
	CHECK(ranOn == std::this_thread::get_id());

	// ▼ 止めの作業に合図を出してから、Shutdown を呼ぶ.
	//    （合図を出さないと、止めの作業が終わらず、Shutdown の join がいつまでも戻らない）
	gateReleased = true;
	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: WaitFor はほかの作業の完了処理を実行しない") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);

	bool otherCompleted = false;

	// ▼ 作業「何もしない」、完了処理「otherCompleted を true にする」を Submit する（こちらが「ほか」）.
	jobSystem.Submit(
		JobPriority::Normal,
		[]() {},
		[&]() { otherCompleted = true; }
	);
	// ▼ 作業も完了処理も何もしないものを Submit して、その handle を target に入れる.
	JobHandle target = jobSystem.Submit(
		JobPriority::Normal,
		[]() {},
		[]() {}
	);
	// ▼ WaitFor(target) を呼ぶ.
	jobSystem.WaitFor(target);
	// ▼ otherCompleted が false のままであることを確かめる.
	CHECK_FALSE(otherCompleted);

	// ▼ PumpMainThread を呼んでから、otherCompleted が true になったことを確かめる.
	//    （ほかの完了処理は、PumpMainThread で実行される）
	for (uint32_t i = 0; i < 5000; i++) {
		if (otherCompleted == true) {
			break;
		}
		jobSystem.PumpMainThread(std::chrono::milliseconds(1));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	CHECK(otherCompleted);

	jobSystem.Shutdown();
}

TEST_CASE("JobSystem: Shutdown のあとの Submit は受け付けない") {
	JobSystem jobSystem;
	jobSystem.Initialize(1);
	jobSystem.Shutdown();

	// ▼ Submit して、返ってきた handle が IsValid() で false であることを確かめる.
	JobHandle handle = jobSystem.Submit(JobPriority::Normal, []() {}, []() {});
	CHECK_FALSE(handle.IsValid());
}
