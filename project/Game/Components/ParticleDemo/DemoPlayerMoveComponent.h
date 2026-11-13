#pragma once
/*====================================
 *
 * パーティクルデモ用。WASD で自分を XZ 平面上で動かすコンポーネント。
 * 子のオブジェクトにエミッター（ParticleSystemComponent）を付けておき、
 * 実行中にエミッターを動かしても粒が出続けることを確かめるために使う。
 *
 * 【キーの割り当て】
 * W / S : 奥 / 手前（+Z / -Z）
 * A / D : 左 / 右（-X / +X）
 *
 * 【移動範囲（rangeX / rangeZ）】
 * x が下限、y が上限。舞台の外へ出ないように、移動後の位置をこの範囲へ収める。
 *
 * 【粒の動きについて】
 * 粒の位置はエミッターからの相対で持たれているため（ParticleStorage.h を参照）、
 * エミッターを動かすと、既に出ている粒も一緒に動く。粒をその場に残す軌跡にはならない。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
} // namespace Cake

struct DemoPlayerMoveComponent {
	float moveSpeed = 5.0f;              // 移動の速さ（m/s）.
	Cake::Vector2 rangeX{-15.0f, 15.0f}; // X の移動範囲（x: 下限、y: 上限）.
	Cake::Vector2 rangeZ{-9.0f, 2.5f};   // Z の移動範囲（x: 下限、y: 上限）.

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(DemoPlayerMoveComponent)
CAKE_PROPERTY_MIN(moveSpeed, "Move Speed", 0.0f)
CAKE_PROPERTY(rangeX, "Range X (Min, Max)")
CAKE_PROPERTY(rangeZ, "Range Z (Min, Max)")
CAKE_REFLECT_END()
} // namespace Cake
