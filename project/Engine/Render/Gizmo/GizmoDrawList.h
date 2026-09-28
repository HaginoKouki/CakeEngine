#pragma once
/*====================================
 *
 * ギズモ（デバッグ用の線）を1フレーム分ためこむ入れ物。
 *
 * 箱・球・視錐台といった形は、すべてここで線分へ分解して詰め込む。
 * 出来上がるのは LINELIST 用の頂点列1本だけなので、
 * 何種類ギズモを出しても描画命令は1回で済む。
 *
 * 【ワールド空間で持つ】
 * 頂点はワールド座標で積む。シェーダへ渡すのは viewProjection だけになり、
 * 形ごとにワールド行列を積み直す必要がなくなる。
 *
 * 【形はコライダーの解釈に合わせる】
 * ギズモは「判定範囲の絵」なので、Transform の扱いを判定側と揃える。
 * 箱は回転もスケールも効かせ、球は中心だけ動かして半径は素のまま使う。
 * ここが食い違うと、見えている線を信じてデバッグできなくなる。
 *
 * 【寿命】
 * 毎フレーム Clear() してから積み直す。積む側（GizmoSystem）と
 * 出す側（Renderer）を分けるための受け渡し用バッファでしかない。
 *
 * 【形を増やすとき】
 * AddLine の組み合わせに落とせるなら、ここへ Add～ を1つ足すだけで済む。
 * 描画側もパスも触らなくてよい。
 *
 * ====================================*/
#include <cstdint>
#include <span>
#include <vector>

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Vector.h"

namespace Cake {

// ギズモ1頂点。Gizmo.VS.hlsl の入力（POSITION / COLOR）と1バイトも違ってはいけない.
struct GizmoVertex {
	Vector4 position{0.0f, 0.0f, 0.0f, 1.0f}; // ワールド空間.
	Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
};
static_assert(sizeof(GizmoVertex) == 32, "HLSLの入力レイアウトと不一致");

// 種類ごとの色をここへ集約する。散らばると「何色が何」が分からなくなる.
namespace GizmoColor {
inline const Vector4 kCollider{0.0f, 1.0f, 0.0f, 1.0f}; // 緑.
inline const Vector4 kCamera{1.0f, 1.0f, 1.0f, 1.0f};
inline const Vector4 kLight{1.0f, 0.84f, 0.0f, 1.0f};    // 黄.
inline const Vector4 kSelected{0.0f, 0.0f, 0.50f, 1.0f}; // 青.
inline const Vector4 kGrid{0.44f, 0.44f, 0.44f, 1.0f};
} // namespace GizmoColor

// Y=0 に敷く地面グリッドの見た目。項目が多いのでまとめて渡す.
struct GizmoGrid {
	bool enabled = true;
	float cellSize = 1.0f; // 1マスの幅.
	int halfCount = 100;   // 中心から片側に何マス伸ばすか（全体は 2*halfCount マス）.

	// グリッド全体の不透明度。他のギズモとは独立して調整できる.
	float opacity = 0.35f;
};

class GizmoDrawList {
private:
	std::vector<GizmoVertex> vertices_;

public:
	// 容量は保ったまま中身だけ捨てる（毎フレームの再確保を避ける）.
	void Clear() { vertices_.clear(); }

	void AddLine(const Vector3& a, const Vector3& b, const Vector4& color);
	void AddRay(const Vector3& origin, const Vector3& direction, const Vector4& color);

	void AddGrid(const GizmoGrid& grid);

	// world のローカル空間で center / size の直方体を12本の線で描く.
	// 8隅をそのまま world で移すので、回転もスケールも効く（BoxCollider の判定と同じ）.
	void AddWireBox(const Matrix4x4& world, const Vector3& center, const Vector3& size, const Vector4& color);

	// 直交する3枚の円で球を表す。
	//
	// 【world は中心を移すためだけに使う】
	// 半径はワールド単位でそのまま描き、world の回転もスケールも乗せない。
	// SphereColliderComponent の radius が同じ扱い（スケールが効かない）なので、
	// ここで拡大すると絵だけ膨らんで判定が付いてこない。
	// 見た目を Transform に追従させたくなったら、直す先は判定側であってここではない。
	void AddWireSphere(const Matrix4x4& world, const Vector3& center, float radius, const Vector4& color, uint32_t segments = 24);

	// 中心をワールド座標で直接渡す版。上の実体はこれを呼ぶだけ.
	void AddWireSphere(const Vector3& center, float radius, const Vector4& color, uint32_t segments = 24);

	// viewProjection の逆行列からNDCの8隅を戻して視錐台を描く.
	void AddFrustum(const Matrix4x4& viewProjection, const Vector4& color);

	// world の原点からXYZ軸を赤緑青で描く.
	void AddAxes(const Matrix4x4& world, float length);

	std::span<const GizmoVertex> GetVertices() const { return vertices_; }
	bool IsEmpty() const { return vertices_.empty(); }
};

} // namespace Cake
