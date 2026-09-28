#pragma once
/*====================================
 *
 * ゲーム1本ぶんの設定値。
 * 「このゲームはどう出力されるか」をここ1箇所に集める。
 *
 * 描画実装の都合（PSO、バッファサイズ等）はここに入れないこと。
 * ここに入るのは、ゲームを作る人が決める値だけ。
 *
 * 【ファイルの位置付け】
 * ProjectSettings/ProjectSettings.json へ保存する。チーム全員で共有する値なので
 * Git へコミットする前提。個人ごとに変わる値（テーマ・前回の状態）は
 * EditorPreferences / EditorCache 側であり、ここには絶対に混ぜない。
 * 混ぜるとコミットのたびに他人の環境設定を踏む。
 *
 * 【エディタからの編集】
 * SettingsWindow が直接メンバを書き換える。値が変わってもファイルへは書かれず、
 * Save ボタンを押したときだけ書き出す（共有データを自動保存しないため）。
 *
 * 【windowTitle が std::string な理由】
 * JSON と ImGui はどちらも UTF-8 の char 列を扱う。ウィンドウ生成時だけ
 * ConvertString でワイド文字へ直す。逆にすると保存も編集も両方で変換が要る。
 *
 * ====================================*/
#include <cstdint>
#include <string>

namespace Cake {

struct ProjectSettings {
	// ファイル形式の版数。項目を変えたらインクリメントする.
	static constexpr int kVersion = 1;
	static constexpr const char* kDefaultPath = "ProjectSettings/ProjectSettings.json";

	// ゲーム画面の設計解像度。アスペクト比の基準になる.
	uint32_t referenceWidth = 1920;
	uint32_t referenceHeight = 1080;

	// 起動時のウィンドウサイズ.
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;

	// ボーダレス全画面で起動するか。エディタビルドでは無視される.
	bool fullscreen = false;

	std::string windowTitle = "CakeEngine";
	std::string startupScene = "Assets/GameScene.scene";

	float GetAspectRatio() const {
		return (referenceHeight != 0)
		           ? static_cast<float>(referenceWidth) / static_cast<float>(referenceHeight)
		           : 1.0f;
	}

	const std::string& GetStartupScene() const { return startupScene; }

	// 失敗時は既定値のまま false を返す（呼び出し側は続行してよい）.
	bool Load(const std::string& path = kDefaultPath);
	bool Save(const std::string& path = kDefaultPath) const;
};

} // namespace Cake
