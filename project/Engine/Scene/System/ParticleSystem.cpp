#include "ParticleSystem.h"

#include <random>
#include <vector>

#include "Engine/Scene/Component/ParticleSystemComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Object/ParticleStorage.h"
#include "Engine/Scene/Scene.h"

namespace Cake {
namespace {

// エミッター1つあたりの粒の数.
constexpr size_t kParticleCount = 10;

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
	particle.transform.translate = {distribution(randomEngine), distribution(randomEngine), distribution(randomEngine)};
	particle.velocity = {distribution(randomEngine), distribution(randomEngine), distribution(randomEngine)};
	particle.color = particleSystem.color;
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

			std::vector<Particle>& particles = store.GetOrCreate(owner);

			// 足りない分を補充する.
			while (particles.size() < kParticleCount) {
				particles.push_back(SpawnParticle(particleSystem, randomEngine));
			}

			// 移動.
			for (Particle& particle : particles) {
				particle.transform.translate += particle.velocity * deltaTime;
			}
		}
	);
}

} // namespace Cake
