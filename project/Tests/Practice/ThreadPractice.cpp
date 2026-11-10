#include "externals/doctest/doctest.h"

#include <cstdio>
#include <mutex>
#include <thread>

TEST_CASE("練習1: 鍵なしで数える") {
	int counter = 0;

	// counter を 100000 回 1 ずつ増やすラムダ式.
	auto countUp = [&counter] {
		for (int i = 0; i < 100000; ++i) {
			++counter;
		}
	};

	// ▼ countUp を動かすスレッドを2本作る（std::thread a(countUp); のように）.
	std::thread t1(countUp);
	std::thread t2(countUp);
	// ▼ 2本とも join する.
	t1.join();
	t2.join();

	std::printf("counter = %d\n", counter); // 100000 × 2 で 200000 になるはず？
}
TEST_CASE("練習2: 鍵ありで数える") {
	int counter = 0;
	std::mutex mutex; // counter を守る鍵.

	auto countUp = [&counter, &mutex] {
		for (int i = 0; i < 100000; ++i) {
			// ▼ ここで鍵を取る（std::lock_guard lock(mutex); の1行）.
			std::lock_guard lock(mutex);
			++counter;
		} // ← lock はここで破棄され、そのとき鍵が自動で返される.
	};

	// ▼ 実験1と同じく、スレッドを2本作って、2本とも join する.
	std::thread t1(countUp);
	std::thread t2(countUp);
	t1.join();
	t2.join();

	std::printf("counter = %d\n", counter);
}
