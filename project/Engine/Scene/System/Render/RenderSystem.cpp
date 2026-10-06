#include "RenderSystem.h"

#include "Engine/Asset/Database/AssetDatabase.h"

#include "Engine/Render/Renderer.h"

#include "Engine/Scene/Component/MeshRendererComponent.h"
#include "Engine/Scene/Component/Particle/ParticleRendererComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {

void DrawMeshRenderers(Scene& scene, Renderer& renderer, const CameraView& cameraView) {
	scene.GetPool<MeshRendererComponent>()->ForEach(
		[&](GameObjectId owner, MeshRendererComponent& meshRenderer) {
			if (!meshRenderer.visible || !meshRenderer.model.IsResolved()) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}

			// material が未解決なら無効ハンドル＝上書きなし、としてそのまま渡す.
			renderer.DrawModel(
				meshRenderer.model.handle,
				object->GetTransform().GetWorldMatrix(),
				cameraView,
				meshRenderer.material.handle
			);
		}
	);
}

void ResolveSceneAssets(Scene& scene, AssetDatabase& assetDatabase) {
	scene.GetPool<MeshRendererComponent>()->ForEach(
		[&](GameObjectId, MeshRendererComponent& meshRenderer) {
			// 未設定（GUIDなし）は解決しようがないので飛ばす.
			if (!meshRenderer.model.IsEmpty() && !meshRenderer.model.IsResolved()) {
				meshRenderer.model.Resolve(assetDatabase);
			}
			// 未設定（GUIDなし）は解決しようがないので飛ばす.
			if (!meshRenderer.material.IsEmpty() && !meshRenderer.material.IsResolved()) {
				meshRenderer.material.Resolve(assetDatabase);
			}
		}
	);

	scene.GetPool<ParticleRendererComponent>()->ForEach(
		[&](GameObjectId, ParticleRendererComponent& particleRenderer) {
			// 未設定（GUIDなし）は解決しようがないので飛ばす.
			if (!particleRenderer.mesh.IsEmpty() && !particleRenderer.mesh.IsResolved()) {
				particleRenderer.mesh.Resolve(assetDatabase);
			}
			// 未設定（GUIDなし）は解決しようがないので飛ばす.
			if (!particleRenderer.material.IsEmpty() && !particleRenderer.material.IsResolved()) {
				particleRenderer.material.Resolve(assetDatabase);
			}
		}
	);
}

} // namespace Cake
