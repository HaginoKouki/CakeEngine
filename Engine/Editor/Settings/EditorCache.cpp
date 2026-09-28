#include "EditorCache.h"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Serialize/JsonFile.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "EditorCache";

nlohmann::json WriteVector3(const Vector3& v) {
	return nlohmann::json::array({v.x, v.y, v.z});
}

// 要素数が足りない配列は既定値を保つ（壊れたファイルでカメラが原点へ飛ばない）.
void ReadVector3(const nlohmann::json& value, Vector3& out) {
	if (value.is_array() && value.size() >= 3) {
		out = {value[0].get<float>(), value[1].get<float>(), value[2].get<float>()};
	}
}
} // namespace

bool EditorCache::Load(const std::string& path) {
	nlohmann::json root;
	if (!ReadJsonFile(path, root, kLogCategory)) {
		return false;
	}

	const EditorCache defaults{};
	try {
		if (root.contains("window") && root["window"].is_object()) {
			const nlohmann::json& window = root["window"];
			windowX = window.value("x", defaults.windowX);
			windowY = window.value("y", defaults.windowY);
			windowWidth = window.value("width", defaults.windowWidth);
			windowHeight = window.value("height", defaults.windowHeight);
			maximized = window.value("maximized", defaults.maximized);
		}

		openScenePath = root.value("openScenePath", defaults.openScenePath);

		if (root.contains("camera") && root["camera"].is_object()) {
			const nlohmann::json& camera = root["camera"];
			if (camera.contains("translate")) {
				ReadVector3(camera["translate"], cameraTranslate);
			}
			if (camera.contains("rotation")) {
				ReadVector3(camera["rotation"], cameraRotation);
			}
			hasCamera = true;
		}
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			std::string("キャッシュの読み取りに失敗したため初期状態で開きます: ") + e.what()
		);
		*this = defaults;
		return false;
	}

	// 異常なサイズは未保存扱いにする（0除算やスワップチェーン生成の失敗を避ける）.
	if (windowWidth <= 0 || windowHeight <= 0) {
		windowWidth = 0;
		windowHeight = 0;
	}
	return true;
}

bool EditorCache::Save(const std::string& path) const {
	nlohmann::json window;
	window["x"] = windowX;
	window["y"] = windowY;
	window["width"] = windowWidth;
	window["height"] = windowHeight;
	window["maximized"] = maximized;

	nlohmann::json camera;
	camera["translate"] = WriteVector3(cameraTranslate);
	camera["rotation"] = WriteVector3(cameraRotation);

	nlohmann::json root;
	root["version"] = kVersion;
	root["window"] = std::move(window);
	root["openScenePath"] = openScenePath;
	root["camera"] = std::move(camera);

	return WriteJsonFile(path, root, kLogCategory);
}

} // namespace Cake
