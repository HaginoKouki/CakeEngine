#include "BossAttackComponent.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "Engine/Scene/Component/MeshRendererComponent.h"
#include "Engine/Scene/Component/SphereColliderComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

#include "Game/Components/BossPartComponent.h"
#include "Game/Components/BulletComponent.h"
#include "Game/Components/DamageDealerComponent.h"
#include "Game/GameLayer.h"

namespace {

// 角度からXY平面の向きを作る.
Cake::Vector3 FromAngle(float radian) {
	return {std::cos(radian), std::sin(radian), 0.0f};
}

// 生きているボスパーツを数える。別プールなので走査中に触っても安全.
int CountAliveParts(const Cake::UpdateContext& ctx) {
	int alive = 0;
	ctx.scene.GetPool<BossPartComponent>()->ForEach(
		[&](Cake::GameObjectId, BossPartComponent& part) {
			// 中心は数えない。
		    // 入れると alive が 0 にならず、フェーズ3へ移れないまま詰む.
			if (part.isCore) {
				return;
			}
			if (part.hp > 0) {
				++alive;
			}
		}
	);
	return alive;
}
// コアが1つでも生きていれば true。コアが存在しなければ false.
bool IsAliveCore(const Cake::UpdateContext& ctx) {
	bool alive = false;
	ctx.scene.GetPool<BossPartComponent>()->ForEach(
		[&](Cake::GameObjectId, BossPartComponent& part) {
			if (part.isCore && part.hp > 0) {
				alive = true;
			}
		}
	);
	return alive;
}

} // namespace

void BossAttackComponent::SpawnPool(const Cake::UpdateContext& ctx) {
	poolCount = std::clamp(poolSize, 1, kMaxPool);
	const Cake::GameObjectId parent = ctx.scene.CreateGameObject("BossBullets");

	for (int i = 0; i < poolCount; ++i) {
		const Cake::GameObjectId id = ctx.scene.CreateGameObject("BossBullet", parent);

		// ポインタは取ったその場で使い切る（次の呼び出しで無効になりうる）.
		if (Cake::GameObject* object = ctx.scene.Find(id)) {
			object->SetActive(false);
		}
		if (auto* renderer = ctx.scene.AddComponent<Cake::MeshRendererComponent>(id)) {
			renderer->model = bulletModel;
		}
		if (auto* collider = ctx.scene.AddComponent<Cake::SphereColliderComponent>(id)) {
			collider->radius = bulletRadius;
			collider->layer = GameLayer::kEnemyBullet;
			collider->mask = GameLayer::kPlayer | GameLayer::kPlayerGuard;
		}
		ctx.scene.AddComponent<BulletComponent>(id);
		ctx.scene.AddComponent<DamageDealerComponent>(id);

		pool[i] = id;
	}
}

void BossAttackComponent::Fire(const Cake::UpdateContext& ctx, const Cake::Vector3& origin, float radian) {
	// 眠っている弾を1つ探す。全部飛んでいたらこの1発は諦める.
	Cake::GameObjectId bulletId;
	for (int i = 0; i < poolCount; ++i) {
		const Cake::GameObject* object = ctx.scene.Find(pool[i]);
		if (object != nullptr && !object->IsActive()) {
			bulletId = pool[i];
			break;
		}
	}
	if (!bulletId.IsValid()) {
		return;
	}

	if (Cake::GameObject* bullet = ctx.scene.Find(bulletId)) {
		Cake::Transform& local = bullet->GetTransform().GetLocalMutable();
		local.translate = origin;
		local.scale = {bulletScale, bulletScale, bulletScale};
		bullet->SetActive(true);
	}
	if (auto* component = ctx.scene.GetComponent<BulletComponent>(bulletId)) {
		component->velocity = FromAngle(radian) * bulletSpeed;
		component->lifeTime = bulletLifeTime;
	}
}

void BossAttackComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	if (!spawned) {
		SpawnPool(ctx);
		totalParts = CountAliveParts(ctx);
		spawned = true;
	}

	// --- フェーズ ---
	const int alive = CountAliveParts(ctx);
	if (alive <= 0) {
		if (IsAliveCore(ctx)) {
			phase = 3;
		} else {
			return;
		}
	} else if (totalParts > 0 && alive * 2 <= totalParts) {
		phase = 2;
	} else {
		phase = 1;
	}

	// フェーズ3で初めて中心に判定を持たせる.
	if (auto* coreCollider = ctx.scene.GetComponent<Cake::SphereColliderComponent>(core)) {
		coreCollider->enabled = (phase >= 3);
	}

	// --- 発射 ---
	cooldown -= ctx.deltaTime;
	if (cooldown > 0.0f) {
		return;
	}

	float interval = fireInterval;
	if (phase == 2) {
		interval *= phase2Scale;
	} else if (phase == 3) {
		interval *= phase3Scale;
	}
	cooldown = interval;

	const Cake::GameObject* boss = ctx.scene.Find(self);
	if (boss == nullptr) {
		return;
	}
	const Cake::Matrix4x4& world = boss->GetTransform().GetWorldMatrix();
	const Cake::Vector3 origin{world.m[3][0], world.m[3][1], 0.0f};

	// プレイヤーへの角度。見失っていたら真下へ撃つ.
	float aim = -std::numbers::pi_v<float> * 0.5f;
	if (const Cake::GameObject* player = ctx.scene.Find(target)) {
		const Cake::Matrix4x4& playerWorld = player->GetTransform().GetWorldMatrix();
		aim = std::atan2(playerWorld.m[3][1] - origin.y, playerWorld.m[3][0] - origin.x);
	}

	const float step = spreadDegree * std::numbers::pi_v<float> / 180.0f;

	switch (phase) {
		case 1:
			Fire(ctx, origin, aim);
			break;

		case 2:
			// 中央を基準に左右対称へ広げる.
			for (int i = 0; i < spreadCount; ++i) {
				const float offset = (static_cast<float>(i) - (spreadCount - 1) * 0.5f) * step;
				Fire(ctx, origin, aim + offset);
			}
			break;

		case 3: {
			const float unit = 2.0f * std::numbers::pi_v<float> / static_cast<float>(ringCount);
			for (int i = 0; i < ringCount; ++i) {
				Fire(ctx, origin, unit * static_cast<float>(i));
			}
			Fire(ctx, origin, aim); // 全方位だけだと避けやすいので1発足す.
			break;
		}
	}
}
