#pragma once
/*====================================
 *
 * テクスチャへの参照を表すハンドル。
 * MaterialHandle / ModelHandle と同じ index + generation の作法に揃えつつ、
 * テクスチャは GPU ディスクリプタが主役で値のまま安定して持ち回れるため、
 * gpuHandle を同梱する。これにより描画側は TextureManager に問い合わせることなく、
 * ハンドルをそのまま SetGraphicsRootDescriptorTable に渡せる。
 *
 *   index      : TextureManager 内部プール（vector<TextureData>）の添字。実体の場所。
 *   generation : そのスロットが何代目か（将来アンロードで古いハンドルを検出する予約枠）。
 *   gpuHandle  : SRV の GPU ディスクリプタハンドル（描画で直接使う）。
 *
 * ※ SRVヒープ上の位置（srvIndex）は index とは別物で、TextureData 側が保持する。
 *
 * ====================================*/

#include <cstdint>
#include <d3d12.h>

namespace Cake {

struct TextureHandle {
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

	uint32_t index = kInvalidIndex;          // プール内の添字（実体の場所）.
	uint32_t generation = 0;                 // 世代番号（将来アンロード用。今は常に有効）.
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{}; // 描画で直接使う SRV ハンドル（併せ持ち）.

	bool IsValid() const { return index != kInvalidIndex; }

	bool operator==(const TextureHandle& rhs) const {
		return index == rhs.index && generation == rhs.generation;
	}
	bool operator!=(const TextureHandle& rhs) const { return !(*this == rhs); }
};

} // namespace Cake
