#include "GameFeelComponent.h"

#include <algorithm>

#include "Engine/Platform/Time/Time.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

namespace {

// xorshift。演出用なので質より軽さを優先する.
float NextSigned(unsigned int& state) {
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return (static_cast<float>(state & 0xFFFFu) / 32768.0f) - 1.0f; // -1〜1.
}

// シーン中の GameFeel を1つ見つけて処理を渡す.
template <class Fn>
void WithFeel(const Cake::UpdateContext& ctx, Fn&& fn) {
	bool done = false;
	ctx.scene.GetPool<GameFeelComponent>()->ForEach(
		[&](Cake::GameObjectId, GameFeelComponent& feel) {
			if (done) {
				return;
			}
			fn(feel);
			done = true;
		}
	);
}

} // namespace

void GameFeelComponent::RequestHitStop(const Cake::UpdateContext& ctx, float duration) {
	WithFeel(ctx, [&](GameFeelComponent& feel) {
		// 短い要求で上書きしない。長いほうを採用する.
		feel.hitStopTimer = std::max<float>(feel.hitStopTimer, duration);
	});
}

void GameFeelComponent::RequestShake(const Cake::UpdateContext& ctx, float power, float duration) {
	WithFeel(ctx, [&](GameFeelComponent& feel) {
		feel.shakePower = std::max<float>(power, feel.shakePower);
		feel.shakeDuration = std::max<float>(duration, feel.shakeTimer);
		feel.shakeTimer = feel.shakeDuration;
	});
}

void GameFeelComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::GameObject* object = ctx.scene.Find(self);
	if (object == nullptr) {
		return;
	}
	if (!started) {
		basePosition = object->GetTransform().GetLocalTransform().translate;
		started = true;
	}

	// 止まっている間も進む時計で数える.
	const float dt = ctx.unscaledDeltaTime;

	// --- ヒットストップ ---
	if (hitStopTimer > 0.0f) {
		hitStopTimer -= dt;
		ctx.time.SetTimeScale(hitStopTimer > 0.0f ? hitStopScale : 1.0f);
	}

	// --- シェイク ---
	Cake::Vector3 offset{};
	if (shakeTimer > 0.0f) {
		shakeTimer -= dt;
		// 残り時間の割合で減衰させる。終わりに向けて自然に収まる.
		const float t = (shakeDuration > 0.0f) ? std::clamp(shakeTimer / shakeDuration, 0.0f, 1.0f) : 0.0f;
		const float amount = shakePower * std::pow(t, shakeDecay);
		offset.x = NextSigned(randomState) * amount;
		offset.y = NextSigned(randomState) * amount;
		if (shakeTimer <= 0.0f) {
			shakePower = 0.0f;
		}
	}

	Cake::Transform& local = object->GetTransform().GetLocalMutable();
	local.translate = {
		basePosition.x + offset.x,
		basePosition.y + offset.y,
		basePosition.z,
	};
}
