#pragma once
/*====================================
 *
 * PropertyType に応じた ImGui ウィジェットを1つ描く。
 * 「型ごとの switch」はこの1箇所だけに存在し、すべてのコンポーネントがこれを共有する。
 *
 * コンポーネントを追加してもここは触らない。
 * 触るのは PropertyType を新設したときだけ（そのときは必ず switch にも追加すること）。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef USE_IMGUI

#include "Engine/Foundation/Reflection/TypeInfo.h"

namespace Cake {

class Scene;
class AssetDatabase;

// 1つのプロパティを描く。値が変更されたら true.
bool DrawProperty(void* instance, const PropertyDesc& desc, Scene& scene, AssetDatabase& assetDatabase);

// 型情報を舐めて全プロパティを描く。いずれかが変更されたら true.
bool DrawProperties(void* instance, const TypeInfo& typeInfo, Scene& scene, AssetDatabase& assetDatabase);


#pragma region PropertyTypeごとの描画関数
bool DrawBoolProperty(const char* label, bool* value);
bool DrawIntProperty(const char* label, int* value, float minValue, float maxValue, float dragSpeed, NumericWidget widget);
bool DrawFloatProperty(const char* label, float* value, float minValue, float maxValue, float dragSpeed, NumericWidget widget);

bool DrawStringProperty(const char* label, std::string* value);

bool DrawVector2Property(const char* label, float* value, float minValue, float maxValue, float dragSpeed);
bool DrawVector3Property(const char* label, float* value, float minValue, float maxValue, float dragSpeed);
bool DrawVector4Property(const char* label, float* value, float minValue, float maxValue, float dragSpeed);

bool DrawTransformProperty(const char* label, Transform* value);
bool DrawColorProperty(const char* label, float* value);
bool DrawGradientProperty(const char* label, Gradient* value);

template <class HandleT>
bool DrawAssetRefProperty(const char* label, AssetRef<HandleT>* value, AssetDatabase& assetDatabase);
bool DrawEntityRefProperty(const char* label, GameObjectId* value, Scene& scene);

bool DrawUnknownProperty(const char* label);
#pragma endregion

} // namespace Cake

#endif // USE_IMGUI
