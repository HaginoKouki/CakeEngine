#pragma once
/*====================================
 *
 * パーティクルを出すためのコンポーネント（Unity の ParticleSystem に相当）。
 * 粒の「出し方・動き方」の設定を持つ。描き方（メッシュ・マテリアル）は
 * ParticleRendererComponent が持つ（Unity の ParticleSystemRenderer と同じ分け方）。
 *
 * データのみを持つ。粒の生成・更新はコンポーネントに書かず、System 側に置く方針。
 * 粒の配列や発生タイマーもここには持たない（コンポーネントは standard-layout を保つため。ComponentPool.h を参照）。
 *
 * 【color】
 * 粒が生成されたときの色（Unity の startColor 相当）。
 * 全粒にまとめて掛ける色ではなく、生成時に各粒へコピーして、以後は粒ごとに持たせる想定。
 *
 * 【emitInterval】
 * 粒を1つ出す間隔（秒）。開始時は粒が無く、この間隔ごとに1つずつ増える。
 * 0 以下だと生成ループが終わらないため、System 側で kMinEmitInterval を下限にして使う。
 *
 * 【lifeTime】
 * 粒の寿命（秒）（Unity の startLifetime 相当）。生成時に各粒へコピーするので、
 * 途中で値を変えても、既に出ている粒の寿命は変わらない。
 *
 * 【依存】
 * このコンポーネントを付けると、ParticleRendererComponent も自動で付く
 * （TypeRegistry::AddComponent 経由の場合のみ。RequireComponent.h を参照）。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Component/ComponentRegistry/RequireComponent.h"

namespace Cake {

struct ParticleRendererComponent;

struct ParticleSystemComponent {
	// 発生間隔の下限（秒）。インスペクタの下限と System 側の安全策の両方で使う.
	static constexpr float kMinEmitInterval = 0.01f;

	// 粒が生成されたときの色.
	Vector4 color = Vector4::One;
	// 粒を1つ出す間隔（秒）.
	float emitInterval = 0.3f;
	// 粒の寿命（秒）.
	float lifeTime = 3.0f;
};

CAKE_REFLECT(ParticleSystemComponent)
CAKE_PROPERTY_COLOR(color, "Color")
CAKE_PROPERTY_MIN(emitInterval, "Emit Interval", ParticleSystemComponent::kMinEmitInterval)
CAKE_PROPERTY_MIN(lifeTime, "Life Time", 0.0f)
CAKE_REFLECT_END()
CAKE_REQUIRE_COMPONENTS(ParticleSystemComponent, ParticleRendererComponent)

} // namespace Cake
