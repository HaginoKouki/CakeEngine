#include "PlayerComponent.h"

#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

void PlayerComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::GameObject* object = ctx.scene.Find(self);
	if (object == nullptr) {
		return;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::A)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.translate.x -= moveSpeed * ctx.deltaTime;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::D)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.translate.x += moveSpeed * ctx.deltaTime;
	}
}
