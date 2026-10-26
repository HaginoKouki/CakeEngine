#pragma once
/*====================================
 *
 * SRV 用のディスクリプタヒープを1本持ち、番号を先頭から順に払い出すクラス。
 * 起動時に kMaxSrvCount 個ぶんのヒープを確保し、Allocate() のたびに 0, 1, 2… と割り当てる。
 * 解放はしない。作り直す SRV（リサイズする描画先など）は、最初に受け取った番号へ作り直すこと。
 *
 * ====================================*/
#include <cstdint>
#include <d3d12.h>

#include "Engine/Graphics/Descriptor/DescriptorHeap.h"

namespace Cake {

class SrvManager {
public:
	static constexpr uint32_t kMaxSrvCount = 512; // ヒープに確保する SRV の数.

private:
	DescriptorHeap heap_;
	uint32_t nextIndex_ = 0; // 次に払い出す番号.

public:
	void Initialize(ID3D12Device* device);

	// 次の番号を1つ払い出す。上限を超えたら assert で止める.
	uint32_t Allocate();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const { return heap_.GetCPUHandle(index); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const { return heap_.GetGPUHandle(index); }
	ID3D12DescriptorHeap* GetHeap() const { return heap_.GetHeap(); }

	// 使用済みの数。デバッグ表示で、番号が増え続けていないかを確かめるのに使う.
	uint32_t GetUsedCount() const { return nextIndex_; }
};

} // namespace Cake
