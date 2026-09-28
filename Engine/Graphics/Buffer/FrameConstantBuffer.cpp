#include "FrameConstantBuffer.h"

#include "GPUResourceUtility.h"

namespace Cake {

void FrameConstantBuffer::Initialize(ID3D12Device* device, size_t capacityBytes) {
	resource_ = CreateBufferResource(device, capacityBytes);
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	base_ = resource_->GetGPUVirtualAddress();
	capacity_ = capacityBytes;
	offset_ = 0;
}

} // namespace Cake
