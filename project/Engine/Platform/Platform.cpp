#include "Platform.h"

#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "Platform";
// タイマーの分解能（ms）。timeBeginPeriod と timeEndPeriod には同じ値を渡す決まり.
constexpr UINT kTimerPeriodMs = 1;
} // namespace

Platform::~Platform() {
	if (timerPeriodRaised_) {
		timeEndPeriod(kTimerPeriodMs);
	}
}

void Platform::Initialize(HINSTANCE hInstance, HWND hwnd, std::chrono::steady_clock::time_point initialTime) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	// タイマーの分解能を 1ms に上げる。既定のままでは、フレーム待機の Sleep が目標を大きく過ぎてから起きる.
	timerPeriodRaised_ = (timeBeginPeriod(kTimerPeriodMs) == TIMERR_NOERROR);
	if (!timerPeriodRaised_) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "タイマーの分解能の設定に失敗しました。FPSが安定しない可能性があります。");
	}

	time_.Initialize(initialTime);
	input_.Initialize(hInstance, hwnd);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void Platform::BeginFrame() {
	time_.Tick();
	input_.Update(time_.GetUnscaledDeltaTime());
}
void Platform::EndFrame() {
	time_.WaitForTargetFrame();
}

} // namespace Cake
