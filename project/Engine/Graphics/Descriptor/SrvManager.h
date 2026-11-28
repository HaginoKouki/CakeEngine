#pragma once
/*====================================
 *
 * SRV 用のディスクリプタヒープを1本持ち、空いている番号を払い出すクラス
 * 起動時に kMaxSrvCount 個ぶんのヒープを確保し、Allocate() 時に空いている番号を返す
 * Free で返した番号は使いまわす。Free は GPU が使い終わってから呼ぶこと（DeferredReleaseQueue を通す）
 * 作り直す SRV（リサイズする描画先など）は、最初に受け取った番号へ作り直すこと。
 *
 * ====================================*/
#include <cstdint>
#include <d3d12.h>

#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/Descriptor/IndexFreeList.h"

namespace Cake {

class SrvManager {
public:
	// フェーズ1で計測して 4096 に決めた（最大使用数は 32。設計書 17 章）。フェーズ2で測り直す.
	static constexpr uint32_t kMaxSrvCount = 4096; // ヒープに確保する SRV の数.

	static constexpr uint32_t kInvalidIndex = IndexFreeList::kInvalidIndex;

private:
	DescriptorHeap heap_;
	IndexFreeList indices_;

public:
	void Initialize(ID3D12Device* device);

	// 番号を1つ払い出す。尽きたら kInvalidIndex を返し、エラーのログを出す.
	uint32_t Allocate();

	// 番号を解放する.
	void Free(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const { return heap_.GetCPUHandle(index); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const { return heap_.GetGPUHandle(index); }
	ID3D12DescriptorHeap* GetHeap() const { return heap_.GetHeap(); }

	// CPU ハンドルから番号を求める。このヒープのハンドルでなければ kInvalidIndex を返す.
	uint32_t GetIndexFromCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;

	// 使用済みの数。デバッグ表示で、番号が増え続けていないかを確かめるのに使う.
	uint32_t GetUsedCount() const { return indices_.GetUsedCount(); }
	uint32_t GetPeakUsedCount() const { return indices_.GetPeakUsedCount(); }
	uint32_t GetCapacity() const { return indices_.GetCapacity(); }
};

} // namespace Cake
