#pragma once
/*====================================
 *
 * SpriteRendererComponent の座標・回転角・色・表示サイズを、
 * ゲーム側のコードから参照・操作できることを確かめるための確認用コンポーネント。
 * SpriteRendererComponent と同じ GameObject に付けて、Play 中にキーで操作する。
 *
 * 【キーの割り当て】
 * 矢印キー : 移動（Transform の translate.xy）
 * Q / E    : 回転（Transform の rotate.z。E で画面上は時計回り）
 * Space    : 色を白と赤で切り替える（不透明度は保つ）
 * Z / X    : 表示サイズを縮小 / 拡大する（0 未満にはしない）
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
class Scene;
struct UpdateContext;
} // namespace Cake

struct SpriteTesterComponent {
	float speed = 200.0f; // 移動の速さ（ピクセル/秒）.
	float rotate = 3.0f;  // 回転の速さ（ラジアン/秒）.
	float size = 50.0f;   // 表示サイズを変える速さ（ピクセル/秒）.

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(SpriteTesterComponent)
CAKE_PROPERTY(speed, "Speed")
CAKE_PROPERTY(rotate, "Rotate")
CAKE_PROPERTY(size, "Size")
CAKE_REFLECT_END()
} // namespace Cake
