#pragma once
/*====================================
 *
 * シーンの階層ウィンドウ（Hierarchy）と、選択オブジェクトの詳細ウィンドウ（Inspector）。
 *
 * Inspector はコンポーネントの型を一切知らない。TypeRegistry から型情報を引き、
 * PropertyDrawer に渡すだけ。コンポーネントを追加してもこのファイルは触らない。
 *
 * 選択状態は呼び出し側（Editor）が持つ。両ウィンドウで共有するため参照で受け取る。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef USE_IMGUI

#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Scene;
class SceneManager;
class AssetDatabase;

struct EditorDrawContext;

// 階層ツリーを描く。クリックで selected が書き換わる.
void DrawHierarchyWindow(Scene& scene, GameObjectId& selected, SceneManager& sceneManager);

// 選択中オブジェクトの名前・Transform・全コンポーネントを描く.
void DrawInspectorWindow(Scene& scene, GameObjectId& selected, EditorDrawContext& ctx);

} // namespace Cake

#endif // USE_IMGUI
