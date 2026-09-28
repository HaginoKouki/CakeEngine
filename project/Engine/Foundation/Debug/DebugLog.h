#pragma once
/*====================================
 *
 * ログ出力とHRESULT（DirectX/COM関連のエラーコード）の解析・表示機能を提供するユーティリティ。
 * 日時別ディレクトリにログファイルを自動生成し、
 * エラーレベル（Info/Warn/Error）ごとに色分けして出力デバッグ情報を表示する。
 *
 * ====================================*/
#include <Windows.h>
#include <string>
#include <vector>
// ファイルに書いたり読んだりするためのライブラリ.
#include <fstream>

namespace Cake {

enum class LogLevel {
	Info,
	Warn,
	Error
};
struct LogEntry {
	LogLevel level;
	std::string timeString;
	std::string category;
	std::string message;
};

class DebugLog {
private:
	std::ofstream logStream_;
	std::vector<LogEntry> logEntries_;

private:
	DebugLog();
	~DebugLog();

	std::string GetCurrentTimeString() const;

public:
	DebugLog(const DebugLog&) = delete;
	DebugLog& operator=(const DebugLog&) = delete;

	static DebugLog& GetInstance() {
		static DebugLog instance;
		return instance;
	}

	void Log(LogLevel level, const std::string& category, const std::string& message);
	void LogHRESULT(HRESULT hr);
	// 初期化開始のログを流す.
	void LogInitStart(const std::string& category) { Log(LogLevel::Info, category, "Initialize Start"); }
	// 初期化終了のログを流す.
	void LogInitComplete(const std::string& category) { Log(LogLevel::Info, category, "Initialize Complete"); }

	const std::vector<LogEntry>& GetLogEntry() { return logEntries_; }
};

void AssertHRESULT(HRESULT hr, const std::string& message);
inline const char* LevelToString(LogLevel level) {
	switch (level) {
		case LogLevel::Info:
			return "INFO "; // 5文字で揃える.
		case LogLevel::Warn:
			return "WARN ";
		case LogLevel::Error:
			return "ERROR";
	}
	return "?????";
}

} // namespace Cake
