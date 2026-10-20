#include "SpriteSystem.h"

#include "Engine/Render/Sprite/SpriteDrawList.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Component/SpriteRendererComponent.h"
#include "Engine/Scene/Scene.h"

namespace Cake {
void CollectScreenSprites(Scene& scene, SpriteDrawList& out) {
	scene.GetPool<SpriteRendererComponent>()->ForEach(
		[&](GameObjectId owner, SpriteRendererComponent& spriteRenderer) {
			if (!spriteRenderer.visible) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}

			SpriteDrawItem item;
			item.world = object->GetTransform().GetWorldMatrix();

			out.Add(item, spriteRenderer.sortingOrder);
		}
	);
}
} // namespace Cake
