#include "DescriptorHeap.h"
#include <cassert>
#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "DescriptorHeap";
}

void DescriptorHeap::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = heapType;
	desc.NumDescriptors = numDescriptors;
	desc.Flags = shaderVisible
	                 ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	                 : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_));
	AssertHRESULT(hr, "DescriptorHeapの生成");

	descriptorSize_ = device->GetDescriptorHandleIncrementSize(heapType);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandle(uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorSize_ * index;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(uint32_t index) const {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = heap_->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorSize_ * index;
	return handle;
}

} // namespace Cake
