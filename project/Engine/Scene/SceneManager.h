#pragma once
/*====================================
 *
 * 現在のシーンを1つ所有し、その生涯（生成・読み込み・保存・破棄）を管理するクラス。
 *
 * これまで Scene はゲーム側のクラスが直接持っていたが、それだとエディタが
 * ゲームの具体型を知らなければシーンへ辿り着けなかった。所有をここへ移すことで、
 * エディタもゲームも「SceneManager が持っている今のシーン」だけを見れば済む。
 *
 * 【モードを知らない】
 * 編集中か実行中かは Editor 側の概念であり、ここには届かない。
 * Update は渡された deltaTime だけを見て、0 ならコンポーネントを走らせない。
 * これによりリリースビルドでも同じコードがそのまま動く。
 *
 * 【将来】
 * シーン遷移（次のシーンを読み込んで差し替える）もここへ集約する。
 *
 * ====================================*/
#include <string>

#include "Engine/Scene/Scene.h"

namespace Cake {

class AssetDatabase;
class InputManager;
class Platform;

class SceneManager {
private:
	Scene scene_;
	AssetDatabase* assetDatabase_ = nullptr; // 非所有.
	Platform* platform_ = nullptr;
	InputManager* input_ = nullptr;          // 非所有。UpdateContext へ載せる.

	// 現在のシーンの保存先。未保存なら空.
	std::string currentPath_;

public:
	void Initialize(Platform* platform, AssetDatabase* assetDatabase);

	// deltaTime が 0 のフレームはコンポーネントを走らせない.
	void Update(float deltaTime, float unscaledDeltaTime);

	// 編集対象を、保存先の決まっていない空のシーンに差し替える.
	void CreateEmptyScene();

	// 現在の保存先へ上書き。未確定なら false.
	bool SaveScene();
	// パスを指定して保存し、以後の保存先にする.
	bool SaveSceneAs(const std::string& path);

	// ファイルから読み込んで差し替える。失敗時は現在のシーンを維持する.
	bool LoadScene(const std::string& path);

	Scene& GetScene() { return scene_; }
	const std::string& GetCurrentPath() const { return currentPath_; }
	bool HasPath() const { return !currentPath_.empty(); }
};

} // namespace Cake
