#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "Engine/Application/BuildConfig.h"

namespace Cake {

class DescriptorHeap; // 前方宣言

class SwapChain {
private:
	static constexpr UINT kBufferCount = 2;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[kBufferCount];
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kBufferCount]{};
#ifdef ENABLE_EDITOR
	DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
#else
	DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
#endif
	UINT currentIndex_ = 0;

	// リサイズ用に保持.
	ID3D12Device* device_ = nullptr;
	DescriptorHeap* rtvHeap_ = nullptr;
	DXGI_FORMAT swapChainFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM; // 生成時フォーマット（RTVはSRGBだが本体はUNORM）

public:
	void Initialize(
		IDXGIFactory7* factory,
		ID3D12Device* device,
		ID3D12CommandQueue* commandQueue,
		HWND hwnd,
		DescriptorHeap* rtvHeap,
		uint32_t width,
		uint32_t height
	);

	void Present();

	// フレーム先頭で呼ぶ：バックバッファへのバリアを張る
	void BeginFrame(ID3D12GraphicsCommandList* commandList);
	// フレーム末尾で呼ぶ：Presentバリアを張る
	void EndFrame(ID3D12GraphicsCommandList* commandList);

	void Resize(uint32_t width, uint32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const { return rtvHandles_[currentIndex_]; }
	UINT GetCurrentIndex() const { return currentIndex_; }
	DXGI_FORMAT GetRTVFormat() const { return rtvFormat_; }
	UINT GetBufferCount() const { return kBufferCount; }
};

} // namespace Cake
