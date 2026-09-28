#include "MenuBar.h"

#ifdef ENABLE_EDITOR

#include <string>

#include "externals/imgui/imgui.h"

#include "Engine/Application/ProjectSettings.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Platform/File/FileDialog.h"
#include "Engine/Scene/SceneManager.h"

#include "Engine/Editor/ImGui/ImGuiManager.h"
#include "Engine/Editor/Settings/EditorPreferences.h"
#include "Engine/Editor/Settings/SettingsWindow.h"

namespace Cake {

void MenuBar::Initialize(
	SceneManager& sceneManager, Asset& asset,
	ProjectSettings& settings, EditorPreferences& preferences, ImGuiManager& imguiManager
) {
	sceneManager_ = &sceneManager;
	asset_ = &asset;
	settings_ = &settings;
	preferences_ = &preferences;
	imguiManager_ = &imguiManager;
}

void MenuBar::Draw(PlayModeState playState, GameObjectId& selected) {
	// Play 中のファイル操作は禁止する。
	// Play 中に保存すると「実行後の状態」がファイルに残り、
	// Stop で巻き戻った内容とファイルの内容が食い違う.
	const bool isEditing = (playState == PlayModeState::Edit);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::BeginDisabled(!isEditing);

			if (ImGui::MenuItem("New Scene")) {
				pendingFileAction_ = PendingFileAction::New;
				confirmRequested_ = true;
			}
			if (ImGui::MenuItem("Open Scene...")) {
				pendingFileAction_ = PendingFileAction::Open;
				confirmRequested_ = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
				DoSave();
			}

			ImGui::EndDisabled();

			if (!isEditing) {
				ImGui::Separator();
				ImGui::TextDisabled("Stop play mode to edit files");
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Project Settings...")) {
				showProjectSettings_ = true;
			}
			if (ImGui::MenuItem("Preferences...")) {
				showPreferences_ = true;
			}
			ImGui::EndMenu();
		}

		// 現在の保存先と、未保存アセットの有無を表示する.
		ImGui::Separator();
		const bool dirtyAssets = asset_->GetMaterialManager()->HasDirtyMaterials();
		ImGui::TextDisabled(
			"%s%s",
			sceneManager_->HasPath() ? sceneManager_->GetCurrentPath().c_str() : "(new scene)",
			dirtyAssets ? " *" : ""
		);

		ImGui::EndMainMenuBar();
	}

	// --- ここから下はメニューバーの外 ---

	// Ctrl+S。どのウィンドウにフォーカスがあっても効かせる.
	if (isEditing && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
		DoSave();
	}

	// メニューを閉じた後に開く。
	// BeginMenu の中で OpenPopup を呼ぶと ID スタックの階層が合わず、モーダルが開かない.
	if (confirmRequested_) {
		confirmRequested_ = false;
		ImGui::OpenPopup("ConfirmDiscard");
	}

	DrawConfirmDiscardPopup(selected);
	DrawSettingsWindows();
}

void MenuBar::DrawConfirmDiscardPopup(GameObjectId& selected) {
	// OpenPopup と同じ ID スタックの階層で呼ぶこと.
	if (!ImGui::BeginPopupModal("ConfirmDiscard", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		return;
	}

	ImGui::TextUnformatted("Unsaved changes will be lost. Continue?");
	ImGui::Spacing();

	if (ImGui::Button("Continue", ImVec2(120.0f, 0.0f))) {
		switch (pendingFileAction_) {
			case PendingFileAction::New:
				DoNewScene(selected);
				break;
			case PendingFileAction::Open:
				DoOpenScene(selected);
				break;
			default:
				break;
		}
		pendingFileAction_ = PendingFileAction::None;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) {
		pendingFileAction_ = PendingFileAction::None;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void MenuBar::DrawSettingsWindows() {
	DrawProjectSettingsWindow(*settings_, &showProjectSettings_);
	DrawPreferencesWindow(*preferences_, *imguiManager_, &showPreferences_);
}

void MenuBar::DoNewScene(GameObjectId& selected) {
	sceneManager_->CreateEmptyScene();
	selected = GameObjectId{};
}

void MenuBar::DoOpenScene(GameObjectId& selected) {
	std::string path;
	if (!OpenFileDialog("Open Scene", "Scene Files", "*.scene", "Assets/_SaveFile/scene", path)) {
		return; // キャンセル.
	}
	if (!sceneManager_->LoadScene(path)) {
		return; // 失敗時は現在のシーンを維持する（SceneManager がログを出す）.
	}
	selected = GameObjectId{}; // 読み込みで既存のIDは全て無効になる.
}

void MenuBar::DoSave() {
	// Unity と同じく、Ctrl+S はシーンと未保存アセットをまとめて保存する.
	if (!sceneManager_->HasPath()) {
		DoSaveSceneAs(); // 保存先が未確定なら先に決めさせる.
	} else {
		sceneManager_->SaveScene();
	}
	asset_->GetMaterialManager()->SaveDirtyMaterials();
}

void MenuBar::DoSaveSceneAs() {
	std::string path;
	if (!SaveFileDialog("Save Scene As", "Scene Files", "*.scene", "Assets/_SaveFile/scene", "scene", path)) {
		return;
	}
	sceneManager_->SaveSceneAs(path);
}

} // namespace Cake

#endif // ENABLE_EDITOR
