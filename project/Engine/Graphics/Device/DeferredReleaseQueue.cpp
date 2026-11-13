#include "DeferredReleaseQueue.h"

namespace Cake {

DeferredReleaseQueue::~DeferredReleaseQueue() {
	ReleaseAll();
}

void DeferredReleaseQueue::Enqueue(std::function<void()> release) {
	pending_.push_back(std::move(release));
}

void DeferredReleaseQueue::CloseFrame(uint64_t fenceValue) {
	// pending_ が空なら、何もせずに return する（空のまとまりを作らない）.
	if (pending_.empty()) {
		return;
	}
	// Batch を作り、fenceValue と、pending_ の中身を入れて、batches_ の末尾に入れる.
	Batch batch;
	batch.fenceValue = fenceValue;
	batch.releases = std::move(pending_);
	batches_.push_back(std::move(batch));

	// pending_ を空にする.
	pending_.clear();
}

void DeferredReleaseQueue::ReleaseCompleted(uint64_t completedValue) {
	for (auto it = batches_.begin(); it != batches_.end();) {
		if (it->fenceValue <= completedValue) {
			// 先頭の fenceValue が completedValue 以下なら、その releases を全部呼んでから pop_front する.
			for (auto& release : it->releases) {
				release();
			}
			it = batches_.erase(it);
		} else {
			// completedValue より大きければ、そこで止める（後ろはもっと新しいので、まだ解放できない）.
			break;
		}
	}
}

void DeferredReleaseQueue::ReleaseAll() {
	// batches_ のすべての releases を呼んで、batches_ を空にする.
	for (auto& batch : batches_) {
		for (auto& release : batch.releases) {
			release();
		}
	}
	batches_.clear();
	// pending_ のすべてを呼んで、pending_ を空にする.
	for (auto& release : pending_) {
		release();
	}
	pending_.clear();
}

size_t DeferredReleaseQueue::GetPendingCount() const {
	// pending_ の数と、batches_ のそれぞれの releases の数を足して返す.
	size_t count = pending_.size();
	for (const auto& batch : batches_) {
		count += batch.releases.size();
	}
	return count;
}

} // namespace Cake
