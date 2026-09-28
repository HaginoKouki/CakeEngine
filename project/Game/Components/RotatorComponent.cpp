#include "RotatorComponent.h"

#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"


void RotatorComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::GameObject* object = ctx.scene.Find(self);
	if (object == nullptr) {
		return;
	}

	// GetLocalMutable はダーティを立てるので、ワールド行列は次の UpdateTransforms で再計算される.
	Cake::Transform& local = object->GetTransform().GetLocalMutable();
	local.rotate += speed * ctx.deltaTime;
}

