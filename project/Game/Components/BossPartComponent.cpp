#include "BossPartComponent.h"

#include "Engine/Scene/Component/SphereColliderComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

#include "Game/Components/DamageDealerComponent.h"
#include "Game/Components/GameFeelComponent.h"

void BossPartComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	const auto* collider = ctx.scene.GetComponent<Cake::SphereColliderComponent>(self);
	if (collider == nullptr) {
		return;
	}

	hitCooldownTimer -= ctx.deltaTime;

	if (hitCooldownTimer <= 0.0f) {
		for (int i = 0; i < collider->hitCount; ++i) {
			if (const auto* dealer = ctx.scene.GetComponent<DamageDealerComponent>(collider->hits[i])) {
				hp -= dealer->damage;
				hitCooldownTimer = hitInterval;

				// 手応え。倒したときだけ強くする.
				const bool killed = (hp <= 0);
				GameFeelComponent::RequestHitStop(ctx, killed ? 0.12f : 0.05f);
				GameFeelComponent::RequestShake(ctx, killed ? 0.35f : 0.12f, killed ? 0.35f : 0.15f);
				break;
			}
		}
	}

	if (hp <= 0) {
		if (Cake::GameObject* object = ctx.scene.Find(self)) {
			object->SetActive(false);
		}
	}
}
