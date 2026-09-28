#include "FrameVertexBuffer.h"

#include <cassert>

#include "GPUResourceUtility.h"

namespace Cake {

void FrameVertexBuffer::Initialize(ID3D12Device* device, size_t capacityBytes) {
	resource_ = CreateBufferResource(device, capacityBytes);
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
	base_ = resource_->GetGPUVirtualAddress();
	capacity_ = capacityBytes;
	offset_ = 0;
}

D3D12_VERTEX_BUFFER_VIEW FrameVertexBuffer::Allocate(const void* data, size_t sizeInBytes, uint32_t stride) {
	// 積むものが無いのは異常ではない。空のVBVを返して呼び出し側に諦めさせる.
	if (sizeInBytes == 0) {
		return D3D12_VERTEX_BUFFER_VIEW{};
	}

	constexpr size_t kAlign = 16;
	offset_ = (offset_ + (kAlign - 1)) & ~(kAlign - 1);

	// 枯渇。範囲外memcpyと不正なGPUアドレス参照を防ぐため、必ずここで食い止める.
	if (offset_ + sizeInBytes > capacity_) {
		assert(false && "フレーム頂点バッファが枯渇しました。容量を増やしてください。");
		return D3D12_VERTEX_BUFFER_VIEW{};
	}

	std::memcpy(mapped_ + offset_, data, sizeInBytes);

	D3D12_VERTEX_BUFFER_VIEW vbv{};
	vbv.BufferLocation = base_ + offset_;
	vbv.SizeInBytes = static_cast<UINT>(sizeInBytes);
	vbv.StrideInBytes = stride;

	offset_ += sizeInBytes;
	return vbv;
}

} // namespace Cake
