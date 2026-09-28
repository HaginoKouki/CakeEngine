#include "DepthBuffer.h"
#include <cassert>
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"

namespace Cake {

namespace {
constexpr const char* kLogCategory = "DepthBuffer";
constexpr uint32_t kMaxDim = 16384;
}

void DepthBuffer::Initialize(
	ID3D12Device* device,
	DescriptorHeap* dsvHeap,
	uint32_t width,
	uint32_t height
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	dsvHeap_ = dsvHeap;

	CreateResource(width, height);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void DepthBuffer::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0 || width > kMaxDim || height > kMaxDim)
		return;
	// 古いリソースを明示的に解放してから作り直す.
	resource_.Reset();
	CreateResource(width, height);
}

void DepthBuffer::CreateResource(uint32_t width, uint32_t height) {
	// リソース生成.
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format_;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// 利用するHeapの設定.
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度値のクリア設定.
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = format_;

	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource_)
	);
	AssertHRESULT(hr, "DepthStencilResourceの生成");

	// DSV生成.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = format_;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	dsvHandle_ = dsvHeap_->GetCPUHandle(0);
	device_->CreateDepthStencilView(resource_.Get(), &dsvDesc, dsvHandle_);
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetDSVHandle() const {
	return dsvHandle_;
}

} // namespace Cake
