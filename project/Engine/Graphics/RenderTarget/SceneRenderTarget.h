#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

namespace Cake {

class DescriptorHeap;

// ゲーム画面をオフスクリーンに描画し、ImGui::Imageで表示するための描画先.
// カラーRT（RTV + SRV）を1枚持つ。深度は共有のDepthBufferを使う.
class SceneRenderTarget {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};    // rtvHeap内のCPUハンドル（描画先）.
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle_{}; // srvHeap内のGPUハンドル（ImGui::Image用）.
	DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 現在のリソース状態（バリアの StateBefore に使う）.
	D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// Resize用に保持.
	ID3D12Device* device_ = nullptr;
	DescriptorHeap* rtvHeap_ = nullptr;
	DescriptorHeap* srvHeap_ = nullptr;
	uint32_t rtvIndex_ = 0;
	uint32_t srvIndex_ = 0;

	// 遅延リサイズ用（フレーム途中でRTを作り直さないため）.
	bool resizeRequested_ = false;
	uint32_t pendingWidth_ = 0;
	uint32_t pendingHeight_ = 0;

	float clearColor_[4] = {0.1f, 0.25f, 0.5f, 1.0f};

public:
	void Initialize(
		ID3D12Device* device,
		DescriptorHeap* rtvHeap,
		DescriptorHeap* srvHeap,
		uint32_t rtvIndex,
		uint32_t srvIndex,
		uint32_t width,
		uint32_t height
	);

	// 描画先へ遷移（SRV等 → RENDER_TARGET）.
	void TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList);
	// ImGuiが読めるように遷移（RENDER_TARGET → PIXEL_SHADER_RESOURCE）.
	void TransitionToShaderResource(ID3D12GraphicsCommandList* commandList);

	// 実リサイズ（GPUアイドル状態で呼ぶこと）.
	void Resize(uint32_t width, uint32_t height);

	// ゲームビューのウィンドウサイズから毎フレーム要求 → 次フレーム頭で適用.
	void RequestResize(uint32_t width, uint32_t height);
	bool ConsumePendingResize(uint32_t& outW, uint32_t& outH);

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle() const { return rtvHandle_; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGpuHandle() const { return srvGpuHandle_; }
	const float* GetClearColor() const { return clearColor_; }
	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }

private:
	void CreateResources(uint32_t width, uint32_t height);
};

} // namespace Cake
