#include "SwapChain.h"
#include <cassert>
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"

namespace Cake {

namespace {
constexpr const char* kLogCategory = "SwapChain";
}

void SwapChain::Initialize(
	IDXGIFactory7* factory,
	ID3D12Device* device,
	ID3D12CommandQueue* commandQueue,
	HWND hwnd,
	DescriptorHeap* rtvHeap,
	uint32_t width,
	uint32_t height
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	// SwapChainの生成.
	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = width;
	desc.Height = height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = kBufferCount;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr = factory->CreateSwapChainForHwnd(
		commandQueue, hwnd, &desc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())
	);
	AssertHRESULT(hr, "スワップチェーンの生成");

	// リサイズ用に保持.
	device_ = device;
	rtvHeap_ = rtvHeap;

	// バックバッファリソースの取得.
	for (UINT i = 0; i < kBufferCount; ++i) {
		hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		AssertHRESULT(hr, "SwapChainバッファの取得");
	}

	// RTVの生成.
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = rtvFormat_;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < kBufferCount; ++i) {
		rtvHandles_[i] = rtvHeap->GetCPUHandle(i);
		device->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc, rtvHandles_[i]);
	}

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void SwapChain::Present() {
	swapChain_->Present(1, 0);
}

void SwapChain::BeginFrame(ID3D12GraphicsCommandList* commandList) {
	currentIndex_ = swapChain_->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources_[currentIndex_].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);
}

void SwapChain::EndFrame(ID3D12GraphicsCommandList* commandList) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources_[currentIndex_].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);
}

void SwapChain::Resize(uint32_t width, uint32_t height) {
	// 既存バックバッファの参照を全て解放（ResizeBuffersの必須条件）.
	for (UINT i = 0; i < kBufferCount; ++i) {
		swapChainResources_[i].Reset();
	}

	// バックバッファを作り直す.
	HRESULT hr = swapChain_->ResizeBuffers(
		kBufferCount, width, height,
		swapChainFormat_, // 生成時と同じフォーマットを指定.
		0
	);
	AssertHRESULT(hr, "スワップチェーンのリサイズ");

	// バックバッファを取得し直して、RTVを作り直す.
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = rtvFormat_;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < kBufferCount; ++i) {
		hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		AssertHRESULT(hr, "リサイズ後のSwapChainバッファ取得");

		rtvHandles_[i] = rtvHeap_->GetCPUHandle(i);
		device_->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc, rtvHandles_[i]);
	}
}

} // namespace Cake
