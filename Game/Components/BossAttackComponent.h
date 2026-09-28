#pragma once
/*====================================
 *
 * ボスの弾幕とフェーズ進行を担当するコンポーネント。ボスの親オブジェクトに付ける。
 *
 * 【フェーズは生きているパーツ数で決まる】
 * BossPartComponent のプールを毎フレーム数えるだけで、パーツ側との配線が要らない。
 * 1: 開幕（狙い撃ち） / 2: 半減（扇状） / 3: 全滅（全方位＋狙い撃ち）
 *
 * 【弾は作り置き】
 * PlayerShooterComponent と同じ方式。最初の1フレームでまとめて作り、
 * 以降は SetActive の切り替えだけで撃つ。
 * 構造が重複しているのは承知の上で、期日の都合により共通化していない。
 *
 * 【XY平面に閉じる】
 * 角度1つから (cos, sin, 0) を作って方向にする。Z は常に 0。
 *
 * 【core】
 * フェーズ3になったら core のコライダーを有効にして、初めて弱点を晒す。
 *
 * ====================================*/
#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Model/ModelHandle.h"
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
} // namespace Cake

struct BossAttackComponent {
	static constexpr int kMaxPool = 96;

	Cake::AssetRef<Cake::ModelHandle> bulletModel;
	Cake::GameObjectId target; // 狙う相手（プレイヤー）.
	Cake::GameObjectId core;   // フェーズ3で弱点になる中心.

	int poolSize = 80;
	float bulletSpeed = 9.0f;
	float bulletRadius = 0.35f;
	float bulletScale = 0.35f;
	float bulletLifeTime = 5.0f;

	float fireInterval = 1.2f; // フェーズ1の発射間隔.
	float phase2Scale = 0.70f; // フェーズ2での短縮率.
	float phase3Scale = 0.45f;

	int spreadCount = 3;        // フェーズ2の扇の本数.
	float spreadDegree = 22.0f; // 扇の隣り合う弾の角度差.
	int ringCount = 10;         // フェーズ3の全方位弾の数.

	// --- 実行時 ---
	bool spawned = false;
	int totalParts = 0;
	int phase = 1;
	float cooldown = 1.0f; // 開幕直後にいきなり撃たない.
	int poolCount = 0;
	Cake::GameObjectId pool[kMaxPool]{};

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);

private:
	void SpawnPool(const Cake::UpdateContext& ctx);
	void Fire(const Cake::UpdateContext& ctx, const Cake::Vector3& origin, float radian);
};

namespace Cake {
CAKE_REFLECT(BossAttackComponent)
CAKE_PROPERTY(bulletModel, "Bullet Model")
CAKE_PROPERTY(target, "Target")
CAKE_PROPERTY(core, "Core")
CAKE_PROPERTY_CLAMP(poolSize, "Pool Size", 1.0f, 96.0f)
CAKE_PROPERTY_RANGE(bulletSpeed, "Bullet Speed", 1.0f, 40.0f)
CAKE_PROPERTY_RANGE(bulletRadius, "Bullet Radius", 0.05f, 2.0f)
CAKE_PROPERTY_RANGE(bulletScale, "Bullet Scale", 0.05f, 2.0f)
CAKE_PROPERTY_RANGE(bulletLifeTime, "Bullet Life", 0.5f, 15.0f)
CAKE_PROPERTY_RANGE(fireInterval, "Fire Interval", 0.1f, 4.0f)
CAKE_PROPERTY_RANGE(phase2Scale, "Phase2 Scale", 0.1f, 1.0f)
CAKE_PROPERTY_RANGE(phase3Scale, "Phase3 Scale", 0.1f, 1.0f)
CAKE_PROPERTY_CLAMP(spreadCount, "Spread Count", 1.0f, 9.0f)
CAKE_PROPERTY_RANGE(spreadDegree, "Spread Degree", 2.0f, 60.0f)
CAKE_PROPERTY_CLAMP(ringCount, "Ring Count", 3.0f, 24.0f)
CAKE_PROPERTY(phase, "Phase") // 実行中の確認用.
CAKE_REFLECT_END()
} // namespace Cake
