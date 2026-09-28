#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

namespace Cake {

/*
 * ディスクリプタヒープの先頭を予約する枠。
 * エディタ固有の描画先（ImGui フォント・ビューポート2枚）がここを使う。
 * TextureManager など動的に確保する側は、必ず kSrvReservedCount から始めること。
 */
namespace Reserved {
#ifdef ENABLE_EDITOR
constexpr uint32_t kSrvImGui = 0;
constexpr uint32_t kSrvSceneRT = 1;
constexpr uint32_t kSrvDebugSceneRT = 2;
constexpr uint32_t kSrvReservedCount = 3;

constexpr uint32_t kRtvSceneRT = 2; // 0,1 は SwapChain
constexpr uint32_t kRtvDebugSceneRT = 3;
#else
constexpr uint32_t kSrvReservedCount = 0;
#endif
} // namespace Reserved

class DescriptorHeap {
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
	uint32_t descriptorSize_ = 0;

public:
	void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// CPU/GPUハンドルをインデックス指定で取得.
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

	ID3D12DescriptorHeap* GetHeap() const {
		return heap_.Get();
	}
	uint32_t GetSize() const {
		return descriptorSize_;
	}
};

} // namespace Cake
