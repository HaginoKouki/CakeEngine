#pragma once
/*====================================
 *
 * メインメニューバー（File / Edit）と、そこから開くものすべて。
 * 確認ダイアログ、Ctrl+S、Project Settings / Preferences ウィンドウを含む。
 *
 * 【ここに確認ダイアログまで入れている理由】
 * OpenPopup と BeginPopupModal は同じ ID スタックの階層で呼ぶ必要がある。
 * メニューを閉じた後（EndMainMenuBar の外）でしか開けないという順番の制約があるため、
 * 順番を守る責任ごと1クラスに閉じ込める。
 *
 * 【Play 中はファイル操作を止める】
 * Play 中に保存すると「実行後の状態」がファイルに残り、Stop で巻き戻った内容と食い違う。
 *
 * 【selected を参照で受ける理由】
 * New / Open はシーンを作り直すので、既存の GameObjectId が全て無効になる。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include "Engine/Editor/PlayMode/PlayModeController.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Asset;
class ImGuiManager;
class SceneManager;
struct ProjectSettings;
struct EditorPreferences;

class MenuBar {
private:
	// メニューから要求された操作。メニューを閉じた後に確認ダイアログを出すため保持する.
	enum class PendingFileAction {
		None,
		New,
		Open
	};
	PendingFileAction pendingFileAction_ = PendingFileAction::None;
	bool confirmRequested_ = false;

	// 設定ウィンドウの開閉状態.
	bool showProjectSettings_ = false;
	bool showPreferences_ = false;

	/* 非所有。寿命は Editor と同じ */
	SceneManager* sceneManager_ = nullptr;
	Asset* asset_ = nullptr;
	ProjectSettings* settings_ = nullptr;
	EditorPreferences* preferences_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;

public:
	void Initialize(
		SceneManager& sceneManager, Asset& asset,
		ProjectSettings& settings, EditorPreferences& preferences, ImGuiManager& imguiManager
	);

	// New / Open で selected をクリアするため、参照で受け取る.
	void Draw(PlayModeState playState, GameObjectId& selected);

private:
	void DrawConfirmDiscardPopup(GameObjectId& selected);
	void DrawSettingsWindows();

	void DoNewScene(GameObjectId& selected);
	void DoOpenScene(GameObjectId& selected);
	void DoSave();
	void DoSaveSceneAs();
};

} // namespace Cake

#endif // ENABLE_EDITOR
