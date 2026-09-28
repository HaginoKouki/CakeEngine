#pragma once
#ifdef USE_IMGUI

#include <string>
#include <vector>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {

class LogWindow {
private:
	// --- ImGui表示用の状態 ---
	// レベルフィルタ（そのレベルを表示するか）.
	bool showInfo_ = true;
	bool showWarn_ = true;
	bool showError_ = true;
	// 一番下にいるとき自動でスクロールするか.
	bool autoScroll_ = true;
	// フィルタを通過した logEntries_ のインデックス一覧（描画高速化のためのキャッシュ）.
	std::vector<int> filteredIndices_;
	// filteredIndices_ を logEntries_ のどこまで構築済みか.
	size_t builtCount_ = 0;

	LogWindow();
	~LogWindow();

public:
	LogWindow(const LogWindow&) = delete;
	LogWindow& operator=(const LogWindow&) = delete;

	static LogWindow& GetInstance() {
		static LogWindow instance;
		return instance;
	}

	void DrawLogWindow(const char* title = "Debug Log", bool* open = nullptr);
};

} // namespace Cake

#endif
