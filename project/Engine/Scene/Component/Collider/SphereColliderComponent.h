#pragma once
/*====================================
 *
 * 球の当たり判定を持たせるコンポーネント。
 * 判定そのものは行わず、「半径・所属レイヤー・当たりに行く相手」と
 * 今フレームの衝突結果だけを持つ入れ物。
 *
 * 【結果は毎フレーム CollisionSystem が上書きする】
 * hits には自分と重なっている相手の GameObjectId が入る。
 * 各コンポーネントは自分の Update でこれを読んで反応する。
 * コールバックにしないのは、通知中に相手が消える問題を避けるため。
 *
 * 【1フレーム遅れる】
 * 判定は移動が終わった後に走るので、hits は前フレーム終了時点の重なり。
 * 60FPSで16msのずれなので、弾やボスの判定では見えない。
 *
 * 【スケールは効かない】
 * radius はワールド単位でそのまま使う。Transform を拡大しても判定は太らない。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

struct SphereColliderComponent {
	// 1つの球が同時に拾える相手の数。溢れた分は捨てる.
	static constexpr int kMaxHits = 8;

	float radius = 0.5f;
	Vector3 center{0.0f, 0.0f, 0.0f}; // Transform から見たローカルの中心.

	int layer = 1; // 自分が何者か（ビットを1つ立てる）.
	int mask = -1; // どのレイヤーと当たるか（-1 で全部）.

	bool enabled = true;

	// --- ここから下は CollisionSystem が毎フレーム書き換える ---
	// リフレクションに載せていないので、保存もインスペクタ表示もされない.
	int hitCount = 0;
	GameObjectId hits[kMaxHits]{};

	bool IsHit() const { return hitCount > 0; }
};

CAKE_REFLECT(SphereColliderComponent)
CAKE_PROPERTY(enabled, "Enabled")
CAKE_PROPERTY_MIN(radius, "Radius", 0.0f)
CAKE_PROPERTY(center, "Center")
CAKE_PROPERTY(layer, "Layer")
CAKE_PROPERTY(mask, "Mask")
CAKE_REFLECT_END()

} // namespace Cake
