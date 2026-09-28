#pragma once
/*====================================
 *
 * 再生操作のツールバー（ImGui 上の "Toolbar"）。Play / Pause / Resume / Step / Stop。
 *
 * 【状態を持たない】
 * 表示はすべて PlayModeController の状態から決まるため、このクラス自身は状態を持たない。
 * それでもクラスにしてあるのは、Play / Stop に必要な SceneManager と Asset を
 * メンバで抱えて、毎フレームの呼び出しを引数1つに収めるため。
 *
 * 【selected を参照で受ける理由】
 * Stop はシーンをスナップショットから復元するので、既存の GameObjectId が全て無効になる。
 * 選択を持っているのは Editor なので、ここでクリアできるよう参照で受け取る。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Asset;
class PlayModeController;
class SceneManager;

class ToolBar {
private:
	/* 非所有。寿命は Editor と同じ */
	PlayModeController* playMode_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	Asset* asset_ = nullptr;

public:
	void Initialize(PlayModeController& playMode, SceneManager& sceneManager, Asset& asset);

	// Stop したとき selected をクリアするため、参照で受け取る.
	void Draw(GameObjectId& selected);
};

} // namespace Cake

#endif // ENABLE_EDITOR
