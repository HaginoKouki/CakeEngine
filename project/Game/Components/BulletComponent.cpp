#include "BulletComponent.h"

#include "Engine/Scene/Component/SphereColliderComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

void BulletComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::GameObject* object = ctx.scene.Find(self);
	if (object == nullptr) {
		return;
	}

	// 寿命切れ.
	lifeTime -= ctx.deltaTime;
	if (lifeTime <= 0.0f) {
		object->SetActive(false);
		return;
	}

	// 何かに当たったら仕舞う。相手が誰かは見ない（減算は相手の仕事）.
	if (const auto* collider = ctx.scene.GetComponent<Cake::SphereColliderComponent>(self)) {
		if (collider->IsHit()) {
			object->SetActive(false);
			return;
		}
	}

	object->GetTransform().GetLocalMutable().translate += velocity * ctx.deltaTime;
}
