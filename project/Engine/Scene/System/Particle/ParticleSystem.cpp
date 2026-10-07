#include "ParticleSystem.h"

#include <algorithm>
#include <random>
#include <vector>

#include "Engine/Scene/Component/Particle/ParticleSystemComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Object/ParticleStorage.h"
#include "Engine/Scene/Scene.h"

namespace Cake {
namespace {

// 生成時の位置・速度の範囲（各軸）.
constexpr float kSpawnRange = 1.0f;

std::mt19937& GetRandomEngine() {
	static std::mt19937 randomEngine{std::random_device{}()};
	return randomEngine;
}

// 粒を1つ作る.
Particle SpawnParticle(const ParticleSystemComponent& particleSystem, std::mt19937& randomEngine) {
	std::uniform_real_distribution<float> distribution(-kSpawnRange, kSpawnRange);

	Particle particle;
	particle.translate = Vector3{distribution(randomEngine), distribution(randomEngine), distribution(randomEngine)};
	particle.velocity = Vector3{distribution(randomEngine), distribution(randomEngine), distribution(randomEngine)} * particleSystem.startSpeed;
	particle.startColor = particleSystem.startColor;
	particle.drawColor = particleSystem.startColor;
	particle.lifeTime = particleSystem.startLifeTime;
	particle.currentTime = 0.0f;
	return particle;
}

} // namespace

void UpdateParticleSystems(Scene& scene, float deltaTime) {
	ParticleStorage& store = scene.GetParticleStore();

	// コンポーネントを外した・オブジェクトを破棄した分の粒を捨てる.
	store.RemoveIf([&](GameObjectId owner) {
		return scene.GetComponent<ParticleSystemComponent>(owner) == nullptr;
	});

	std::mt19937& randomEngine = GetRandomEngine();

	scene.GetPool<ParticleSystemComponent>()->ForEach(
		[&](GameObjectId owner, ParticleSystemComponent& particleSystem) {
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}

			EmitterState& emitter = store.GetOrCreate(owner);
			std::vector<Particle>& particles = emitter.particles;

			// 経過時間を進めて移動する.
			for (Particle& particle : particles) {
				particle.currentTime += deltaTime;
				particle.translate += particle.velocity * deltaTime;
			}

			// 寿命に達した粒を消す.
			std::erase_if(particles, [](const Particle& particle) {
				return particle.currentTime >= particle.lifeTime;
			});

			// 一定間隔で粒を出す.
		    // 間隔が 0 以下だと while が終わらないので、下限を設けて使う.
			const float interval = particleSystem.burstCount / particleSystem.rateOverTime;
			emitter.emitTimer += deltaTime;
			if (particleSystem.isEnableEmission) {
				while (emitter.emitTimer >= interval) {
					emitter.emitTimer -= interval;
					for (int i = 0; i < static_cast<int>(particleSystem.burstCount); ++i) {
						particles.push_back(SpawnParticle(particleSystem, randomEngine));
					}
				}
			}

			// 寿命に合わせて色を変える.
			if (particleSystem.isEnableColorOverLifetime) {
				// キーの並びは保存順のままなので、エミッターごとに1回だけ並べ替えてから使う.
				const Gradient gradient = particleSystem.colorOverLifetime.Sorted();
				for (Particle& particle : particles) {
					// 年齢（0〜1）。寿命 0 の粒は割り算せずに終端の色にする.
					const float age = (particle.lifeTime > 0.0f) ? particle.currentTime / particle.lifeTime : 1.0f;
					particle.drawColor = Vector4::Multiply(particle.startColor, gradient.Evaluate(age));
				}
			} else {
				// 途中で無効にしたときに元の色へ戻すため、無効でも毎フレーム書き戻す.
				for (Particle& particle : particles) {
					particle.drawColor = particle.startColor;
				}
			}
		}
	);
}

} // namespace Cake
