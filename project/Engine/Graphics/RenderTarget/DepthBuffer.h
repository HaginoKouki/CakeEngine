#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

namespace Cake {

class DescriptorHeap; // 前方宣言

class DepthBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};
	DXGI_FORMAT format_ = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// リサイズ用に保持.
	ID3D12Device* device_ = nullptr;
	DescriptorHeap* dsvHeap_ = nullptr;

public:
	void Initialize(
		ID3D12Device* device,
		DescriptorHeap* dsvHeap,
		uint32_t width,
		uint32_t height
	);
	void Resize(uint32_t width, uint32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;
	DXGI_FORMAT GetFormat() const { return format_; }

private:
	void CreateResource(uint32_t width, uint32_t height);
};

} // namespace Cake
