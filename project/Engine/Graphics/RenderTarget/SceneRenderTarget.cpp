#include "SceneRenderTarget.h"
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"

namespace Cake {

namespace {
constexpr const char* kLogCategory = "SceneRenderTarget";
constexpr uint32_t kMaxDim = 16384;
} // namespace

void SceneRenderTarget::Initialize(
	ID3D12Device* device,
	DescriptorHeap* rtvHeap,
	DescriptorHeap* srvHeap,
	uint32_t rtvIndex,
	uint32_t srvIndex,
	uint32_t width,
	uint32_t height
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	rtvHeap_ = rtvHeap;
	srvHeap_ = srvHeap;
	rtvIndex_ = rtvIndex;
	srvIndex_ = srvIndex;

	CreateResources(width, height);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void SceneRenderTarget::CreateResources(uint32_t width, uint32_t height) {
	width_ = width;
	height_ = height;
	resource_.Reset();

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = format_;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RTとして使う.

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clear{};
	clear.Format = format_;
	clear.Color[0] = clearColor_[0];
	clear.Color[1] = clearColor_[1];
	clear.Color[2] = clearColor_[2];
	clear.Color[3] = clearColor_[3];

	// 初期状態をSRVにしておく（最初のフレームのバリア SRV→RT と整合させるため）.
	currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	HRESULT hr = device_->CreateCommittedResource(
		&heap, D3D12_HEAP_FLAG_NONE, &desc,
		currentState_, &clear, IID_PPV_ARGS(&resource_)
	);
	AssertHRESULT(hr, "SceneRenderTargetの生成");

	// RTV（同じ番号に作り直す）.
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format_;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvHandle_ = rtvHeap_->GetCPUHandle(rtvIndex_);
	device_->CreateRenderTargetView(resource_.Get(), &rtvDesc, rtvHandle_);

	// SRV（同じ番号に作り直す）.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = format_;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device_->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHeap_->GetCPUHandle(srvIndex_));
	srvGpuHandle_ = srvHeap_->GetGPUHandle(srvIndex_);
}

void SceneRenderTarget::TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList) {
	if (currentState_ == D3D12_RESOURCE_STATE_RENDER_TARGET)
		return;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);
	currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void SceneRenderTarget::TransitionToShaderResource(ID3D12GraphicsCommandList* commandList) {
	if (currentState_ == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		return;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);
	currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void SceneRenderTarget::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0)
		return;
	CreateResources(width, height);
}

void SceneRenderTarget::RequestResize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0)
		return;
	if (width > kMaxDim || height > kMaxDim)
		return; // 異常値ガード.
	if (width == width_ && height == height_)
		return;
	resizeRequested_ = true;
	pendingWidth_ = width;
	pendingHeight_ = height;
}

bool SceneRenderTarget::ConsumePendingResize(uint32_t& outW, uint32_t& outH) {
	if (!resizeRequested_)
		return false;
	resizeRequested_ = false;
	outW = pendingWidth_;
	outH = pendingHeight_;
	return true;
}

} // namespace Cake
