#pragma once
/*====================================
 *
 * エディタ機能の統括クラス。
 * SceneManager が持つ現在のシーンを更新・描画し、その上に ImGui のエディタUI を被せる。
 * ゲーム固有のクラスを一切知らないため、ユーザーのゲームコードを差し替えても影響を受けない。
 *
 * このクラスは ENABLE_EDITOR が定義されているときのみ存在する。
 *
 *【ここが持つもの・持たないもの】
 * UI そのものは各ウィンドウクラス（MenuBar / ToolBar / SceneWindow / GameWindow）へ委譲する。
 * このクラスに残すのは、ウィンドウをまたぐ判断だけ。
 *   - エディタ全体で共有する状態（選択中オブジェクト、再生状態、入力先）
 *   - フレーム境界（ImGui の Begin/End、DockSpace）
 *   - RT の作り直し（深度バッファを2ビューで共用しているため、両方を見てから決める）
 * 各ウィンドウが持つのは、そのウィンドウ固有の状態だけに留めること。
 *
 *【破棄順】
 * main.cpp では engine より後に宣言すること。
 * ImGui のシャットダウンと RT の解放が、デバイス破棄より先に走る必要がある。
 * GPU アイドルは Renderer::EndFrame の ExecuteAndWait が毎フレーム保証している。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include <Windows.h>

#include "externals/imgui/imgui.h"
#include "externals/imgui/ImGuizmo.h"

#include "Engine/Platform/Time/Time.h"

#include "Engine/Editor/EditorDrawContext.h"
#include "Engine/Editor/ImGui/ImGuiManager.h"
#include "Engine/Editor/PlayMode/PlayModeController.h"
#include "Engine/Editor/Settings/EditorCache.h"
#include "Engine/Editor/Settings/EditorPreferences.h"
#include "Engine/Editor/Viewport/GameWindow.h"
#include "Engine/Editor/Viewport/SceneWindow.h"
#include "Engine/Editor/MenuBar/MenuBar.h"
#include "Engine/Editor/ToolBar/ToolBar.h"

#include "Engine/Scene/Object/GameObjectId.h"

#include "Engine/Application/ProjectSettings.h"

namespace Cake {

class Platform;
class Graphics;
class Asset;
class Renderer;

class SceneRenderer;
class SceneManager;

class Editor {
private:
	// 入力を通す対象。フォーカス中のウィンドウで決まる.
	enum class InputContext {
		None,
		Game,
		DebugCamera
	};

	/* Editor層が所有するもの */
	ImGuiManager imguiManager_;
	PlayModeController playMode_;

	MenuBar menuBar_;
	ToolBar toolBar_;
	SceneWindow sceneWindow_; // エディタカメラ視点。RT・カメラ・ギズモ設定を内包する.
	GameWindow gameWindow_;   // ゲームカメラ視点。RT を内包する.

	/* Editor層が依存する層 */
	ProjectSettings* settings_ = nullptr;
	EditorPreferences* preferences_ = nullptr; // 非所有。Application が持つ.
	Platform* platform_ = nullptr;
	Graphics* graphics_ = nullptr;
	Asset* asset_ = nullptr;
	Renderer* renderer_ = nullptr;

	SceneRenderer* sceneRenderer_ = nullptr;
	SceneManager* sceneManager_ = nullptr;

	EditorDrawContext ctx_;

	/* ウィンドウをまたいで共有する状態。個々のウィンドウには持たせない */
	GameObjectId selected_;
	InputContext inputContext_ = InputContext::None; // 前フレームのフォーカスで確定した入力先.

public:
	void Initialize(
		HWND hwnd, Platform& platform, Graphics& graphics, Asset& asset,
		Renderer& renderer, SceneRenderer& sceneRenderer, SceneManager& sceneManager,
		ProjectSettings& settings, EditorPreferences& preferences, const EditorCache& cache
	);

	// 毎フレームの更新.
	void Update(Time& time);

	// 毎フレームの描画.
	void Draw();

	void BeginFrame(Time& time);
	void EndFrame();

	// RT の作り直し（GPU待ちを含む）。コマンドを積み始める前に呼ぶこと.
	void PrepareRenderTargets();

	// 終了直前に呼ぶ。次回起動で復元したい状態を out へ書き出す.
	void CaptureCache(EditorCache& out) const;

private:
	void UpdateInputContext(); // 今フレームのフォーカスから次フレームの入力先を決める.
	void ApplyInputContext();  // inputContext_ を各カメラ／ゲームへ反映する.

	void DrawEditorUI();       // エディタUI一式の描画.
	void DrawEnvironmentTab();
};

} // namespace Cake

#endif // ENABLE_EDITOR
