#include "SettingsWindow.h"

#ifdef ENABLE_EDITOR

#include <algorithm>
#include <cstdio>

#include "externals/imgui/imgui.h"

#include "Engine/Application/ProjectSettings.h"
#include "Engine/Editor/ImGui/ImGuiManager.h"
#include "Engine/Editor/Settings/EditorPreferences.h"
#include "Engine/Foundation/Utility/Convert.h"
#include "Engine/Platform/File/FileDialog.h"

namespace Cake {
namespace {

constexpr int kTextBufferSize = 260;

// std::string を InputText で編集する。
// 毎フレーム buffer を作り直すが、編集された瞬間に書き戻すので状態は失われない.
bool InputString(const char* label, std::string& target, ImGuiInputTextFlags flags = 0) {
	char buffer[kTextBufferSize];
	std::snprintf(buffer, sizeof(buffer), "%s", target.c_str());
	if (ImGui::InputText(label, buffer, sizeof(buffer), flags)) {
		target = buffer;
		return true;
	}
	return false;
}

// uint32 のペアを InputInt2 で編集し、下限でクランプして書き戻す.
bool InputDimension2(const char* label, uint32_t& width, uint32_t& height) {
	int values[2] = {static_cast<int>(width), static_cast<int>(height)};
	if (!ImGui::InputInt2(label, values)) {
		return false;
	}
	width = static_cast<uint32_t>((std::max)(values[0], 16));
	height = static_cast<uint32_t>((std::max)(values[1], 16));
	return true;
}

} // namespace

void DrawProjectSettingsWindow(ProjectSettings& settings, bool* open) {
	if (open != nullptr && !*open) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(460.0f, 320.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Project Settings", open)) {
		ImGui::End();
		return;
	}

	ImGui::TextDisabled("%s", ProjectSettings::kDefaultPath);
	ImGui::Separator();
	ImGui::Spacing();

	// --- 画面 ---
	ImGui::SeparatorText("Display");
	InputDimension2("Reference Resolution", settings.referenceWidth, settings.referenceHeight);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("ゲーム画面の設計解像度。Game ビューのアスペクト比の基準になります");
	}
	ImGui::Text("アスペクト比: %.4f", settings.GetAspectRatio());

	ImGui::Spacing();

	// --- ウィンドウ ---
	ImGui::SeparatorText("Window");
	InputDimension2("Window Size", settings.windowWidth, settings.windowHeight);
	ImGui::Checkbox("Fullscreen", &settings.fullscreen);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("ボーダレス全画面で起動します。エディタビルドでは無視されます");
	}

	// 編集結果はエディタのタイトル組み立て（Editor::UpdateWindowTitle）が拾うので、ここでは SetWindowText を呼ばない.
	InputString("Window Title", settings.windowTitle);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("ゲーム実行時のウィンドウタイトルです");
	}

	ImGui::TextDisabled("※ サイズとフルスクリーンは次回起動時に適用されます");

	ImGui::Spacing();

	// --- シーン ---
	ImGui::SeparatorText("Scene");
	InputString("Startup Scene", settings.startupScene);
	ImGui::SameLine();
	if (ImGui::Button("...##startupScene")) {
		std::string path;
		if (OpenFileDialog("Select Startup Scene", "Scene Files", "*.scene", "Assets", path)) {
			settings.startupScene = path;
		}
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("ゲーム起動時に最初に読み込まれるシーンです");
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Save", ImVec2(120.0f, 0.0f))) {
		settings.Save();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload", ImVec2(120.0f, 0.0f))) {
		settings.Load();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Save を押すまでファイルへは書かれません");

	ImGui::End();
}

namespace {

// Preferences の左ペインに並べる分類。
// 項目を増やすときは enum・kPreferenceCategories・switch の3箇所を対応させる.
enum class PreferenceCategory : int {
	General = 0,
	Appearance,

	Count
};

struct PreferenceCategoryInfo {
	PreferenceCategory category;
	const char* label;
};

constexpr PreferenceCategoryInfo kPreferenceCategories[] = {
	{PreferenceCategory::General, "General"},
	{PreferenceCategory::Appearance, "Appearance"},
};
static_assert(
	IM_ARRAYSIZE(kPreferenceCategories) == static_cast<int>(PreferenceCategory::Count),
	"PreferenceCategory を増やしたら kPreferenceCategories にも足すこと"
);

// 戻り値は「ファイルへ書くべきか」。見た目への即時反映は各ページの中で済ませる.
bool DrawGeneralPage(EditorPreferences& preferences) {
	bool commit = false;

	ImGui::SeparatorText("Language");
	{
		const char* items[] = {
			ToDisplayName(EditorLanguage::Japanese),
		};
		static_assert(
			IM_ARRAYSIZE(items) == static_cast<int>(EditorLanguage::Count),
			"EditorLanguage を増やしたら items にも足すこと"
		);

		int index = static_cast<int>(preferences.language);
		if (ImGui::Combo("Language", &index, items, IM_ARRAYSIZE(items))) {
			preferences.language = static_cast<EditorLanguage>(index);
			commit = true;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("現在は値を保存するだけで、UI 文字列の切り替えは未実装です");
		}
	}

	return commit;
}

bool DrawAppearancePage(EditorPreferences& preferences, ImGuiManager& imguiManager) {
	bool commit = false;

	ImGui::SeparatorText("Theme");
	{
		const char* items[] = {
			ToDisplayName(EditorTheme::Dark),
			ToDisplayName(EditorTheme::Light),
		};
		static_assert(
			IM_ARRAYSIZE(items) == static_cast<int>(EditorTheme::Count),
			"EditorTheme を増やしたら items にも足すこと"
		);

		int index = static_cast<int>(preferences.theme);
		if (ImGui::Combo("Theme", &index, items, IM_ARRAYSIZE(items))) {
			preferences.theme = static_cast<EditorTheme>(index);
			imguiManager.ApplyTheme(preferences.theme); // 次フレームから新しい配色になる.
			commit = true;                              // コンボは1回で確定する操作なので即保存してよい.
		}
	}

	ImGui::Spacing();

	ImGui::SeparatorText("UI Scaling");
	{
		if (ImGui::Checkbox("Use System DPI", &preferences.followSystemDpi)) {
			imguiManager.ApplyUIScaleFromPreferences(preferences);
			commit = true;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Windows の「拡大／縮小」設定に合わせます。外すと下のスライダーの値を使います");
		}
		ImGui::TextDisabled(
			"システム検出値: %.0f%% (%.2f x)",
			imguiManager.GetSystemDpiScale() * 100.0f, imguiManager.GetSystemDpiScale()
		);

		ImGui::Spacing();

		// 追従中は手動値を使わないので、触れないことを見た目で示す.
		ImGui::BeginDisabled(preferences.followSystemDpi);
		{
			float scale = preferences.uiScale;
			if (ImGui::SliderFloat(
					"UI Scale", &scale,
					EditorPreferences::kMinUIScale, EditorPreferences::kMaxUIScale,
					"%.2f x", ImGuiSliderFlags_AlwaysClamp
				)) {
				preferences.uiScale = scale;
				imguiManager.ApplyUIScale(scale); // ドラッグ中もそのまま見た目へ出す.
			}
			// スライダーはドラッグ中ずっと true を返す。手を離して確定した瞬間だけ保存する.
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				commit = true;
			}

			if (ImGui::Button("Reset##uiScale")) {
				preferences.uiScale = EditorPreferences::kDefaultUIScale;
				imguiManager.ApplyUIScale(preferences.uiScale);
				commit = true;
			}
			ImGui::SameLine();
			ImGui::TextDisabled("既定値 %.2f x", EditorPreferences::kDefaultUIScale);
		}
		ImGui::EndDisabled();

		ImGui::Spacing();
		ImGui::Text("適用中: %.2f x", imguiManager.GetUIScale());
	}

	return commit;
}

} // namespace

void DrawPreferencesWindow(EditorPreferences& preferences, ImGuiManager& imguiManager, bool* open) {
	if (open != nullptr && !*open) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(640.0f, 400.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Preferences", open)) {
		ImGui::End();
		return;
	}

	// 選択中の分類。作業中だけ覚えていれば足りるので、ファイルへは保存しない.
	static PreferenceCategory selected = PreferenceCategory::General;

	// 幅をフォントサイズ基準にしておくと、UIスケールを変えても比率が崩れない.
	const float listWidth = ImGui::GetFontSize() * 9.0f;
	const float footerHeight = ImGui::GetTextLineHeightWithSpacing();

	// --- 左：分類リスト ---
	ImGui::BeginChild(
		"##preferenceCategories",
		ImVec2(listWidth, -footerHeight),
		ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX
	);
	for (const PreferenceCategoryInfo& info : kPreferenceCategories) {
		if (ImGui::Selectable(info.label, selected == info.category)) {
			selected = info.category;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// --- 右：選択中の分類の中身 ---
	bool commit = false;
	ImGui::BeginChild("##preferencePage", ImVec2(0.0f, -footerHeight));
	switch (selected) {
		case PreferenceCategory::General:
			commit = DrawGeneralPage(preferences);
			break;
		case PreferenceCategory::Appearance:
			commit = DrawAppearancePage(preferences, imguiManager);
			break;
		default:
			break;
	}
	ImGui::EndChild();

	// --- 下：保存先 ---
	ImGui::TextDisabled("%s", EditorPreferences::kDefaultPath);

	// 個人設定なので押し忘れで消えないよう、編集が確定した時点で書く.
	if (commit) {
		preferences.Save();
	}

	ImGui::End();
}

} // namespace Cake

#endif // ENABLE_EDITOR
