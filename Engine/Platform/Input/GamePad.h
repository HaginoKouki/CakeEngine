#pragma once
/*====================================
 *
 * XInputによるゲームパッド入力クラス。
 * パッド1台分の接続状態・ボタン・スティック・トリガーを毎フレーム取得し、
 * KeyStateによる押下判定とデッドゾーン処理済みのスティック値を提供する。
 * 振動（バイブレーション）の時間管理もここで行う。
 * InputManagerが最大4台分を所有し、外部はInputManager経由でアクセスする。
 *
 * ====================================*/
#include <Windows.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib") // Windows8以降.

#include <cstdint>

#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Platform/Input/KeyState.h"

namespace Cake {

// ボタン定義。値はXInputのボタンビットそのもの.
enum class PadButton : uint16_t {
	Up = XINPUT_GAMEPAD_DPAD_UP,
	Down = XINPUT_GAMEPAD_DPAD_DOWN,
	Left = XINPUT_GAMEPAD_DPAD_LEFT,
	Right = XINPUT_GAMEPAD_DPAD_RIGHT,
	Start = XINPUT_GAMEPAD_START,
	Back = XINPUT_GAMEPAD_BACK,
	LeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,   //!< 左スティック押し込み.
	RightThumb = XINPUT_GAMEPAD_RIGHT_THUMB, //!< 右スティック押し込み.
	LeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
	RightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	A = XINPUT_GAMEPAD_A,
	B = XINPUT_GAMEPAD_B,
	X = XINPUT_GAMEPAD_X,
	Y = XINPUT_GAMEPAD_Y,

	// トリガーもボタンとして扱えるよう、XInputが使っていない空きビットを割り当てる.
	LeftTrigger = 0x0400,
	RightTrigger = 0x0800,
};

class GamePad {
public:
	static constexpr int kMaxCount = XUSER_MAX_COUNT; // XInputの接続上限は4台.

private:
	DWORD userIndex_ = 0; // XInputのスロット番号(0〜3).
	bool connected_ = false;

	uint16_t buttons_ = 0;    // 今フレームの押下ビット.
	uint16_t preButtons_ = 0; // 前フレームの押下ビット.

	Vector2 leftStick_ = Vector2::Zero; // デッドゾーン処理済み(長さ0〜1、上が+y).
	Vector2 rightStick_ = Vector2::Zero;
	float leftTrigger_ = 0.0f; // 0〜1.
	float rightTrigger_ = 0.0f;

	// 未接続スロットへのXInputGetStateはコストが高いので、間引いて再検索する.
	static constexpr float kReconnectInterval = 1.0f;
	float reconnectTimer_ = 0.0f;

	bool vibrating_ = false;
	float vibrationTimer_ = 0.0f; // 残り秒数.

public:
	GamePad() = default;
	~GamePad();

	GamePad(const GamePad&) = delete;
	GamePad& operator=(const GamePad&) = delete;

	void Initialize(DWORD userIndex);

	void Update(float unscaledDeltaTime);

	bool IsConnected() const { return connected_; }

	/// <returns>ボタンの状態</returns>
	KeyState GetButtonState(PadButton button) const;

	const Vector2& GetLeftStick() const { return leftStick_; }
	const Vector2& GetRightStick() const { return rightStick_; }
	float GetLeftTrigger() const { return leftTrigger_; }
	float GetRightTrigger() const { return rightTrigger_; }

	/// <param name="left">低周波モーターの強さ(0〜1)</param>
	/// <param name="right">高周波モーターの強さ(0〜1)</param>
	/// <param name="duration">継続秒数。0以下ならStopVibrationまで振動し続ける</param>
	void SetVibration(float left, float right, float duration = 0.2f);
	void StopVibration();

private:
	void ClearState(); // 未接続時に入力を全てゼロへ倒す.
};

} // namespace Cake
