#include "ParticlePass.h"

#include <vector>

#include "Engine/Render/Renderer.h"
#include "Engine/Scene/Component/Particle/ParticleSystemComponent.h"
#include "Engine/Scene/Component/Particle/ParticleRendererComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Object/ParticleStorage.h"
#include "Engine/Scene/Scene.h"

namespace Cake {

void ParticlePass::Execute(RenderContext& ctx) {
	Scene& scene = *ctx.scene;
	const ParticleStorage& store = scene.GetParticleStore();

	// エミッターごとに使い回す.
	std::vector<InstanceData> instances;

	scene.GetPool<ParticleSystemComponent>()->ForEach(
		[&](GameObjectId owner, ParticleSystemComponent&) {
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}
			// 依存による自動追加は TypeRegistry 経由のときだけなので、Renderer が無い場合もある.
			const ParticleRendererComponent* particleRenderer =
				scene.GetComponent<ParticleRendererComponent>(owner);
			if (particleRenderer == nullptr || !particleRenderer->mesh.IsResolved()) {
				return;
			}
			const EmitterState* emitter = store.Find(owner);
			if (emitter == nullptr || emitter->particles.empty()) {
				return;
			}

			// ビルボード用の回転。平行移動とスケールを除いたカメラの回転そのもの.
			// 視点（ゲームビュー／シーンビュー）ごとに違うので、毎回 ctx.view から取る.
			const Matrix4x4& billboard = ctx.view->rotationMatrix;
			// 粒の位置はエミッターからの相対なので、位置だけエミッターのワールド行列で動かす.
			// 向きと大きさはエミッターの回転・スケールの影響を受けない.
			const Matrix4x4& emitterWorld = object->GetTransform().GetWorldMatrix();
			instances.clear();
			instances.reserve(emitter->particles.size());
			for (const Particle& particle : emitter->particles) {
				const Vector3 worldPosition = Vector3::Transform(particle.translate, emitterWorld);
				// スケール → 画面内の回転 → カメラへ向ける → 位置へ移動.
				const Matrix4x4 world =
					Matrix4x4::MakeScalingMatrix({particle.scale.x, particle.scale.y, 1.0f}) *
					Matrix4x4::MakeZRotationMatrix(particle.rotate) *
					billboard *
					Matrix4x4::MakeTranslateMatrix(worldPosition);
				instances.push_back(InstanceData{world, particle.drawColor});
			}


			ctx.renderer->DrawModelInstanced(
				particleRenderer->mesh.handle, instances, *ctx.view, particleRenderer->material.handle
			);
		}
	);
}

} // namespace Cake
