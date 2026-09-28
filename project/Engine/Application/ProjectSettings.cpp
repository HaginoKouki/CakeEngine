#include "ProjectSettings.h"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Serialize/JsonFile.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "ProjectSettings";

// 解像度として受け付ける範囲。0 や負値が入るとスワップチェーンの生成が落ちる.
constexpr uint32_t kMinDimension = 16;
constexpr uint32_t kMaxDimension = 16384;

uint32_t ClampDimension(int64_t value, uint32_t fallback) {
	if (value < static_cast<int64_t>(kMinDimension) || value > static_cast<int64_t>(kMaxDimension)) {
		return fallback;
	}
	return static_cast<uint32_t>(value);
}
} // namespace

bool ProjectSettings::Load(const std::string& path) {
	nlohmann::json root;
	if (!ReadJsonFile(path, root, kLogCategory)) {
		return false;
	}

	const ProjectSettings defaults{};

	// 一旦ローカルへ読み、全部読み終えてから自分へ反映する。
	// 途中で型エラーが出た場合に「半分だけ適用された設定」で動かないようにするため.
	ProjectSettings loaded;
	try {
		loaded.referenceWidth = ClampDimension(root.value("referenceWidth", static_cast<int64_t>(defaults.referenceWidth)), defaults.referenceWidth);
		loaded.referenceHeight = ClampDimension(root.value("referenceHeight", static_cast<int64_t>(defaults.referenceHeight)), defaults.referenceHeight);
		loaded.windowWidth = ClampDimension(root.value("windowWidth", static_cast<int64_t>(defaults.windowWidth)), defaults.windowWidth);
		loaded.windowHeight = ClampDimension(root.value("windowHeight", static_cast<int64_t>(defaults.windowHeight)), defaults.windowHeight);
		loaded.fullscreen = root.value("fullscreen", defaults.fullscreen);
		loaded.windowTitle = root.value("windowTitle", defaults.windowTitle);
		loaded.startupScene = root.value("startupScene", defaults.startupScene);
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			std::string("設定の読み取りに失敗したため既定値を使います: ") + e.what()
		);
		return false;
	}

	if (loaded.windowTitle.empty()) {
		loaded.windowTitle = defaults.windowTitle;
	}

	*this = std::move(loaded);
	return true;
}

bool ProjectSettings::Save(const std::string& path) const {
	nlohmann::json root;
	root["version"] = kVersion;
	root["referenceWidth"] = referenceWidth;
	root["referenceHeight"] = referenceHeight;
	root["windowWidth"] = windowWidth;
	root["windowHeight"] = windowHeight;
	root["fullscreen"] = fullscreen;
	root["windowTitle"] = windowTitle;
	root["startupScene"] = startupScene;

	return WriteJsonFile(path, root, kLogCategory);
}

} // namespace Cake
