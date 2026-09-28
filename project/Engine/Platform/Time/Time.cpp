#include "Time.h"

#include <algorithm>

namespace Cake {

void Time::Tick() {
	// フレームの開始時刻を取得.
	const auto now = std::chrono::steady_clock::now();

	++frameCount_;

	// 初回は基準が無い。ここで初期化にかかった時間を計上すると、
	// 1フレーム目だけいきなり大きく進んでしまう.
	if (!hasPrevTime_) {
		prevTime_ = now;
		hasPrevTime_ = true;
		unscaledDeltaTime_ = 0.0f;
		deltaTime_ = 0.0f;
		return;
	}

	const float elapsed = std::chrono::duration<float>(now - prevTime_).count();
	prevTime_ = now;

	// デバッガで止めた直後などの巨大な値を、ここで頭打ちにする.
	unscaledDeltaTime_ = std::min<float>(elapsed, kMaxDeltaTime);
	// 累計時間を更新.
	unscaledTotalTime_ += static_cast<double>(unscaledDeltaTime_);

	deltaTime_ = 0.0f; // 今フレームの寄与を一旦ゼロにしてから加算し直す.
	ApplyScale();
}

void Time::ApplyScale() {
	const float newDelta = isPaused_ ? 0.0f : unscaledDeltaTime_ * timeScale_;
	totalTime_ += (newDelta - deltaTime_); // 今フレーム分を差し替える.
	deltaTime_ = newDelta;
}

void Time::Reset() {
	// prevTime_ は次の Tick で入れ直すので触らない.
	hasPrevTime_ = false;
	unscaledDeltaTime_ = 0.0f;
	deltaTime_ = 0.0f;
}

void Time::SetTimeScale(float scale) {
	timeScale_ = (scale > 0.0f) ? scale : 0.0f;
	ApplyScale();
}
void Time::SetPaused(bool paused) {
	isPaused_ = paused;
	ApplyScale();
}

} // namespace Cake
