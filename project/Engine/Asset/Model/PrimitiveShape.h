#pragma once
/*====================================
 *
 * エンジン内蔵の基本図形（三角形・矩形・立方体・球）の識別子と、
 * それらを「ファイルを持たないアセット」として扱うための固定情報。
 *
 * 基本図形は実ファイルが無いので .meta を置けず、GUID を発行できない。
 * そこで GUID をここへ直書きし、AssetDatabase が起動時に索引へ差し込む。
 * これにより MeshRendererComponent 側は特別扱いを一切せず、通常のモデルと
 * まったく同じ AssetRef<ModelHandle> で基本図形を指せる。
 *
 * 【変えてはいけないもの】
 *   kPrimitiveShapeGuids : 変えると保存済みシーンの参照が全部切れる.
 *   kPrimitiveShapeKeys  : ModelManager のキャッシュキー兼、索引上のパス.
 *
 * 図形を増やすときは enum と配列2本すべてに追記すること（要素数は
 * static_assert で突き合わせているので、書き忘れるとコンパイルエラーになる）。
 *
 * ====================================*/
#include <cstddef>
#include <cstdint>

#include "Engine/Foundation/Identity/Guid.h"

namespace Cake {

// 内蔵の基本図形.
enum class PrimitiveShape : uint8_t {
	Triangle,
	Quad,
	Cube,
	Sphere,
	Tetrahedron,

	Count, // 番兵。図形として使ってはいけない.
};

constexpr size_t kPrimitiveShapeCount = static_cast<size_t>(PrimitiveShape::Count);

// ModelManager のキャッシュキー兼、アセット索引上のパス.
// 実ファイルのパスと衝突しないよう "Builtin/" を接頭辞にしてある.
inline constexpr const char* kPrimitiveShapeKeys[kPrimitiveShapeCount] = {
	"Builtin/Triangle",
	"Builtin/Quad",
	"Builtin/Cube",
	"Builtin/Sphere",
	"Builtin/Tetrahedron",
};

// 内蔵アセットの目印。実ファイルのGUIDは乱数なので衝突は事実上起きない.
inline constexpr uint64_t kBuiltinAssetGuidHigh = 0xB011710000000000ull;

// 固定GUID。下位64bitが図形ごとの通し番号（0は無効値なので1始まり）.
inline constexpr Guid kPrimitiveShapeGuids[kPrimitiveShapeCount] = {
	Guid{kBuiltinAssetGuidHigh, 1},
	Guid{kBuiltinAssetGuidHigh, 2},
	Guid{kBuiltinAssetGuidHigh, 3},
	Guid{kBuiltinAssetGuidHigh, 4},
	Guid{kBuiltinAssetGuidHigh, 5},
};

inline const char* GetPrimitiveKey(PrimitiveShape shape) {
	return kPrimitiveShapeKeys[static_cast<size_t>(shape)];
}
inline Guid GetPrimitiveGuid(PrimitiveShape shape) {
	return kPrimitiveShapeGuids[static_cast<size_t>(shape)];
}

} // namespace Cake
