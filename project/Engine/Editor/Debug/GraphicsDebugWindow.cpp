#include "GraphicsDebugWindow.h"

#ifdef ENABLE_EDITOR
#include <format>
#include <imgui.h>
#include "Engine/Platform/Job/JobSystem.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Upload/UploadBudget.h"

namespace Cake {

void GraphicsDebugWindow::Initialize(Graphics* graphics, JobSystem* jobSystem) {
	graphics_ = graphics;
	jobSystem_ = jobSystem;
}

void GraphicsDebugWindow::Draw(const char* title) {
	// たたまれているときも Begin と End は1回ずつ呼ぶ.
	if (!ImGui::Begin(title)) {
		ImGui::End();
		return;
	}

	// SRV の使用数.
	ImGui::SeparatorText("SRV");
	uint32_t usedCount = graphics_->GetSrvManager()->GetUsedCount();
	uint32_t capacity = graphics_->GetSrvManager()->GetCapacity();
	uint32_t peakUsedCount = graphics_->GetSrvManager()->GetPeakUsedCount();
	ImGui::Text("%s", std::format("使用 {} / {}", usedCount, capacity).c_str());
	ImGui::Text("%s", std::format("最大使用数 {}", peakUsedCount).c_str());
	ImGui::ProgressBar(static_cast<float>(usedCount) / static_cast<float>(capacity), ImVec2(-1.0f, 0.0f));

	// GPU への転送.
	ImGui::SeparatorText("Upload");
	ImGui::Text("%s", std::format("直前のフレームの転送量 {} MB", graphics_->GetUploadQueue()->GetBytesLastFrame() / (1024.0 * 1024.0)).c_str());
	ImGui::Text("%s", std::format("上限 {} MB", kUploadBytesPerFrame / (1024.0 * 1024.0)).c_str());
	ImGui::Text("%s", std::format("予約の数 {}", graphics_->GetUploadQueue()->GetPendingCount()).c_str());

	// 解放待ち.
	ImGui::SeparatorText("Deferred Release");
	ImGui::Text("%s", std::format("未解放予約の数 {}", graphics_->GetDeferredReleaseQueue()->GetPendingCount()).c_str());

	// ジョブ.
	ImGui::SeparatorText("Jobs");
	ImGui::Text("%s", std::format("未実行の処理 {}", jobSystem_->GetQueuedCount()).c_str());
	ImGui::Text("%s", std::format("完了処理待ち {}", jobSystem_->GetPendingCompletionCount()).c_str());

	ImGui::End();
}

} // namespace Cake
#endif
