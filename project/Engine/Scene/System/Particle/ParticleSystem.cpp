#include "ParticleSystem.h"

#include <algorithm>
#include <random>
#include <vector>
#include <cmath>

#include "Engine/Foundation/Math/Math.h"
#include "Engine/Scene/Component/Particle/ParticleSystemComponent.h"
#include "Engine/Scene/Component/Particle/ParticleForceFieldComponent.h"
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

// 変換が潰れていると見なす軸の長さ。これ以下だと逆行列が作れない.
constexpr float kMinAxisScale = 1.0e-6f;

// 拡大率が 0 の軸があるか（逆行列が作れないか）.
bool IsDegenerate(const Matrix4x4& world) {
	for (int i = 0; i < 3; ++i) {
		const Vector3 row{world.m[i][0], world.m[i][1], world.m[i][2]};
		if (Math::Length(row) <= kMinAxisScale) {
			return true;
		}
	}
	return false;
}

// 1フレーム分の Field の情報。毎フレーム集め直す.
struct FieldEntry {
	Matrix4x4 inverseWorld; // 粒の位置を Field のローカルへ直すため.
	Vector3 center;
	Vector3 halfSize;
	Vector3 acceleration; // ワールド空間.
};

// 有効な Field を集める。粒ごとに Field のプールを舐め直さないよう、フレームの最初に1回だけ行う.
std::vector<FieldEntry> CollectForceFields(Scene& scene) {
	std::vector<FieldEntry> fields;
	scene.GetPool<ParticleForceFieldComponent>()->ForEach(
		[&](GameObjectId owner, ParticleForceFieldComponent& field) {
			if (!field.enabled) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}
			const Matrix4x4& world = object->GetTransform().GetWorldMatrix();
			if (IsDegenerate(world)) {
				return; // 拡大率 0 では範囲が無く、逆行列も作れない.
			}

			FieldEntry entry;
			entry.inverseWorld = Math::Inverse(world);
			entry.center = field.center;
			// 負の size を入れられても壊れないよう絶対値で受ける（BoxCollider と同じ）.
			entry.halfSize = {std::fabs(field.size.x) * 0.5f, std::fabs(field.size.y) * 0.5f, std::fabs(field.size.z) * 0.5f};
			entry.acceleration = field.acceleration;
			fields.push_back(entry);
		}
	);
	return fields;
}

// 箱の中にいるか。Field のローカルに直せば、回転・拡大した箱でも軸ごとの比較で済む.
bool IsInside(const FieldEntry& field, const Vector3& worldPosition) {
	const Vector3 local = Math::TransformCoord(worldPosition, field.inverseWorld) - field.center;
	return std::fabs(local.x) <= field.halfSize.x && std::fabs(local.y) <= field.halfSize.y && std::fabs(local.z) <= field.halfSize.z;
}

} // namespace

void UpdateParticleSystems(Scene& scene, float deltaTime) {
	ParticleStorage& store = scene.GetParticleStore();

	// コンポーネントを外した・オブジェクトを破棄した分の粒を捨てる.
	store.RemoveIf([&](GameObjectId owner) {
		return scene.GetComponent<ParticleSystemComponent>(owner) == nullptr;
	});

	std::mt19937& randomEngine = GetRandomEngine();

	// Field はエミッターの数に関係なく、フレームに1回だけ集める.
	const std::vector<FieldEntry> fields = CollectForceFields(scene);

	scene.GetPool<ParticleSystemComponent>()->ForEach(
		[&](GameObjectId owner, ParticleSystemComponent& particleSystem) {
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}

			EmitterState& emitter = store.GetOrCreate(owner);
			std::vector<Particle>& particles = emitter.particles;

			// 粒の位置・速度はエミッターから見た値なので、Field（ワールド）との行き来にエミッターの行列を使う.
			const Matrix4x4& emitterWorld = object->GetTransform().GetWorldMatrix();
			// エミッターが潰れている（拡大率 0）と逆行列が作れないので、そのときは Field を効かせない.
			const bool applyFields = !fields.empty() && !IsDegenerate(emitterWorld);
			const Matrix4x4 emitterInverse = applyFields ? Math::Inverse(emitterWorld) : Matrix4x4::Identity;

			// Field の加速度を足し、経過時間を進めて移動する.
		    // 速度を先に変えてから位置を動かすので、加速度はそのフレームのうちに位置へ反映される.
			for (Particle& particle : particles) {
				if (applyFields) {
					const Vector3 worldPosition = Math::TransformCoord(particle.translate, emitterWorld);
					Vector3 worldAcceleration = Vector3::Zero;
					for (const FieldEntry& field : fields) {
						if (IsInside(field, worldPosition)) {
							worldAcceleration += field.acceleration;
						}
					}
					// 粒の速度はエミッターから見た向きなので、ワールドの加速度を変換してから足す.
					particle.velocity += Math::TransformNormal(worldAcceleration, emitterInverse) * deltaTime;
				}
				particle.currentTime += deltaTime;
				particle.translate += particle.velocity * deltaTime;
			}

			// 寿命に達した粒を消す.
			std::erase_if(particles, [](const Particle& particle) {
				return particle.currentTime >= particle.lifeTime;
			});

			// 一定間隔で粒を出す.
		    // 間隔 = 1回に出す数 ÷ 1秒あたりの数。発生が止まっている間はタイマーも進めない
		    // （止めている間に溜まった分が、再開した瞬間にまとめて出てしまうため）.
			if (particleSystem.isEnableEmission && particleSystem.rateOverTime > 0.0f) {
				const int burstCount = (std::max)(particleSystem.burstCount, 1);
				// 間隔が 0 以下や極端に短いと while が終わらないので、下限を設けて使う.
				const float interval = (std::max)(static_cast<float>(burstCount) / particleSystem.rateOverTime, particleSystem.kMinEmitInterval);
				emitter.emitTimer += deltaTime;
				while (emitter.emitTimer >= interval) {
					emitter.emitTimer -= interval;
					for (int i = 0; i < burstCount; ++i) {
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
