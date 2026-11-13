#include "externals/doctest/doctest.h"

#include "Engine/Graphics/Device/DeferredReleaseQueue.h"

using namespace Cake;

TEST_CASE("DeferredReleaseQueue: 締めて、GPU が通り過ぎるまで解放しない") {
	DeferredReleaseQueue queue;
	int released = 0;

	// ▼ released を 1 増やすラムダ式を Enqueue する.
	queue.Enqueue([&released]() { ++released; });
	// ▼ ReleaseCompleted(100) を呼んでも、まだ締めていないので released が 0 のままであることを確かめる.
	queue.ReleaseCompleted(100);
	CHECK(released == 0);
	// ▼ CloseFrame(5) で締める.
	queue.CloseFrame(5);
	// ▼ ReleaseCompleted(4) では、まだ released が 0 であることを確かめる.
	queue.ReleaseCompleted(4);
	CHECK(released == 0);
	// ▼ ReleaseCompleted(5) で released が 1 になることを確かめる.
	queue.ReleaseCompleted(5);
	CHECK(released == 1);
	// ▼ GetPendingCount() が 0 になったことを確かめる.
	CHECK(queue.GetPendingCount() == 0);
}

TEST_CASE("DeferredReleaseQueue: フレームの途中でフェンスが進んでも、早すぎる解放にならない") {
	DeferredReleaseQueue queue;
	int released = 0;

	// フレームの途中で TextureManager が ExecuteAndWait を呼び、番号が 5 まで進んだとする.
	// ▼ その前と後に1つずつ Enqueue する（どちらも released を 1 増やす）.
	queue.Enqueue([&released]() { ++released; });
	queue.Enqueue([&released]() { ++released; });
	// ▼ フレームの終わりの番号は 6 として、CloseFrame(6) で締める.
	queue.CloseFrame(6);
	// ▼ ReleaseCompleted(5) では、どちらもまだ解放されないことを確かめる.
	queue.ReleaseCompleted(5);
	CHECK(released == 0);
	// ▼ ReleaseCompleted(6) で、2つとも解放されることを確かめる.
	queue.ReleaseCompleted(6);
	CHECK(released == 2);
}

TEST_CASE("DeferredReleaseQueue: ReleaseAll はすべてを解放する") {
	DeferredReleaseQueue queue;
	int released = 0;
	// ▼ 締めたものと、まだ締めていないものを1つずつ積み、ReleaseAll で両方解放されることを確かめる.
	queue.Enqueue([&released]() { ++released; });
	queue.CloseFrame(5);
	queue.Enqueue([&released]() { ++released; });
	queue.ReleaseAll();
	CHECK(released == 2);
}
