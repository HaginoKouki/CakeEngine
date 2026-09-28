#pragma once
/*====================================
 *
 * まっすぐ飛ぶ弾。速度・寿命・威力だけを持つ。
 *
 * 生成も破棄もしない。PlayerShooterComponent が事前に作ったものを
 * SetActive で出し入れして使い回す（Update 中の構造変化を避けるため）。
 *
 * 当たったら自分を仕舞うだけで、ダメージの計算は当たられた側が行う。
 * どちらも自分のコライダーの hits を読むだけなので、依存が一方向にならない。
 *
 * 【親を持たせないこと】
 * translate をワールド座標として直接書き換えるため、ルート直下に置く前提。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
} // namespace Cake

struct BulletComponent {
	// --- 実行時。撃たれた瞬間に PlayerShooter が書き込む ---
	Cake::Vector3 velocity{}; // ワールド空間の秒速.
	float lifeTime = 0.0f;    // 残り秒数。0以下で仕舞う.

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(BulletComponent)
CAKE_REFLECT_END()
} // namespace Cake
