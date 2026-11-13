#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <vector>

namespace Cake {

// GPU が使い終わるまで、リソースの解放を遅らせる箱。メインスレッド専用.
class DeferredReleaseQueue {
private:
	// フェンスの番号が付いた、解放の予約のまとまり.
	struct Batch {
		uint64_t fenceValue = 0; // この番号を GPU が通り過ぎたら解放してよい.
		std::vector<std::function<void()>> releases;
	};

	// まだ番号が付いていない予約（今のフレームで積まれたもの）.
	std::vector<std::function<void()>> pending_;
	// 番号が付いた予約。古い順に並ぶ.
	std::deque<Batch> batches_;

public:
	~DeferredReleaseQueue();

	// 解放の予約を積む.
	void Enqueue(std::function<void()> release);

	// 今のフレームで積んだ予約に、番号を付ける。フレームの終わり（ExecuteAndWait の後）に呼ぶ.
	void CloseFrame(uint64_t fenceValue);

	// GPU が completedValue まで通り過ぎたので、番号がそれ以下の予約を解放する.
	void ReleaseCompleted(uint64_t completedValue);

	// 番号に関係なく、すべての予約を解放する。GPU を待ち切ったあと（終了時など）に呼ぶ.
	void ReleaseAll();

	// まだ解放していない予約の数（GraphicsDebugWindow で表示する）.
	size_t GetPendingCount() const;
};

} // namespace Cake
