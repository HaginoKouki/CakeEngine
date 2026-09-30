#pragma once
/*====================================
 *
 * 毎フレーム自分を回転させるコンポーネント。仕組みの動作確認用。
 *
 * Update を持つので、TypeRegistry が自動的に毎フレームの呼び出し対象に加える。
 * 継承もインターフェース実装も不要で、この関数を書いただけで呼ばれる。
 *
 * Scene& を受け取るのは、自分の Transform へ辿るため。
 * ここがコンポーネントが Scene を知る唯一の経路になる。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
class Scene;
struct UpdateContext;
} // namespace Cake

struct PlayerComponent {

	float moveSpeed = 1.0f;

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(PlayerComponent)
CAKE_PROPERTY(moveSpeed, "MoveSpeed")
CAKE_REFLECT_END()
} // namespace Cake
