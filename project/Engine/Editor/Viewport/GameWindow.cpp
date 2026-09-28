#include "GameWindow.h"

#ifdef ENABLE_EDITOR

#include "externals/imgui/imgui.h"

#include "Engine/Application/ProjectSettings.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/Device/CommandManager.h"
#include "Engine/Render/CameraView.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/SceneRenderer.h"
#include "Engine/Scene/SceneManager.h"

namespace Cake {

void GameWindow::Initialize(
	Graphics& graphics, Renderer& renderer, SceneRenderer& sceneRenderer,
	SceneManager& sceneManager, ProjectSettings& settings
) {
	graphics_ = &graphics;
	renderer_ = &renderer;
	sceneRenderer_ = &sceneRenderer;
	sceneManager_ = &sceneManager;
	settings_ = &settings;

	// SceneRenderTarget（ゲームカメラ視点）.
	rt_.Initialize(
		graphics_->GetDevice()->GetD3DDevice(),
		graphics_->GetRtvHeap(), graphics_->GetSrvHeap(),
		Reserved::kRtvSceneRT, Reserved::kSrvSceneRT,
		settings_->windowWidth, settings_->windowHeight
	);
}

void GameWindow::Render() {
	CommandManager* cmd = graphics_->GetCommandManager();
	Scene& scene = sceneManager_->GetScene();

	renderer_->BeginScene(cmd, &rt_);
	CameraView gameView{};
	if (sceneRenderer_->TryMakeMainCameraView(
			scene, rt_.GetWidth(), rt_.GetHeight(), gameView
		)) {
		sceneRenderer_->Render(scene, gameView);
	}
	renderer_->EndScene(&rt_);
}

void GameWindow::Draw() {
	ImGui::Begin("Game");
	focused_ = ImGui::IsWindowFocused();

	const float kTargetAspect = settings_->GetAspectRatio();

	// ゲームカメラのアスペクト比を基準にレターボックス.
	ImVec2 avail = ImGui::GetContentRegionAvail();

	// avail 内に収まる、targetAspect を保った最大の矩形を求める.
	float drawW = avail.x;
	float drawH = avail.x / kTargetAspect;
	if (drawH > avail.y) { // 縦がはみ出すなら縦基準に切り替え.
		drawH = avail.y;
		drawW = avail.y * kTargetAspect;
	}

	int iw = (int)drawW;
	int ih = (int)drawH;
	if (iw > 0 && ih > 0) {
		// RT をレターボックス矩形サイズに合わせる（次フレーム頭で適用される）.
		rt_.RequestResize((uint32_t)iw, (uint32_t)ih);

		// ウィンドウ内で中央寄せ（余白 = ImGuiウィンドウ背景 = 黒帯）.
		ImVec2 cursor = ImGui::GetCursorPos();
		cursor.x += (avail.x - drawW) * 0.5f;
		cursor.y += (avail.y - drawH) * 0.5f;
		ImGui::SetCursorPos(cursor);

		ImTextureID tex = (ImTextureID)rt_.GetSRVGpuHandle().ptr;
		ImGui::Image(tex, ImVec2(drawW, drawH));
	}

	ImGui::End();
}

} // namespace Cake

#endif // ENABLE_EDITOR
