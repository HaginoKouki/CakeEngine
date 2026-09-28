#include "Editor.h"

#ifdef ENABLE_EDITOR

#include <algorithm>

#include "Engine/Platform/Platform.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Graphics/Device/CommandManager.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/SceneRenderer.h"
#include "Engine/Scene/SceneManager.h"

#include "Engine/Editor/Inspector/MaterialEditor.h"
#include "Engine/Editor/Inspector/SceneInspector.h"
#include "Engine/Editor/Log/LogWindow.h"

namespace Cake {

void Editor::Initialize(
	HWND hwnd, Platform& platform, Graphics& graphics, Asset& asset,
	Renderer& renderer, SceneRenderer& sceneRenderer, SceneManager& sceneManager,
	ProjectSettings& settings, EditorPreferences& preferences, const EditorCache& cache
) {
	platform_ = &platform;
	graphics_ = &graphics;
	asset_ = &asset;
	renderer_ = &renderer;
	sceneRenderer_ = &sceneRenderer;
	sceneManager_ = &sceneManager;
	settings_ = &settings;
	preferences_ = &preferences;

	imguiManager_.Initialize(hwnd, graphics_->GetDevice()->GetD3DDevice(), graphics_->GetSwapChain(), graphics_->GetSrvHeap());

	// 保存されていた表示設定を適用する（ImGuiManager::Initialize は Dark・等倍で初期化済み）.
	imguiManager_.ApplyUIScaleFromPreferences(*preferences_);
	imguiManager_.ApplyTheme(preferences_->theme);

	// 各ウィンドウ。RT の生成とカメラの復元はそれぞれの中で完結する.
	gameWindow_.Initialize(graphics, renderer, sceneRenderer, sceneManager, settings);
	sceneWindow_.Initialize(platform, graphics, renderer, sceneRenderer, sceneManager, settings, cache);
	toolBar_.Initialize(playMode_, sceneManager, asset);
	menuBar_.Initialize(sceneManager, asset, settings, preferences, imguiManager_);

	// エディタUIへ配る参照束。以降シグネチャは (対象, ctx_) に統一する.
	ctx_.device = graphics_->GetDevice()->GetD3DDevice();
	ctx_.shaderLibrary = graphics_->GetShaderLibrary();
	ctx_.textureManager = asset_->GetTextureManager();
	ctx_.materialManager = asset_->GetMaterialManager();
	ctx_.modelManager = asset_->GetModelManager();
	ctx_.assetDatabase = asset_->GetAssetDatabase();
}

void Editor::Update(Time& time) {
	playMode_.Tick(time);

	// 前フレームのフォーカスで確定した入力先を反映してから更新する（1フレーム遅延）.
	ApplyInputContext();

	sceneManager_->Update(time.GetDeltaTime(), time.GetUnscaledDeltaTime());
	// デバッグカメラ更新.
	sceneWindow_.Update(time.GetUnscaledDeltaTime());

	// エディタUI構築。ここで今フレームのフォーカスを取得し、次フレーム用の inputContext_ を決める.
	DrawEditorUI();
}

void Editor::UpdateInputContext() {
	// ImGui は同時に1つしか focus しないので、優先順で見れば足りる.
	if (gameWindow_.IsFocused()) {
		inputContext_ = InputContext::Game;
	} else if (sceneWindow_.IsFocused()) {
		inputContext_ = InputContext::DebugCamera;
	} else {
		inputContext_ = InputContext::None;
	}
}

void Editor::ApplyInputContext() {
	// Game 選択中だけゲームへ入力を通し、DebugCamera 選択中だけデバッグカメラを動かす.
	platform_->GetInput().SetGameInputEnabled(inputContext_ == InputContext::Game);
	sceneWindow_.SetCameraActive(inputContext_ == InputContext::DebugCamera);
}

void Editor::Draw() {
	// ゲームカメラ視点 → SceneRT.
	gameWindow_.Render();
	// デバッグカメラ視点 → DebugSceneRT（同じシーンを別カメラで描く）.
	sceneWindow_.Render(selected_);
}

void Editor::BeginFrame(Time& time) {
	imguiManager_.BeginFrame();
	ImGuizmo::BeginFrame();
	imguiManager_.BeginDockSpace(); // ドッキングの土台を先に敷く.
	// 別DPIのモニタへ移動した場合などに追従する.
	imguiManager_.RefreshSystemDpi(*preferences_);

	graphics_->GetPerformanceProfiler()->BeginFrame(graphics_->GetCommandManager(), time);
}

void Editor::EndFrame() {
	renderer_->PrepareBackbufferToEditor();

	imguiManager_.EndFrame(graphics_->GetCommandManager());
	graphics_->GetPerformanceProfiler()->EndFrame(graphics_->GetCommandManager());
}

void Editor::PrepareRenderTargets() {
	// 2枚の RT の遅延リサイズをまとめて適用する。
	// GPUアイドルは重いので、両方のリサイズ要求を取り出してから1回だけ待つ.
	uint32_t gw = 0, gh = 0, sw = 0, sh = 0;
	const bool gameResized = gameWindow_.ConsumePendingResize(gw, gh);
	const bool sceneResized = sceneWindow_.ConsumePendingResize(sw, sh);
	if (!gameResized && !sceneResized) {
		return;
	}

	graphics_->GetCommandManager()->WaitForGPU();
	if (gameResized) {
		gameWindow_.ResizeRenderTarget(gw, gh);
	}
	if (sceneResized) {
		sceneWindow_.ResizeRenderTarget(sw, sh);
	}

	// 深度は1枚を両パスで共用するので、両ビューを覆う最大サイズに合わせる.
	const uint32_t depthW = (std::max)(gameWindow_.GetRenderTargetWidth(), sceneWindow_.GetRenderTargetWidth());
	const uint32_t depthH = (std::max)(gameWindow_.GetRenderTargetHeight(), sceneWindow_.GetRenderTargetHeight());
	graphics_->ResizeDepthBuffer(depthW, depthH);
}

void Editor::DrawEditorUI() {
	const PlayModeState playState = playMode_.GetState();

	// ビューポート2枚。ここで各ウィンドウが今フレームのフォーカスを拾う.
	gameWindow_.Draw();
	sceneWindow_.Draw(selected_, playState);

	// 拾ったフォーカスから次フレームの入力先を確定する.
	UpdateInputContext();

	toolBar_.Draw(selected_);
	menuBar_.Draw(playState, selected_);

	DrawHierarchyWindow(sceneManager_->GetScene(), selected_, *sceneManager_);
	DrawInspectorWindow(sceneManager_->GetScene(), selected_, ctx_);
	DrawEnvironmentTab();

	LogWindow::GetInstance().DrawLogWindow();

	DrawMaterialList(ctx_);

	graphics_->GetPerformanceProfiler()->DrawHud();

	ImGui::ShowDemoWindow();
}

void Editor::DrawEnvironmentTab() {
	ImGui::Begin("Environment");

	if (ImGui::CollapsingHeader("SkyBox", ImGuiTreeNodeFlags_DefaultOpen)) {
		DrawMaterialInspector(renderer_->GetSkyboxMaterial(), ctx_);
	}

	ImGui::End();
}

void Editor::CaptureCache(EditorCache& out) const {
	// 未保存の新規シーンは空文字。次回起動では ProjectSettings の起動シーンへ falls back する.
	out.openScenePath = sceneManager_->GetCurrentPath();

	// カメラの値の意味を知っているのは SceneWindow なので、そちらに書かせる.
	sceneWindow_.CaptureCache(out);
}

} // namespace Cake

#endif // ENABLE_EDITOR
