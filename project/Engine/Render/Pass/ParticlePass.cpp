#include "ParticlePass.h"

#include <vector>

#include "Engine/Render/Renderer.h"
#include "Engine/Scene/Component/ParticleSystemComponent.h"
#include "Engine/Scene/Component/ParticleRendererComponent.h"
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

			// 粒の transform はエミッターからの相対なので、エミッターのワールド行列を掛ける.
			const Matrix4x4& emitterWorld = object->GetTransform().GetWorldMatrix();
			instances.clear();
			instances.reserve(emitter->particles.size());
			for (const Particle& particle : emitter->particles) {
				const Matrix4x4 local = Matrix4x4::MakeAffineMatrix(
					particle.transform.scale, particle.transform.rotate, particle.transform.translate
				);
				instances.push_back(InstanceData{local * emitterWorld, particle.color});
			}

			ctx.renderer->DrawModelInstanced(
				particleRenderer->mesh.handle, instances, *ctx.view, particleRenderer->material.handle
			);
		}
	);
}

} // namespace Cake
