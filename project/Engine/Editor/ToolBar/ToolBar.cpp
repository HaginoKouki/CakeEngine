#include "ToolBar.h"

#ifdef ENABLE_EDITOR

#include "externals/imgui/imgui.h"

#include "Engine/Asset/Asset.h"
#include "Engine/Editor/PlayMode/PlayModeController.h"
#include "Engine/Scene/SceneManager.h"

namespace Cake {

void ToolBar::Initialize(PlayModeController& playMode, SceneManager& sceneManager, Asset& asset) {
	playMode_ = &playMode;
	sceneManager_ = &sceneManager;
	asset_ = &asset;
}

void ToolBar::Draw(GameObjectId& selected) {
	ImGui::Begin("Toolbar");

	if (playMode_->GetState() == PlayModeState::Edit) {
		if (ImGui::Button("Play")) {
			playMode_->Play(sceneManager_->GetScene());
		}
	} else {
		if (ImGui::Button("Stop")) {
			playMode_->Stop(sceneManager_->GetScene(), *asset_->GetAssetDatabase());
			selected = GameObjectId{}; // 復元でIDが全部無効になる.
		}
	}

	ImGui::SameLine();

	if (playMode_->GetState() == PlayModeState::Play) {
		if (ImGui::Button("Pause ")) {
			playMode_->Pause();
		}
	} else if (playMode_->GetState() == PlayModeState::Paused) {
		if (ImGui::Button("Resume")) {
			playMode_->Resume();
		}
	} else {
		ImGui::BeginDisabled(true);
		ImGui::Button("Pause ");
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	ImGui::BeginDisabled(playMode_->GetState() != PlayModeState::Paused);
	if (ImGui::Button("Step")) {
		playMode_->RequestStep();
	}
	ImGui::EndDisabled();

	ImGui::End();
}

} // namespace Cake

#endif // ENABLE_EDITOR
