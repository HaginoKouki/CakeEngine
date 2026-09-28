#pragma once
/*====================================
 *
 * 前回終了時のエディタの状態。次の起動で作業を続きから始めるためのキャッシュ。
 *
 * 【設定ではない】
 * ここにある値は「いつ消えても作業内容が失われない」ものだけに限る。
 * ファイルを消せばエディタが初期状態で開くだけ、という状態を保つこと。
 * 消えると困る値（＝成果物）は .scene か ProjectSettings 側の責務。
 *
 * 【ウィンドウ矩形はクライアント矩形ではない】
 * 復元時に CreateWindow へそのまま渡すため、枠を含むウィンドウ矩形で持つ。
 * AdjustWindowRect を二重に掛けると、起動のたびにウィンドウが太っていく。
 *
 * 【保存タイミング】
 * Application の終了時に1回だけ書く。毎フレーム書くと SSD への無駄な書き込みになる。
 * ウィンドウ矩形だけは WM_DESTROY で HWND が消える前に控える必要があるため、
 * WindowApp 側が控えたものを終了時に受け取る。
 *
 * ====================================*/
#include <numbers>
#include <string>

#include "Engine/Foundation/Math/Vector.h"

namespace Cake {

struct EditorCache {
	static constexpr int kVersion = 1;
	static constexpr const char* kDefaultPath = "UserSettings/EditorCache.json";

	// --- ウィンドウ（スクリーン座標のウィンドウ矩形）---
	int windowX = 0;
	int windowY = 0;
	int windowWidth = 0; // 0 は未保存（ProjectSettings の値を使う）.
	int windowHeight = 0;
	bool maximized = false;

	// --- 開いていたシーン。空なら ProjectSettings の起動シーンを使う ---
	std::string openScenePath;

	// --- エディタカメラ ---
	bool hasCamera = false;
	Vector3 cameraTranslate = {0.0f, 1.0f, 10.0f};
	Vector3 cameraRotation = {0.0f, std::numbers::pi_v<float>, 0.0f};

	bool HasWindowRect() const { return windowWidth > 0 && windowHeight > 0; }

	bool Load(const std::string& path = kDefaultPath);
	bool Save(const std::string& path = kDefaultPath) const;
};

} // namespace Cake
