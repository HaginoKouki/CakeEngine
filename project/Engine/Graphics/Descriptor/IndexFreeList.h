#pragma once
/*====================================
 *
 * 0 から容量-1 までの番号を払い出し、返された番号を空きリストに溜めて使い回す
 * D3D12 を知らない。Tests から GPU なしで確かめるため、SrvManager から番号の管理だけを分けた
 * スレッドに対して安全ではない。メインスレッド専用
 *
 * ====================================*/
#include <cstdint>
#include <limits>
#include <vector>

namespace Cake {

class IndexFreeList {
public:
	// 無効値.
	static constexpr uint32_t kInvalidIndex = (std::numeric_limits<uint32_t>::max)();

private:
	uint32_t capacity_ = 0;             // 払い出せる番号の数.
	uint32_t nextUnused_ = 0;           // まだ一度も払い出していない番号のうち、いちばん小さいもの.
	std::vector<uint32_t> freeIndices_; // 空きリスト.

	std::vector<bool> inUse_;    // 番号ごとの「今使っているか」。二重解放を見つけるのに使う.
	uint32_t usedCount_ = 0;     // 今使っている番号の数.
	uint32_t peakUsedCount_ = 0; // usedCount_ のこれまでの最大.

public:
	/// <param name="capacity">容量</param>
	void Initialize(uint32_t capacity);

	// 番号を1つ払い出す。容量を使い切っていたら kInvalidIndex を返す.
	uint32_t Allocate();

	// 番号を空ける。範囲外の番号や、すでに空いている番号なら、何もせずに false を返す.
	bool Free(uint32_t index);

	uint32_t GetCapacity() const { return capacity_; }
	uint32_t GetUsedCount() const { return usedCount_; }
	uint32_t GetPeakUsedCount()  const{ return peakUsedCount_; }
	bool IsInUse(uint32_t index) const { return index < capacity_ ? inUse_[index] : false; }
};

} // namespace Cake
