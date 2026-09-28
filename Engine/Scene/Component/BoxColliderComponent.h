#pragma once
/*====================================
 *
 * 直方体の当たり判定を持たせるコンポーネント。
 * 判定そのものは行わず、「大きさ・所属レイヤー・当たりに行く相手」と
 * 今フレームの衝突結果だけを持つ入れ物。
 * SphereColliderComponent の直方体版で、メンバの並びも読み方も球と揃えてある。
 *
 * 【向き付きの箱として扱う（OBB）】
 * ワールド行列から向きと大きさを取り出すため、回転させれば判定も傾き、
 * Transform を拡大すれば判定も太る。ギズモに出る緑の枠がそのまま判定範囲になる。
 * radius をワールド単位でそのまま使う球とは、スケールの扱いが違う点に注意。
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
 * 【押し戻しはしない】
 * 重なりを報告するだけで、位置は動かさない。
 * 壁ずりや接地が要るなら、hits を読んだ側が自分で戻すこと。
 *
 * 【メンバを増減したら】
 * CollisionSystem.cpp の CollectBoxes も一緒に直すこと。
 * あちらがここのメンバを直接読み書きしている。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

struct BoxColliderComponent {
	// 1つの箱が同時に拾える相手の数。溢れた分は捨てる.
	static constexpr int kMaxHits = 8;

	Vector3 center = Vector3::Zero; // Transform から見たローカルの中心.
	Vector3 size = Vector3::One;    // ローカルでの一辺の長さ（半分ではない）.

	int layer = 1; // 自分が何者か（ビットを1つ立てる）.
	int mask = -1; // どのレイヤーと当たるか（-1 で全部）.

	bool enabled = true;

	// --- ここから下は CollisionSystem が毎フレーム書き換える ---
	// リフレクションに載せていないので、保存もインスペクタ表示もされない.
	int hitCount = 0;
	GameObjectId hits[kMaxHits]{};

	bool IsHit() const { return hitCount > 0; }
};

CAKE_REFLECT(BoxColliderComponent)
CAKE_PROPERTY(enabled, "Enabled")
CAKE_PROPERTY(center, "Center")
CAKE_PROPERTY_MIN(size, "Size", 0.0f)
CAKE_PROPERTY(layer, "Layer")
CAKE_PROPERTY(mask, "Mask")
CAKE_REFLECT_END()

} // namespace Cake
