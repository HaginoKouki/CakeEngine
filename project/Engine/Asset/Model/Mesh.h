#pragma once
/*====================================
 *
 * 描画の最小単位となるジオメトリ構造。
 * Mesh はサブメッシュ群とマテリアルスロット（MaterialHandle の配列）を持ち、
 * SubMesh は頂点データと「親 Mesh の何番のスロットを使うか」を持つ。
 * Material 実体は MaterialManager が、Mesh 実体は ModelManager が所有する。
 * Mesh はマテリアルを生ポインタではなくハンドルで参照する（Resolve 経由でアクセス）。
 *
 * ====================================*/

#include <cstdint>
#include <string>
#include <vector>

#include <d3d12.h>
#include <wrl.h> // Microsoft::WRL::ComPtr.

#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Asset/Material/MaterialHandle.h"

namespace Cake {

// 頂点データ.
struct VertexData {
	Cake::Vector4 position;
	Cake::Vector2 texcoord;
	Cake::Vector3 normal;
};

// サブメッシュ：頂点データ群 ＋ 使用するマテリアルスロット番号.
// GPU頂点バッファはロード時に1回だけ作られ、全描画で共有される.
struct SubMesh {
	std::vector<VertexData> vertices; // CPU側頂点（衝突判定・AABB算出などに再利用可能）.

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource; // GPU頂点バッファ.
	D3D12_VERTEX_BUFFER_VIEW vbv{};
	uint32_t vertexCount = 0;

	uint32_t materialSlot = 0; // 親 Mesh の materialSlots への添字.
};

// メッシュ：サブメッシュ群 ＋ マテリアルスロット.
// materialSlots が指す Material 実体は MaterialManager が所有する（Mesh は非所有・ハンドル参照）.
struct Mesh {
	std::string name;
	std::vector<SubMesh> subMeshes;
	std::vector<MaterialHandle> materialSlots; // スロット番号 → マテリアルハンドル.

	// スロット数（= materialSlots.size()）.
	uint32_t GetSlotCount() const { return static_cast<uint32_t>(materialSlots.size()); }
};

} // namespace Cake
