#include "DemoPlayerMoveComponent.h"

#include <algorithm>

#include "Engine/Foundation/Math/Math.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/System/UpdateContext.h"

void DemoPlayerMoveComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::GameObject* object = ctx.scene.Find(self);
	if (object == nullptr) {
		return;
	}

	// 押されているキーを、XZ 平面上の向きにまとめる.
	Cake::Vector3 direction = Cake::Vector3::Zero;
	if (ctx.IsKeyHeld(Cake::KeyCode::W)) {
		direction.z += 1.0f;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::S)) {
		direction.z -= 1.0f;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::D)) {
		direction.x += 1.0f;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::A)) {
		direction.x -= 1.0f;
	}

	// 入力が無い（または打ち消し合っている）フレームは Transform に触らない.
	// GetLocalMutable は呼んだ時点でダーティを立てるため.
	if (direction.x == 0.0f && direction.z == 0.0f) {
		return;
	}

	// 斜めに動いても速さが変わらないよう、長さを 1 にそろえる.
	direction = Cake::Math::Normalize(direction);

	Cake::Transform& local = object->GetTransform().GetLocalMutable();
	local.translate += direction * (moveSpeed * ctx.deltaTime);

	// 下限と上限が逆に入力されていても動くよう、小さい方を下限として扱う（std::clamp は下限 > 上限だと未定義動作になる）.
	local.translate.x = std::clamp(local.translate.x, (std::min)(rangeX.x, rangeX.y), (std::max)(rangeX.x, rangeX.y));
	local.translate.z = std::clamp(local.translate.z, (std::min)(rangeZ.x, rangeZ.y), (std::max)(rangeZ.x, rangeZ.y));
}
