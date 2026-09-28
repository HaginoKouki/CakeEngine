#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

#include <d3d12.h>
#include <wrl.h>

namespace Cake {

// フレーム内で使い捨てる定数バッファ（バンプ／線形アロケータ）.
// ExecuteAndWait で毎フレーム GPU を待ち切る前提なので、多重バッファリングは不要で、
// フレーム先頭に Reset() するだけで安全に再利用できる.
class FrameConstantBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	uint8_t* mapped_ = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS base_ = 0;
	size_t capacity_ = 0;
	size_t offset_ = 0;

public:
	// UploadヒープのCBを1本確保して常時Mapする.
	void Initialize(ID3D12Device* device, size_t capacityBytes);

	// フレーム先頭で呼ぶ。使用位置を先頭へ戻す.
	void Reset() { offset_ = 0; }

	// data を書き込み、その GPU アドレスを返す（CBV 用に 256 バイト境界へ揃える）.
	D3D12_GPU_VIRTUAL_ADDRESS Allocate(const void* data, size_t size) {
		constexpr size_t kAlign = 256; // D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT.
		offset_ = (offset_ + (kAlign - 1)) & ~(kAlign - 1);

		// 枯渇時。assertはNDEBUGで消えるため、Releaseでも効く分岐で必ず食い止める.
		// 範囲外memcpy（ヒープ破壊）と不正GPUアドレス参照を防ぐのが目的.
		if (offset_ + size > capacity_) {
			assert(false && "フレーム定数バッファが枯渇しました。容量を増やしてください。");
			offset_ = 0; // 描画は乱れるが、バッファ内に留まるので落ちはしない.
		}

		std::memcpy(mapped_ + offset_, data, size);
		D3D12_GPU_VIRTUAL_ADDRESS addr = base_ + offset_;
		offset_ += size;
		return addr;
	}
};

} // namespace Cake
