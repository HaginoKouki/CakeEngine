#pragma once
/*====================================
 *
 * Inspector ウィンドウの最下部に描く、選択オブジェクトのマテリアル編集セクション。
 *
 * SceneInspector 本体をコンポーネントの型に依存させないため、
 * 「MeshRenderer を見てマテリアルを引く」という型固有の知識はこのファイルに閉じ込める。
 * SceneInspector からは1行呼ぶだけで済む。
 *
 * 【何を描くか】
 * MeshRenderer の material（このオブジェクトだけの上書き）が設定されていればそれ1枚。
 * 未設定ならモデル側の既定マテリアルを全スロットぶん並べる。
 * 後者は MaterialOrigin::Embedded なので MaterialEditor 側で自動的に読み取り専用になる。
 *
 * 【重複の排除】
 * 同じマテリアルが複数スロットから参照されるのは普通なので、
 * ハンドルの index で重複を除いてから並べる（同じUIが何枚も出ないようにする）。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef USE_IMGUI

#include "Engine/Editor/EditorDrawContext.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Scene;

// Inspector の ImGui::Begin / End の内側から呼ぶこと.
void DrawInspectorMaterialSection(Scene& scene, GameObjectId selected, EditorDrawContext& ctx);

} // namespace Cake

#endif // USE_IMGUI
