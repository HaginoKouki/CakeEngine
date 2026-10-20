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
			item.isTextureMissing = !spriteRenderer.texture.IsEmpty() && !spriteRenderer.texture.IsResolved();
			item.texture = spriteRenderer.texture.handle;
			item.world = object->GetTransform().GetWorldMatrix();
			item.color = spriteRenderer.color;
			item.size = spriteRenderer.size;

			out.Add(item, spriteRenderer.sortingOrder);
		}
	);
}
} // namespace Cake
