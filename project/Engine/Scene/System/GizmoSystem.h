#pragma once
/*====================================
 *
 * シーンを走査して「絵に出ない情報」を線に起こす System。
 * コライダーの範囲、カメラの視錐台、ライトの向きなど、
 * 通常の描画では見えないものを GizmoDrawList へ積む。
 *
 * 描画そのものは行わない（積むだけ）。GPUへ出すのは GizmoPass の仕事。
 * 何を出すか・どれを強調するかは GizmoSettings で受け取るため、
 * エディタのチェックボックスとそのまま1対1で対応させられる。
 *
 * 実行前に Scene::UpdateTransforms() でワールド行列が確定していること。
 *
 * ====================================*/
#include "externals/imgui/imgui.h"
#include "externals/imgui/ImGuizmo.h"

#include "Engine/Render/Gizmo/GizmoDrawList.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Scene;
class GizmoDrawList;

// 何を出すかの設定。エディタが1つ持ち、UIと直結させる.
struct GizmoSettings {
	bool enabled = true;
	bool showColliders = true;
	bool showCameras = true;
	bool showLights = true;
	bool highlightSelected = true;

	ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;

	GizmoGrid grid;

	// 選択中のオブジェクト。無効なら強調しない.
	GameObjectId selected{};

	// 視錐台の形はアスペクト比だけで決まる。ゲームビューの比率を渡すこと.
	float gameAspect = 16.0f / 9.0f;
};

void CollectSceneGizmos(Scene& scene, const GizmoSettings& settings, GizmoDrawList& out);

} // namespace Cake
