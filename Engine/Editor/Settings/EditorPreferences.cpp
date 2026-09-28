#include "EditorPreferences.h"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Serialize/JsonFile.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "EditorPreferences";

// 範囲外の値（手書きで壊された・版数が違う）は既定値へ倒す.
template <class EnumT>
EnumT ToEnum(int value, EnumT fallback) {
	if (value < 0 || value >= static_cast<int>(EnumT::Count)) {
		return fallback;
	}
	return static_cast<EnumT>(value);
}
// 範囲外・NaN は既定値へ倒す。
// 「!(min <= v && v <= max)」の形にしてあるのは、NaN をここで一緒に弾くため.
float ClampUIScale(float value, float fallback) {
	if (!(value >= EditorPreferences::kMinUIScale && value <= EditorPreferences::kMaxUIScale)) {
		return fallback;
	}
	return value;
}
} // namespace

const char* ToDisplayName(EditorTheme theme) {
	switch (theme) {
		case EditorTheme::Dark:
			return "Dark";
		case EditorTheme::Light:
			return "Light";
		default:
			return "Unknown";
	}
}

const char* ToDisplayName(EditorLanguage language) {
	switch (language) {
		case EditorLanguage::Japanese:
			return "日本語";
		default:
			return "Unknown";
	}
}

bool EditorPreferences::Load(const std::string& path) {
	nlohmann::json root;
	if (!ReadJsonFile(path, root, kLogCategory)) {
		return false;
	}

	const EditorPreferences defaults{};
	try {
		theme = ToEnum(root.value("theme", static_cast<int>(defaults.theme)), defaults.theme);
		language = ToEnum(root.value("language", static_cast<int>(defaults.language)), defaults.language);
		// version 1 のファイルには存在しないので、既定値で埋まる.
		uiScale = ClampUIScale(root.value("uiScale", defaults.uiScale), defaults.uiScale);
		followSystemDpi = root.value("followSystemDpi", defaults.followSystemDpi);
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			std::string("設定の読み取りに失敗したため既定値を使います: ") + e.what()
		);
		*this = defaults;
		return false;
	}
	return true;
}

bool EditorPreferences::Save(const std::string& path) const {
	nlohmann::json root;
	root["version"] = kVersion;
	root["theme"] = static_cast<int>(theme);
	root["language"] = static_cast<int>(language);
	root["uiScale"] = uiScale;
	root["followSystemDpi"] = followSystemDpi;

	return WriteJsonFile(path, root, kLogCategory);
}

} // namespace Cake
