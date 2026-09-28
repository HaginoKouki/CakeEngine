#pragma once

#include "Engine/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
} // namespace Cake

struct BossPartComponent {
	int hp = 5;
	float hitInterval = 0.2f;
	float hitCooldownTimer = 0.0f;
	bool isCore = false; // 中心（弱点）ならtrue。フェーズの頭数に入れない.

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};


namespace Cake {
CAKE_REFLECT(BossPartComponent)
CAKE_PROPERTY(hp, "HP")
CAKE_PROPERTY(isCore, "Is Core")
CAKE_REFLECT_END()
} // namespace Cake
