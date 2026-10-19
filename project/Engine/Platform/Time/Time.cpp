#include "Time.h"

#include <algorithm>
#include <thread>

namespace Cake {

void Time::Initialize(std::chrono::steady_clock::time_point initialTime) {
	initialTime_ = initialTime;
	unscaledTotalTime_ = 0.0;
	scaledTotalTime_ = 0.0;
	Reset();
}

void Time::Reset() {
	prevTime_ = std::nullopt;
	unscaledDeltaTime_ = 0.0f;
	scaledDeltaTime_ = 0.0f;
}

void Time::Tick() {
	// フレームの開始時刻を取得.
	const auto now = std::chrono::steady_clock::now();

	++frameCount_;

	// 初回は基準が無い。ここで初期化にかかった時間を計上すると、
	// 1フレーム目だけいきなり大きく進んでしまう.
	if (!prevTime_.has_value()) {
		prevTime_ = now;
		unscaledDeltaTime_ = 0.0f;
		scaledDeltaTime_ = 0.0f;
		return;
	}

	const float elapsed = std::chrono::duration<float>(now - prevTime_.value()).count();

	// デバッガで止めた直後などの巨大な値を、ここで頭打ちにする.
	unscaledDeltaTime_ = std::min<float>(elapsed, kMaxDeltaTime);
	// 累計時間を更新.
	unscaledTotalTime_ += static_cast<double>(unscaledDeltaTime_);

	scaledDeltaTime_ = isPaused_ ? 0.0f : unscaledDeltaTime_ * timeScale_;
	scaledTotalTime_ += scaledDeltaTime_;

	// 次フレームの計算用に今フレーム開始時刻を保存.
	prevTime_ = now;
}

void Time::WaitForTargetFrame() {
	using namespace std::chrono;

	// 目標FPSが未設定、または0以下なら制限しない.
	if (!targetFrameRate_.has_value() || targetFrameRate_.value() <= 0.0f) {
		return;
	}
	// Reset 直後でフレーム開始時刻が無いときは待たない.
	if (!prevTime_.has_value()) {
		return;
	}

	const double fps = static_cast<double>(targetFrameRate_.value());
	// 目標時間（60FPS なら約 16.67ms）.
	const auto targetFrameTime = duration_cast<steady_clock::duration>(duration<double>(1.0 / fps));
	// 目標よりわずかに短い時間（60FPS なら 1/65 秒）.
	const auto minFrameTime = duration_cast<steady_clock::duration>(duration<double>(1.0 / (fps + 5.0)));

	const auto frameStart = prevTime_.value();
	// これより長くかかったフレームは VSync がすでに刻んでいるとみなし、重ねて待たない.
	if (steady_clock::now() - frameStart >= minFrameTime) {
		return;
	}

	const auto deadline = frameStart + targetFrameTime;
	// 目標の 2ms 前までは Sleep で CPU を休ませる.
	constexpr auto kSpinMargin = milliseconds(2);
	std::this_thread::sleep_until(deadline - kSpinMargin);
	// 残りは回して待ち、Sleep の寝過ごしを避ける.
	while (steady_clock::now() < deadline) {
		std::this_thread::yield();
	}
}

} // namespace Cake
