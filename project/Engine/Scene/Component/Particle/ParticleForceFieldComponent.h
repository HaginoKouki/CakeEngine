#pragma once
/*====================================
 *
 * 範囲内の粒に加速度を与えるコンポーネント（Unity の Particle System Force Field に相当）。
 * 風のように「この箱の中にいる粒を一定の向きへ押し流す」のに使う。
 * エミッター（ParticleSystemComponent）とは別の GameObject に付けて置く想定。
 *
 * データのみを持つ。粒への適用は UpdateParticleSystems が行う。
 *
 * 【範囲（center / size）】
 * BoxColliderComponent と同じ読み方。Transform から見たローカルの箱で、
 * オブジェクトを回転・拡大すると範囲も一緒に回転・拡大する。ギズモの水色の枠がそのまま範囲になる。
 *
 * 【acceleration】
 * ワールド空間の加速度（m/s²）。風の向きと強さ。
 * Field を回転させても、範囲の箱は回るが風の向きは変わらない。
 * 複数の Field が重なった場所では足し合わせる。
 *
 * 【効く相手】
 * すべての ParticleSystemComponent の粒に効く（エミッターごとの絞り込みは無い）。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct ParticleForceFieldComponent {
	bool enabled = true;

	Vector3 center = Vector3::Zero; // Transform から見たローカルの中心.
	Vector3 size = Vector3::One;    // ローカルでの一辺の長さ（半分ではない）.

	// ワールド空間の加速度（m/s²）。風の向きと強さ.
	Vector3 acceleration = Vector3{1.0f, 0.0f, 0.0f};
};

CAKE_REFLECT(ParticleForceFieldComponent)
CAKE_PROPERTY(enabled, "Enabled")
CAKE_PROPERTY(center, "Center")
CAKE_PROPERTY_MIN(size, "Size", 0.0f)
CAKE_PROPERTY(acceleration, "Acceleration")
CAKE_REFLECT_END()

} // namespace Cake
