#pragma once
/*====================================
 *
 * エディタカメラ視点の編集用ウィンドウ（ImGui 上の "Scene"）。
 *
 * 持つのは「自分の描画先」「自分の視点（EditorCamera）」「何を線で出すか（GizmoSettings）」
 * 「自分のフォーカス状態」まで。選択中オブジェクトと再生状態は Editor から引数で受け取る。
 *
 * 【エディタカメラの所有者】
 * このカメラは Scene ウィンドウの視点そのものなので、ここが持つ。
 * 次回起動での復元も、値の意味を知っているこのクラスが行う（Initialize / CaptureCache）。
 *
 * 【ギズモの重ね順】
 * ImGuizmo::SetDrawlist() を呼ばないと別ウィンドウの上に描かれる。
 * ビューポート画像の直後に、そのスクリーン矩形を渡して呼ぶこと。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include <cstdint>

#include "externals/imgui/imgui.h"
#include "externals/imgui/ImGuizmo.h"

#include "Engine/Graphics/RenderTarget/SceneRenderTarget.h"

#include "Engine/Editor/Camera/EditorCamera.h"
#include "Engine/Editor/PlayMode/PlayModeController.h"

#include "Engine/Scene/Object/GameObjectId.h"
#include "Engine/Scene/System/GizmoSystem.h"

namespace Cake {

class Platform;
class Graphics;
class Renderer;
class SceneRenderer;
class SceneManager;
struct ProjectSettings;
struct EditorCache;

class SceneWindow {
private:
	SceneRenderTarget rt_;
	EditorCamera camera_;
	GizmoSettings gizmoSettings_;
	bool focused_ = false;

	/* 非所有。寿命は Editor と同じ */
	Graphics* graphics_ = nullptr;
	Renderer* renderer_ = nullptr;
	SceneRenderer* sceneRenderer_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	ProjectSettings* settings_ = nullptr;

public:
	void Initialize(
		Platform& platform, Graphics& graphics, Renderer& renderer,
		SceneRenderer& sceneRenderer, SceneManager& sceneManager,
		ProjectSettings& settings, const EditorCache& cache
	);

	// エディタカメラの更新。タイムスケールの影響を受けない dt を渡すこと.
	void Update(float unscaledDeltaTime);

	// 入力を受け取るかどうか。フォーカスに応じて Editor が切り替える.
	void SetCameraActive(bool active) { camera_.SetActive(active); }

	// エディタカメラ視点をRTへ描く.
	void Render(GameObjectId selected);

	// ImGui ウィンドウを出す.
	void Draw(GameObjectId selected, PlayModeState playState);

	// 今フレームのフォーカス。Draw の後に読むこと.
	bool IsFocused() const { return focused_; }

	// 終了直前に呼ぶ。次回起動で復元したいカメラ状態を out へ書き出す.
	void CaptureCache(EditorCache& out) const;

	/* --- RT の遅延リサイズ。GPUアイドルの都合で Editor が順番を握る --- */
	bool ConsumePendingResize(uint32_t& width, uint32_t& height) {
		return rt_.ConsumePendingResize(width, height);
	}
	void ResizeRenderTarget(uint32_t width, uint32_t height) {
		rt_.Resize(width, height);
	}
	uint32_t GetRenderTargetWidth() const { return rt_.GetWidth(); }
	uint32_t GetRenderTargetHeight() const { return rt_.GetHeight(); }

private:
	void UpdateGizmoShortcuts();                                     // W / E / R で操作を切り替える.
	void DrawPlayModeBorder(PlayModeState playState);                // Play / Paused 中の枠.
	void DrawOperationLabel(const ImVec2& imageMin);                 // 左上の操作モード表示.
	void DrawGizmoOptions(const ImVec2& imageMin, const ImVec2& imageMax); // 左下のGizmosボタンとフィルタ.

	void DrawTransformGizmo(const ImVec2& imageMin, const ImVec2& imageMax, GameObjectId selected);
	void DrawViewGizmo(const ImVec2& imageMin, const ImVec2& imageMax);
};

} // namespace Cake

#endif // ENABLE_EDITOR
