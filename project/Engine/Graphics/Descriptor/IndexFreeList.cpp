#include "IndexFreeList.h"

namespace Cake {

void IndexFreeList::Initialize(uint32_t capacity) {
	capacity_ = capacity;

	nextUnused_ = 0;
	usedCount_ = 0;
	peakUsedCount_ = 0;

	freeIndices_.clear();

	inUse_.assign(capacity, false);
}

uint32_t IndexFreeList::Allocate() {
	uint32_t index = kInvalidIndex;

	if (!freeIndices_.empty()) {
		// 空きリストに番号があれば、末尾から1つ取り出して index にする.
		index = freeIndices_.back();
		freeIndices_.pop_back();
	} else {
		if (nextUnused_ < capacity_) {
			// 空きリストに番号が無ければ、nextUnused を進める.
			index = nextUnused_++;
		} else {
			// 容量を使い切っている.
			return kInvalidIndex;
		}
	}

	// inUse_[index] を true にする.
	inUse_[index] = true;
	// usedCount_ を 1 増やす。peakUsedCount_ を超えたら、peakUsedCount_ も更新する.
	if (++usedCount_ > peakUsedCount_) {
		peakUsedCount_ = usedCount_;
	}
	return index;
}

bool IndexFreeList::Free(uint32_t index) {
	// index が capacity_ 以上なら、範囲外なので false を返す（kInvalidIndex もここで弾かれる）.
	if (index >= capacity_) {
		return false;
	}
	// inUse_[index] が false なら、二重解放か、まだ払い出していない番号なので false を返す.
	if (!inUse_[index]) {
		return false;
	}

	// inUse_[index] を false にする.
	inUse_[index] = false;

	// 空きリストの末尾に index を入れる.
	freeIndices_.push_back(index);

	// usedCount_ を 1 減らす.
	usedCount_--;

	return true;
}

} // namespace Cake
