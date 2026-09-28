#include "SceneWindow.h"

#ifdef ENABLE_EDITOR

#include <algorithm>

#include "Engine/Application/ProjectSettings.h"
#include "Engine/Foundation/Math/Transform.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/Device/CommandManager.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Render/CameraView.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/SceneRenderer.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"

#include "Engine/Editor/Settings/EditorCache.h"

namespace Cake {

void SceneWindow::Initialize(
	Platform& platform, Graphics& graphics, Renderer& renderer,
	SceneRenderer& sceneRenderer, SceneManager& sceneManager,
	ProjectSettings& settings, const EditorCache& cache
) {
	graphics_ = &graphics;
	renderer_ = &renderer;
	sceneRenderer_ = &sceneRenderer;
	sceneManager_ = &sceneManager;
	settings_ = &settings;

	// DebugSceneRenderTarget（デバッグカメラ視点。RTV=3 / SRV=2）.
	rt_.Initialize(
		graphics_->GetDevice()->GetD3DDevice(),
		graphics_->GetRtvHeap(), graphics_->GetSrvHeap(),
		Reserved::kRtvDebugSceneRT, Reserved::kSrvDebugSceneRT,
		settings_->windowWidth, settings_->windowHeight
	);

	camera_.Initialize(&platform.GetInput());

	// カメラの復元は EditorCamera::Initialize（内部で Reset）より後に行うこと.
	if (cache.hasCamera) {
		camera_.SetTranslate(cache.cameraTranslate);
		camera_.SetRotation(cache.cameraRotation);
	}
}

void SceneWindow::Update(float unscaledDeltaTime) {
	camera_.Update(unscaledDeltaTime);
}

void SceneWindow::CaptureCache(EditorCache& out) const {
	out.hasCamera = true;
	out.cameraTranslate = camera_.GetTranslate();
	out.cameraRotation = camera_.GetRotation();
}

void SceneWindow::Render(GameObjectId selected) {
	CommandManager* cmd = graphics_->GetCommandManager();
	Scene& scene = sceneManager_->GetScene();

	// デバッグカメラ視点 → DebugSceneRT（同じシーンを別カメラで描く）.
	gizmoSettings_.selected = selected;
	gizmoSettings_.gameAspect = settings_->GetAspectRatio();
	RenderViewOptions debugOptions{};
	debugOptions.gizmos = &gizmoSettings_;

	renderer_->BeginScene(cmd, &rt_);
	sceneRenderer_->Render(
		scene,
		camera_.GetCameraView(rt_.GetWidth(), rt_.GetHeight()),
		debugOptions
	);
	renderer_->EndScene(&rt_);
}

void SceneWindow::Draw(GameObjectId selected, PlayModeState playState) {
	ImGui::Begin("Scene");
	focused_ = ImGui::IsWindowFocused();

	UpdateGizmoShortcuts();

	ImVec2 avail = ImGui::GetContentRegionAvail();

	int iw = (int)avail.x;
	int ih = (int)avail.y;
	if (iw > 0 && ih > 0) {
		rt_.RequestResize((uint32_t)iw, (uint32_t)ih);

		ImTextureID tex = (ImTextureID)rt_.GetSRVGpuHandle().ptr;
		ImGui::Image(tex, avail);
		const ImVec2 imageMin = ImGui::GetItemRectMin();
		const ImVec2 imageMax = ImGui::GetItemRectMax();

		// Play / Paused 中は枠を出す。「この編集は Stop で消える」ことを常に見せるため.
		DrawPlayModeBorder(playState);
		// 変形ギズモを出す.
		DrawTransformGizmo(imageMin, imageMax, selected);
		// 画像の左上にモードを重ねる.
		DrawOperationLabel(imageMin);
		// 画像の右上に軸ギズモを重ねる.
		DrawViewGizmo(imageMin, imageMax);
		// 画像の左下に Gizmos ボタンを重ねる.
		DrawGizmoOptions(imageMin, imageMax);
	}

	ImGui::End();
}

void SceneWindow::UpdateGizmoShortcuts() {
	if (!focused_ || ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGuizmo::IsUsing()) {
		return;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_W, false)) {
		gizmoSettings_.gizmoOperation = ImGuizmo::TRANSLATE;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_E, false)) {
		gizmoSettings_.gizmoOperation = ImGuizmo::ROTATE;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
		gizmoSettings_.gizmoOperation = ImGuizmo::SCALE;
	}
}

void SceneWindow::DrawPlayModeBorder(PlayModeState playState) {
	if (playState == PlayModeState::Edit) {
		return;
	}
	const ImU32 color = (playState == PlayModeState::Play)
	                        ? IM_COL32(70, 160, 255, 255)  // Play: 青.
	                        : IM_COL32(255, 190, 60, 255); // Paused: 橙.

	// GetItemRect* は直前の Image のスクリーン矩形.
	ImGui::GetWindowDrawList()->AddRect(
		ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
		color, 0.0f, 0, 5.0f
	);
}

void SceneWindow::DrawOperationLabel(const ImVec2& imageMin) {
	ImGui::GetWindowDrawList()->AddText(
		ImVec2(imageMin.x + 8.0f, imageMin.y + 8.0f),
		IM_COL32(255, 255, 255, 200),
		gizmoSettings_.gizmoOperation == ImGuizmo::TRANSLATE ? "Translate (W)"
		: gizmoSettings_.gizmoOperation == ImGuizmo::ROTATE  ? "Rotate (E)"
															 : "Scale (R)"
	);
}

void SceneWindow::DrawGizmoOptions(const ImVec2& imageMin, const ImVec2& imageMax) {
	ImGui::SetCursorScreenPos(ImVec2(imageMin.x + 8.0f, imageMax.y - 32.0f));
	if (ImGui::Button(gizmoSettings_.enabled ? "Gizmos: On" : "Gizmos: Off")) {
		gizmoSettings_.enabled = !gizmoSettings_.enabled;
	}
	if (ImGui::BeginPopupContextItem("GizmoFilter")) {
		ImGui::Checkbox("Grid", &gizmoSettings_.grid.enabled);
		ImGui::SliderFloat("Grid Opacity", &gizmoSettings_.grid.opacity, 0.0f, 1.0f, "%.2f");
		ImGui::DragFloat("Cell Size", &gizmoSettings_.grid.cellSize, 0.1f, 0.1f, 1000.0f);
		ImGui::DragInt("Half Count", &gizmoSettings_.grid.halfCount, 1, 1, 200);
		ImGui::Separator();
		ImGui::Checkbox("Colliders", &gizmoSettings_.showColliders);
		ImGui::Checkbox("Cameras", &gizmoSettings_.showCameras);
		ImGui::Checkbox("Lights", &gizmoSettings_.showLights);
		ImGui::EndPopup();
	}
}

void SceneWindow::DrawTransformGizmo(const ImVec2& imageMin, const ImVec2& imageMax, GameObjectId selected) {
	Scene& scene = sceneManager_->GetScene();
	GameObject* object = scene.Find(selected);
	if (object == nullptr) {
		return;
	}

	// この ImGui ウィンドウの描画リストへ重ねる。
	// これが無いと別ウィンドウの上に描かれたり、まったく出なかったりする.
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(imageMin.x, imageMin.y, imageMax.x - imageMin.x, imageMax.y - imageMin.y);
	ImGuizmo::SetOrthographic(false);

	const CameraView view = camera_.GetCameraView(rt_.GetWidth(), rt_.GetHeight());

	// ギズモはワールド行列を操作する.
	Matrix4x4 world = object->GetTransform().GetWorldMatrix();

	if (ImGuizmo::Manipulate(
			&view.viewMatrix.m[0][0], &view.projectionMatrix.m[0][0],
			gizmoSettings_.gizmoOperation, gizmoSettings_.gizmoMode,
			&world.m[0][0]
		)) {

		// world = local × parentWorld なので、local = world × inverse(parentWorld).
		Matrix4x4 parentWorld = Matrix4x4::Identity;
		const GameObjectId parent = object->GetTransform().GetParent();
		if (parent.IsValid()) {
			if (const GameObject* parentObject = scene.Find(parent)) {
				parentWorld = parentObject->GetTransform().GetWorldMatrix();
			}
		}
		object->GetTransform().GetLocalMutable() = Math::DecomposeAffine(world * Matrix4x4::Inverse(parentWorld));
	}
}

void SceneWindow::DrawViewGizmo(const ImVec2& imageMin, const ImVec2& imageMax) {
	ImDrawList* dl = ImGui::GetWindowDrawList();

	// 配置（画像の右上）とサイズ.
	const float kRadius = 34.0f;           // ギズモの外周半径.
	const float kMargin = 12.0f;           // 画像端からの余白.
	const float kAxisLen = kRadius - 8.0f; // 軸線の長さ.
	const float kTipRadius = 8.0f;         // 先端の丸の半径.
	const ImVec2 center(imageMax.x - kMargin - kRadius, imageMin.y + kMargin + kRadius);

	// カメラのビュー回転で世界軸を回す（TransformNormalは回転のみ・射影なし）.
	const Matrix4x4 view = Matrix4x4::MakeAffineMatrix(Vector3::One, camera_.GetRotation(), camera_.GetTranslate());

	struct Marker {
		ImVec2 pos{};       // スクリーン位置.
		float depth = 0.0f; // カメラ空間z（大きいほど奥）.
		ImU32 color{};
		char label = 0; // 正軸のみ。負軸は0.
		bool positive = false;
	};

	// 世界軸→カメラ空間→スクリーンへ落とす.
	auto makeAxis = [&](const Vector3& worldDir, ImU32 col, char label) {
		Vector3 d = Vector3::TransformNormal(worldDir, view);
		Marker m;
		m.pos = ImVec2(center.x + d.x * kAxisLen, center.y - d.y * kAxisLen); // スクリーンYは下向きなので反転.
		m.depth = d.z;
		m.color = col;
		m.label = label;
		m.positive = (label != 0);
		return m;
	};

	const ImU32 kColX = IM_COL32(232, 76, 61, 255);
	const ImU32 kColY = IM_COL32(140, 200, 60, 255);
	const ImU32 kColZ = IM_COL32(60, 130, 230, 255);
	const ImU32 kColXDim = IM_COL32(232, 76, 61, 110); // 負軸は暗く.
	const ImU32 kColYDim = IM_COL32(140, 200, 60, 110);
	const ImU32 kColZDim = IM_COL32(60, 130, 230, 110);

	Marker markers[6] = {
		makeAxis(Vector3::UnitX, kColX, 'X'),
		makeAxis(Vector3::UnitY, kColY, 'Y'),
		makeAxis(Vector3::UnitZ, kColZ, 'Z'),
		makeAxis(Vector3(-1.0f, 0.0f, 0.0f), kColXDim, 0),
		makeAxis(Vector3(0.0f, -1.0f, 0.0f), kColYDim, 0),
		makeAxis(Vector3(0.0f, 0.0f, -1.0f), kColZDim, 0),
	};

	// 奥のものから描く（手前が上に重なる）.
	int order[6] = {0, 1, 2, 3, 4, 5};
	std::sort(order, order + 6, [&](int a, int b) { return markers[a].depth > markers[b].depth; });

	for (int i = 0; i < 6; ++i) {
		const Marker& m = markers[order[i]];
		if (m.positive) {
			dl->AddLine(center, m.pos, m.color, 2.0f);
			dl->AddCircleFilled(m.pos, kTipRadius, m.color);
			// 先端に軸ラベル（中央寄せ）.
			char buf[2] = {m.label, 0};
			ImVec2 ts = ImGui::CalcTextSize(buf);
			dl->AddText(ImVec2(m.pos.x - ts.x * 0.5f, m.pos.y - ts.y * 0.5f), IM_COL32(20, 20, 20, 255), buf);
		} else {
			// 負軸は線なしの小さめドット.
			dl->AddCircleFilled(m.pos, kTipRadius - 2.0f, m.color);
		}
	}
}

} // namespace Cake

#endif // ENABLE_EDITOR
