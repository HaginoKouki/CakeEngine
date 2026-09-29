#include "RenderSystem.h"
#include <vector>

#include "Engine/Asset/Database/AssetDatabase.h"

#include "Engine/Render/Renderer.h"

#include "Engine/Scene/Component/MeshRendererComponent.h"
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
			// 仮: インスタンシングの動作確認用。"ParticleTest" だけ10個ずらして描く.
			if (object->GetName() == "ParticleTest") {
				const Matrix4x4& base = object->GetTransform().GetWorldMatrix();
				std::vector<Matrix4x4> worlds;
				worlds.reserve(10);
				for (uint32_t i = 0; i < 10; ++i) {
					const float offset = 0.5f * static_cast<float>(i);
					worlds.push_back(base * Matrix4x4::MakeTranslateMatrix(Vector3(offset, offset, offset)));
				}
				renderer.DrawModelInstanced(meshRenderer.model.handle, worlds, cameraView, meshRenderer.material.handle);
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
}

} // namespace Cake
