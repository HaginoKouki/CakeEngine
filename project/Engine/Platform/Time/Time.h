#pragma once
/*====================================
 *
 * エンジン内で唯一の「時間の権威」。
 * 前フレームからの経過時間をここ1箇所で測り、deltaTime として全ての Update へ配る。
 *
 * 【ここ以外で経過時間を測らないこと】
 * 測る場所が増えると値が食い違い、「今どの時間で動いているのか」が追えなくなる。
 * InputManager も PerformanceProfiler も、自前で測らずここから受け取る。
 *
 * 【scaled と unscaled】
 * ゲームロジックは scaled（timeScale 適用済み）を使う。
 * エディタのカメラやUIのアニメーションは unscaled を使う。
 * Stop 中は timeScale が 0 になるが、エディタ側は動き続ける必要があるため。
 *
 * 【クランプ】
 * ブレークポイントで止めた直後などに巨大な deltaTime が流れると、
 * オブジェクトが吹き飛び、当たり判定もすり抜ける。
 * 実測値は kMaxDeltaTime で頭打ちにする。捨てた分は取り戻さない
 * （遅れを取り戻そうとすると、重い環境で加速して余計に破綻するため）。
 *
 * std::chrono しか使わない。この層で最も依存が軽いファイル。
 *
 * ====================================*/
#include <cstdint>
#include <optional>
#include <chrono>

namespace Cake {

class Time final {
private:
	// 1フレームとして認める経過時間の上限（秒）。超えた分は捨てる.
	static constexpr float kMaxDeltaTime = 0.1f;
	std::chrono::steady_clock::time_point initialTime_{};
	// 前回のフレーム開始時間.
	std::optional<std::chrono::steady_clock::time_point> prevTime_;

	// 実経過時間（timeScale適用前の値）.
	float unscaledDeltaTime_ = 0.0f;
	// ゲーム内経過時間（timeScale適用後の値）.
	float scaledDeltaTime_ = 0.0f;

	// 起動からの累計時間.
	double unscaledTotalTime_ = 0.0;
	double scaledTotalTime_ = 0.0;

	uint64_t frameCount_ = 0;

	bool isPaused_ = false;
	// 時間の流れ方。0で停止、1で等速。負の値は0に丸める.
	float timeScale_ = 1.0f;

	// 目標のフレームレート。0 以下なら待たない.
	std::optional<float> targetFrameRate_ = 60.0f;

public:
	void Initialize(std::chrono::steady_clock::time_point initialTime);
	// 計測を打ち切り、次の Tick を初回として扱う.
	// シーン読み込みなど、止まっていた時間を deltaTime に混ぜたくない場面で呼ぶ.
	void Reset();

	// フレームの先頭で1回だけ呼ぶ.
	// これ以降に走る処理は、そのフレーム中すべて同じ deltaTime を見る.
	void Tick();

	// フレームの末尾（Present の後）で1回だけ呼ぶ.
	// 前回からの経過が目標のフレーム時間より短ければ、その時間になるまで待つ.
	void WaitForTargetFrame();

	/* ===== ゲームロジック用（timeScale 適用済み）===== */

	float GetScaledDeltaTime() const { return scaledDeltaTime_; }
	double GetScaledTotalTime() const { return scaledTotalTime_; }

	/* ===== エディタ・UI・入力用（実時間）===== */
	/* Stop 中（timeScale = 0）でも進む. */

	float GetUnscaledDeltaTime() const { return unscaledDeltaTime_; }
	double GetUnscaledTotalTime() const { return unscaledTotalTime_; }

	// 起動からのフレーム数.
	uint64_t GetFrameCount() const { return frameCount_; }


	/* ===== 時間の流れ方 ===== */

	float GetTimeScale() const { return timeScale_; }
	// 次 Tick からの時間の流れを決める。0 で停止、1 で等速。負の値は 0 に丸める.
	void RequestSetTimeScale(float scale) { timeScale_ = (scale > 0.0f) ? scale : 0.0f; }

	bool IsPaused() const { return isPaused_; }
	// 次 Tick からの時間を停止するかを決める.
	void RequestSetPaused(bool paused) { isPaused_ = paused; }
};

} // namespace Cake
