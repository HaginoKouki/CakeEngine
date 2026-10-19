#pragma once
/*====================================
 *
 * Platform層の部品をまとめて所有し、生涯を管理するクラス。
 * Platform層そのものと言って良い.
 * 
 * 【依存の向き】
 * この層より上（Graphics / Asset / Render / Scene / Editor）を一切知らない。
 * 下は Foundation のみ。この規則が破れると層が意味を失う。
 *
 * ====================================*/

#include "Engine/Platform/Time/Time.h"
#include "Engine/Platform/Input/InputManager.h"
#include "Engine/Platform/Sound/SoundManager.h"

namespace Cake {

class Platform {
private:
	Time time_;
	InputManager input_;
	SoundManager sound_;

	bool timerPeriodRaised_ = false; // timeBeginPeriod が成功したか.

public:
	Platform() = default;
	~Platform();

	// デストラクタで timeEndPeriod を呼ぶため、コピーで二重に呼ばれないようにする.
	Platform(const Platform&) = delete;
	Platform& operator=(const Platform&) = delete;

	void Initialize(HINSTANCE hInstance, HWND hwnd, std::chrono::steady_clock::time_point initialTime);

	// フレーム先頭。時間を測り、入力を更新する.
	void BeginFrame();
	// フレーム終了処理。FPS固定用の待機時間など.
	void EndFrame();

	Time& GetTime() { return time_; }
	InputManager& GetInput() { return input_; }
	SoundManager& GetSound() { return sound_; }
};

} // namespace Cake
