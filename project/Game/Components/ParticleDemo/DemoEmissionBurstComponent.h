#pragma once
/*====================================
 *
 * パーティクルデモ用。Space を押すたびに、短い時間だけ粒を発生させるコンポーネント。
 * エミッター（ParticleSystemComponent）と同じ GameObject に付けて使う。
 *
 * 粒は自分では作らない。ParticleSystemComponent::isEnableEmission を実行中に
 * 切り替えるだけで、生成は UpdateParticleSystems に任せる。
 * コンポーネントの Update は UpdateParticleSystems より先に呼ばれるので、
 * 押したフレームから粒が出る。
 *
 * 【出る粒の数】
 * duration の間、ParticleSystemComponent の Rate Over Time と Burst Count に従って出る。
 * 止めている間は発生タイマーも進まないため、押す前に粒が溜まることはない。
 *
 * 【remainingTime】
 * 実行中にだけ変わる値。Play / Stop のスナップショットで戻せるよう、反映ブロックに載せてある
 * （PlayModeController.h を参照）。
 *
 * 【依存】
 * このコンポーネントを付けると、ParticleSystemComponent も自動で付く
 * （TypeRegistry::AddComponent 経由の場合のみ。RequireComponent.h を参照）。
 *
 * ====================================*/
#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Component/ComponentRegistry/RequireComponent.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
struct ParticleSystemComponent;
} // namespace Cake

struct DemoEmissionBurstComponent {
	float duration = 0.12f;     // 1 回押すごとに粒を発生させる時間（秒）.
	float remainingTime = 0.0f; // 発生を止めるまでの残り時間（秒）。実行中にだけ変わる.

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(DemoEmissionBurstComponent)
CAKE_PROPERTY_MIN(duration, "Duration", 0.0f)
CAKE_PROPERTY_MIN(remainingTime, "Remaining Time", 0.0f)
CAKE_REFLECT_END()
CAKE_REQUIRE_COMPONENTS(DemoEmissionBurstComponent, ParticleSystemComponent)
} // namespace Cake
