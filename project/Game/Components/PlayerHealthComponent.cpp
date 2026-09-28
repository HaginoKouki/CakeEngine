#include "PlayerHealthComponent.h"

#include "Engine/Scene/Component/MeshRendererComponent.h"
#include "Engine/Scene/Component/SphereColliderComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

#include "Game/Components/DamageDealerComponent.h"
#include "Game/Components/PlayerShieldComponent.h"
#include "Game/Components/GameFeelComponent.h"

void PlayerHealthComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	if (!started) {
		hp = maxHp;
		started = true;
	}

	Cake::GameObject* object = ctx.scene.Find(self);
	if (object == nullptr) {
		return;
	}

	invincibleTimer -= ctx.deltaTime;

	// --- 被弾 ---
	if (invincibleTimer <= 0.0f) {
		if (const auto* collider = ctx.scene.GetComponent<Cake::SphereColliderComponent>(self)) {
			for (int i = 0; i < collider->hitCount; ++i) {
				const auto* dealer = ctx.scene.GetComponent<DamageDealerComponent>(collider->hits[i]);
				if (dealer == nullptr) {
					continue;
				}
				// 盾が同じフレームに受け止めた相手は、体に届いていないことにする。
				// 盾と体の判定球は少し重なっているので、深く踏み込まれた弾は両方が拾う.
				if (PlayerShieldComponent::IsBlockedThisFrame(ctx, self, collider->hits[i])) {
					continue;
				}

				hp -= dealer->damage;
				invincibleTimer = invincibleTime;
				blinkTimer = 0.0f;

				GameFeelComponent::RequestHitStop(ctx, 0.10f);
				GameFeelComponent::RequestShake(ctx, 0.5f, 0.3f);
				break;
			}
		}
	}

	// --- 点滅 ---
	if (auto* renderer = ctx.scene.GetComponent<Cake::MeshRendererComponent>(self)) {
		if (invincibleTimer > 0.0f) {
			blinkTimer += ctx.deltaTime;
			if (blinkTimer >= blinkInterval) {
				blinkTimer -= blinkInterval;
				renderer->visible = !renderer->visible;
			}
		} else {
			renderer->visible = true; // 消えたまま戻らない事故を防ぐ.
		}
	}

	// --- 死亡 ---
	if (hp <= 0) {
		object->SetActive(false);
	}
}
