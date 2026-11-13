#include "DemoEmissionBurstComponent.h"

#include "Engine/Scene/Component/Particle/ParticleSystemComponent.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

void DemoEmissionBurstComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::ParticleSystemComponent* particleSystem = ctx.scene.GetComponent<Cake::ParticleSystemComponent>(self);
	if (particleSystem == nullptr) {
		return;
	}

	// 押した瞬間に残り時間を入れ直す。押しっぱなしでは延びない.
	if (ctx.IsKeyDown(Cake::KeyCode::Space)) {
		remainingTime = duration;
	}

	// 残り時間がある間だけ発生させる。粒の生成そのものは、この後に呼ばれる UpdateParticleSystems が行う.
	particleSystem->isEnableEmission = remainingTime > 0.0f;
	if (remainingTime > 0.0f) {
		remainingTime -= ctx.deltaTime;
	}
}
