#pragma once
/*====================================
 *
 * シーンを照らすライトのパラメータを持つコンポーネント。
 * データのみを持ち、実際の収集と GPU への転送は LightSystem が行う。
 *
 * 【向きは持たない】
 * 照らす向きは GameObject の Transform から取り出す（world 行列のローカル +Z 軸）。
 * ここに direction を置くと Transform と二重管理になり、
 * Hierarchy で回しても向きが変わらないという食い違いが起きる。
 *
 * 【今は Directional のみ】
 * Point / Spot を足すときは LightType を追加する。
 * PropertyType に enum 用の型が無いため、型の切り替えUIはその時に併せて用意する。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct LightComponent {
	bool enabled = true;
	Vector4 color = Vector4::One;
	float intensity = 1.0f;

	// 複数ある場合にどれを使うか。小さいほど優先。同値ならプール順（＝生成順）.
	int priority = 0;
};

CAKE_REFLECT(LightComponent)
CAKE_PROPERTY(enabled, "Enabled")
CAKE_PROPERTY_COLOR(color, "Color")
CAKE_PROPERTY_MIN(intensity, "Intensity", 0.0f)
CAKE_PROPERTY(priority, "Priority")
CAKE_REFLECT_END()

} // namespace Cake
