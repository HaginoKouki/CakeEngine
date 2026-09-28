#pragma once
/*====================================
 *
 * プレイヤーの体力と、被弾後の無敵時間を管理するコンポーネント。
 *
 * 自分のコライダーの hits を読み、DamageDealerComponent を持つ相手が
 * 触れていたら減らす。相手が弾か敵本体かは区別しない。
 *
 * 【無敵時間が無いと即死する】
 * 判定は重なっている間ずっと報告し続けるので、これが無いと敵弾に触れた瞬間に
 * 数フレーム分まとめて削られる。ここは省略できない。
 *
 * 【点滅】
 * 無敵中は MeshRendererComponent の visible を反転させ続ける。
 * 無敵が効いていることを見せると同時に、被弾の手応えにもなる。
 *
 * 【死亡】
 * hp が尽きたら自分を非アクティブにするだけ。
 * 勝敗の進行は GameState 側が hp を読んで判断する（Day6）。
 *
 * ====================================*/
#include "Engine/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
} // namespace Cake

struct PlayerHealthComponent {
	int maxHp = 5;
	float invincibleTime = 1.0f; // 被弾後、無敵でいる秒数.
	float blinkInterval = 0.07f; // 点滅の周期.

	// --- 実行時 ---
	int hp = 0; // Play 開始時に maxHp で埋める.
	float invincibleTimer = 0.0f;
	float blinkTimer = 0.0f;
	bool started = false;

	bool IsDead() const { return started && hp <= 0; }

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(PlayerHealthComponent)
CAKE_PROPERTY_CLAMP(maxHp, "Max HP", 1.0f, 99.0f)
CAKE_PROPERTY_RANGE(invincibleTime, "Invincible Time", 0.0f, 3.0f)
CAKE_PROPERTY_RANGE(blinkInterval, "Blink Interval", 0.02f, 0.3f)
CAKE_PROPERTY(hp, "HP") // 実行中に減るのを見るため反映しておく.
CAKE_REFLECT_END()
} // namespace Cake
