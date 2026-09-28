#pragma once
/*====================================
 *
 * フレーム内で使い捨てる頂点バッファ（バンプアロケータ）。
 * FrameConstantBuffer の頂点版で、ギズモのように
 * 「毎フレーム中身が変わり、GPUへ渡したら用済み」になる形を置く場所。
 *
 * ExecuteAndWait で毎フレーム GPU を待ち切る前提なので多重バッファリングは不要。
 * フレーム先頭に Reset() するだけで安全に使い回せる。
 *
 * 【枯渇時】
 * SizeInBytes = 0 の VBV を返す。呼び出し側はこれを見て描画を諦めること。
 * 落とさずに「そのフレームだけ出ない」で済ませるための約束。
 *
 * ====================================*/
#include <cstdint>
#include <cstring>

#include <d3d12.h>
#include <wrl.h>

namespace Cake {

class FrameVertexBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	uint8_t* mapped_ = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS base_ = 0;
	size_t capacity_ = 0;
	size_t offset_ = 0;

public:
	// Uploadヒープに1本確保して常時Mapする.
	void Initialize(ID3D12Device* device, size_t capacityBytes);

	// フレーム先頭で呼ぶ。使用位置を先頭へ戻す.
	void Reset() { offset_ = 0; }

	// data を書き込み、そのまま IASetVertexBuffers へ渡せる VBV を返す.
	D3D12_VERTEX_BUFFER_VIEW Allocate(const void* data, size_t sizeInBytes, uint32_t stride);
};

} // namespace Cake
