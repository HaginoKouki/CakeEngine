#include "JsonFile.h"

#include <filesystem>
#include <fstream>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {

bool ReadJsonFile(const std::string& path, nlohmann::json& out, const char* logCategory) {
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		// 初回起動では必ずここを通る。エラーではないので Info.
		DebugLog::GetInstance().Log(LogLevel::Info, logCategory, "ファイルがありません: " + path);
		return false;
	}

	nlohmann::json parsed;
	try {
		ifs >> parsed;
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, logCategory,
			std::string("JSONの解析に失敗: ") + e.what()
		);
		return false;
	}

	// 解析に成功したときだけ差し替える（途中で失敗しても out は無傷）.
	out = std::move(parsed);
	return true;
}

bool WriteJsonFile(const std::string& path, const nlohmann::json& root, const char* logCategory) {
	std::error_code ec;
	const std::filesystem::path parent = std::filesystem::path(path).parent_path();
	if (!parent.empty()) {
		std::filesystem::create_directories(parent, ec);
		if (ec) {
			DebugLog::GetInstance().Log(
				LogLevel::Error, logCategory,
				"フォルダを作成できません: " + parent.generic_string()
			);
			return false;
		}
	}

	std::ofstream ofs(path, std::ios::trunc);
	if (!ofs.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Error, logCategory, "書き込み用に開けません: " + path);
		return false;
	}
	ofs << root.dump(2);

	DebugLog::GetInstance().Log(LogLevel::Info, logCategory, "保存: " + path);
	return true;
}

} // namespace Cake
