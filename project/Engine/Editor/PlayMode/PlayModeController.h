#pragma once
/*====================================
 *
 * Play / Stop の状態と、シーンのスナップショットを管理する。
 *
 * Play を押した時点のシーンを JSON 文字列としてメモリに控え、Stop でそこへ戻す。
 * Play 中の変更（インスペクタでの編集も、ゲームロジックによる移動も）はすべて
 * Stop で巻き戻る。
 *
 * 【なぜ JSON 文字列か】
 * SceneSerializer をそのまま使えるため。バイナリの memcpy は速いが、
 * std::string を持つコンポーネントで浅いコピーになり破綻する。
 * オブジェクトが数千個規模になるまでは JSON で困らない。
 *
 * 【重要】リフレクションに載っていない状態は復元されない。
 * Play 中にだけ変わる値は、必ず CAKE_PROPERTY で登録されたメンバに置くこと。
 *
 * 【選択状態】
 * Stop するとシーンが作り直され、既存の GameObjectId はすべて無効になる。
 * エディタ側は Stop 後に選択を解除すること。
 *
 * ====================================*/
#include <string>

namespace Cake {

class Scene;
class AssetDatabase;
class Time;

enum class PlayModeState {
	Edit,   // 編集中。コンポーネントは動かず、ここでの変更のみが保存される.
	Play,   // 実行中。ゲームロジックが動き、ここでの変更は巻き戻される。.
	Paused, // 一時停止中。ゲームロジックは止まるが、ここでの変更は巻き戻される.
};

class PlayModeController {
private:
	PlayModeState state_ = PlayModeState::Edit;
	std::string snapshot_;
	bool stepRequested_ = false; // コマ送りの要求.

public:
	// Edit->Play.
	// 開始時にシーンのスナップショットを取る.
	bool Play(Scene& scene);
	// Play/Paused → Edit.
	// スナップショットを元にシーンを復元する.
	bool Stop(Scene& scene, AssetDatabase& database);
	// Play → Paused.
	void Pause();
	// Paused → Play.
	void Resume();
	// Paused のとき、次の Update で1フレームだけ進めるよう要求する.
	void RequestStep();

	// フレームの先頭で呼び、状態を Time へ反映する.
	void Tick(Time& time);

	PlayModeState GetState() const { return state_; }
};

} // namespace Cake
