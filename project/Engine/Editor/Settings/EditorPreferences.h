#pragma once
/*====================================
 *
 * エディタの個人設定。テーマや言語など「人によって変わるが、作るゲームには影響しない」値。
 *
 * ProjectSettings と分けてあるのは、こちらを Git へコミットしないため。
 * 同じファイルに同居させると、テーマを変えただけで差分が出てチームに波及する。
 *
 * 【ビルドガードを付けない理由】
 * ImGui に一切依存しない素のデータなので、USE_IMGUI / ENABLE_EDITOR を見ない。
 * 使う側（SettingsWindow、ImGuiManager）がガードを持てば足りる。
 *
 * 【language について】
 * 現状は値を保存するだけで、UI 文字列の差し替えは実装していない。
 * ローカライズ表を入れるときに、ここを起点に配る。
 *
 * 【uiScale について】
 * エディタUIだけの拡大率。ゲーム画面の解像度（ProjectSettings 側）とは無関係。
 * 範囲外の値を入れると ImGui のサイズ計算が破綻するため、Load で必ずクランプする。
 *
 * ====================================*/
#include <string>

namespace Cake {

enum class EditorTheme : int {
	Dark = 0,
	Light = 1,

	Count
};

enum class EditorLanguage : int {
	Japanese = 0,

	Count
};

// コンボボックスの表示名。enum を増やしたらここも足すこと.
const char* ToDisplayName(EditorTheme theme);
const char* ToDisplayName(EditorLanguage language);

struct EditorPreferences {
	// ファイル形式の版数。項目を変えたらインクリメントする.
	static constexpr int kVersion = 2;
	static constexpr const char* kDefaultPath = "UserSettings/EditorPreferences.json";

	// 手動指定の許容範囲.
	static constexpr float kMinUIScale = 0.5f;
	static constexpr float kMaxUIScale = 2.0f;
	static constexpr float kDefaultUIScale = 1.0f;

	// OS の拡大率を含めた「実際に適用する値」の上限。
	// 300% のモニタを手動上限の 2.0 で切り捨てないよう、こちらは広く取る.
	static constexpr float kMaxAppliedUIScale = 3.0f;

	EditorTheme theme = EditorTheme::Dark;
	EditorLanguage language = EditorLanguage::Japanese;

	// true なら Windows の拡大率に追従し、uiScale は使わない.
	bool followSystemDpi = true;
	float uiScale = kDefaultUIScale;

	bool Load(const std::string& path = kDefaultPath);
	bool Save(const std::string& path = kDefaultPath) const;
};

} // namespace Cake
