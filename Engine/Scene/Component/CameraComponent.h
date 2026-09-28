#pragma once
/*====================================
 *
 * シーンを写すカメラのパラメータを持つコンポーネント。
 * データのみを持ち、視点の組み立ては CameraSystem が行う。
 *
 * 【姿勢もアスペクト比も持たない】
 * 位置と向きは GameObject の Transform から取る。
 * アスペクト比は出力先（バックバッファやシーンRT）の解像度から毎フレーム計算する。
 * どちらもここに置くと二重管理になり、実際の描画結果と食い違う。
 *
 * ====================================*/
#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct CameraComponent {
	float fovYDegree = 60.0f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;
	bool enabled = true;

	// 複数ある場合にどれで描くか。小さいほど優先。同値ならプール順（＝生成順）.
	int priority = 0;
};

CAKE_REFLECT(CameraComponent)
CAKE_PROPERTY(enabled, "Enabled")
CAKE_PROPERTY_RANGE(fovYDegree, "Field of View", 1.0f, 179.0f)
CAKE_PROPERTY_MIN(nearClip, "Near", 0.001f)
CAKE_PROPERTY_DRAG(farClip, "Far", 0.01f, kUnboundedMax, 1.0f)
CAKE_PROPERTY(priority, "Priority")
CAKE_REFLECT_END()

} // namespace Cake
