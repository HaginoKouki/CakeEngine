#include "LogWindow.h"
#ifdef USE_IMGUI

#include <cassert>
#include <format>

#include "externals/imgui/imgui.h"

#include "Engine/Editor/imgui/Icons.h"

namespace Cake {
namespace {

ImVec4 LevelToColor(const LogLevel& level) {
	switch (level) {
		case LogLevel::Info:
			return ImGui::GetStyle().Colors[ImGuiCol_::ImGuiCol_Text];
			break;
		case LogLevel::Warn:
			return ImVec4{1.0f, 0.84f, 0.0f, 1.0f};
			break;
		case LogLevel::Error:
			return ImVec4{1.0f, 0.00f, 0.0f, 1.0f};
			break;
	}
	return ImVec4{1.0f, 0.0f, 1.0f, 1.0f};
}
} // namespace

LogWindow::LogWindow() {}
LogWindow::~LogWindow() {}
void LogWindow::DrawLogWindow(const char* title, bool* open) {
	auto& logEntries = DebugLog::GetInstance().GetLogEntry();

	// open が渡されていて閉じられているなら何もしない.
	if (open && !*open) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(720, 400), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(title, open)) {
		ImGui::End();
		return;
	}

	// --- フィルタUI ---
	// チェックボックスが変化したかを filterChanged にためる.
	bool filterChanged = false;
	filterChanged |= ImGui::Checkbox(std::format("{} Info", kInfoIcon).c_str(), &showInfo_);
	ImGui::SameLine();
	filterChanged |= ImGui::Checkbox(std::format("{} Worn", kWornIcon).c_str(), &showWarn_);
	ImGui::SameLine();
	filterChanged |= ImGui::Checkbox(std::format("{} Error", kErrorIcon).c_str(), &showError_);
	ImGui::SameLine();
	ImGui::Checkbox(ICON_FA_ANGLES_DOWN"Auto-scroll", &autoScroll_);
	ImGui::SameLine();
	// 件数表示（全体 / 表示中）.
	ImGui::TextDisabled("(%zu / %zu)", filteredIndices_.size(), logEntries.size());

	ImGui::Separator();

	// そのレベルを表示するか.
	auto passesFilter = [this](LogLevel lv) -> bool {
		switch (lv) {
			case LogLevel::Info:
				return showInfo_;
			case LogLevel::Warn:
				return showWarn_;
			case LogLevel::Error:
				return showError_;
		}
		return true;
	};

	// フィルタが変わったらインデックスキャッシュを作り直す.
	if (filterChanged) {
		filteredIndices_.clear();
		builtCount_ = 0;
	}
	// 新しく追加された分（と作り直し分）だけを走査してインデックスを追加.
	// 全ログを毎フレーム走査しないための最適化.
	for (size_t i = builtCount_; i < logEntries.size(); ++i) {
		if (passesFilter(logEntries[i].level)) {
			filteredIndices_.push_back(static_cast<int>(i));
		}
	}
	builtCount_ = logEntries.size();

	// --- ログ本体（スクロール領域）---
	ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	// 表示行だけを処理するクリッパ（全ログ保持でも軽い）.
	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(filteredIndices_.size()));
	while (clipper.Step()) {
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
			const LogEntry& e = logEntries[filteredIndices_[row]];

			// 表示時に1行へ整形（ファイル出力と同じ形式）.
			std::string levelIcon;
			switch (e.level) {
				case LogLevel::Info:
					levelIcon = kInfoIcon;
					break;
				case LogLevel::Warn:
					levelIcon = kWornIcon;
					break;
				case LogLevel::Error:
					levelIcon = kErrorIcon;
					break;
			}
			std::string line = std::format(
				"{} {} [{}] [{:<15}] {}",
				levelIcon,
				e.timeString,
				LevelToString(e.level),
				e.category,
				e.message
			);

			ImGui::PushStyleColor(ImGuiCol_Text, LevelToColor(e.level));
			ImGui::TextUnformatted(line.c_str());
			ImGui::PopStyleColor();
		}
	}

	// 一番下までスクロールしているときだけ追従させる.
	if (autoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();
	ImGui::End();
}

} // namespace Cake

#endif
