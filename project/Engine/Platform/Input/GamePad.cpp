#include "GamePad.h"

#include <algorithm>
#include <format>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "GamePad";

constexpr float kThumbMax = 32767.0f;
constexpr float kTriggerMax = 255.0f;

// 円形デッドゾーン。閾値未満は0、外側を0〜1へ再マップする.
// 軸ごとに切り捨てるやり方と違い、斜め入力でも大きさが素直に伸びる.
Vector2 ApplyStickDeadZone(SHORT rawX, SHORT rawY, float deadZone) {
	Vector2 raw{static_cast<float>(rawX), static_cast<float>(rawY)};
	float length = Vector2::Length(raw);
	if (length <= deadZone) {
		return Vector2::Zero;
	}
	// 負方向は-32768まで来るので、1.0を超えないようにクランプ.
	length = std::min<float>(length, kThumbMax);
	float scale = (length - deadZone) / (kThumbMax - deadZone);
	return Vector2::Normalize(raw) * scale;
}

// トリガーも閾値未満を切って0〜1へ再マップする.
float ApplyTriggerDeadZone(BYTE raw) {
	constexpr float kThreshold = static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	float value = static_cast<float>(raw);
	if (value <= kThreshold) {
		return 0.0f;
	}
	return (value - kThreshold) / (kTriggerMax - kThreshold);
}

// 0〜1の強さをXInputのモーター値(0〜65535)へ変換.
WORD ToMotorSpeed(float strength) {
	return static_cast<WORD>(std::clamp(strength, 0.0f, 1.0f) * 65535.0f);
}
} // namespace

GamePad::~GamePad() {
	// 振動させたまま終了するとパッドが震え続けるので必ず止める.
	StopVibration();
}

void GamePad::Initialize(DWORD userIndex) {
	userIndex_ = userIndex;
}

void GamePad::Update(float unscaledDeltaTime) {
	preButtons_ = buttons_;

	// 未接続スロットへのXInputGetStateは重いので、毎フレームは叩かない.
	if (!connected_) {
		reconnectTimer_ -= unscaledDeltaTime;
		if (reconnectTimer_ > 0.0f) {
			ClearState();
			return;
		}
		reconnectTimer_ = kReconnectInterval;
	}

	XINPUT_STATE state{};
	if (XInputGetState(userIndex_, &state) != ERROR_SUCCESS) {
		// 接続中だったパッドが抜かれた場合だけログを出す.
		if (connected_) {
			DebugLog::GetInstance().Log(
				LogLevel::Info, kLogCategory,
				std::format("Pad {} disconnected", userIndex_)
			);
		}
		connected_ = false;
		ClearState();
		return;
	}

	if (!connected_) {
		connected_ = true;
		DebugLog::GetInstance().Log(
			LogLevel::Info, kLogCategory,
			std::format("Pad {} connected", userIndex_)
		);
	}

	const XINPUT_GAMEPAD& pad = state.Gamepad;

	leftStick_ = ApplyStickDeadZone(
		pad.sThumbLX, pad.sThumbLY,
		static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	);
	rightStick_ = ApplyStickDeadZone(
		pad.sThumbRX, pad.sThumbRY,
		static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	);
	leftTrigger_ = ApplyTriggerDeadZone(pad.bLeftTrigger);
	rightTrigger_ = ApplyTriggerDeadZone(pad.bRightTrigger);

	// トリガーを仮想ボタンとしてビットに合成する.
	buttons_ = pad.wButtons;
	if (leftTrigger_ > 0.0f) {
		buttons_ |= static_cast<uint16_t>(PadButton::LeftTrigger);
	}
	if (rightTrigger_ > 0.0f) {
		buttons_ |= static_cast<uint16_t>(PadButton::RightTrigger);
	}

	// 振動の残り時間を減らし、切れたら止める.
	if (vibrating_ && vibrationTimer_ > 0.0f) {
		vibrationTimer_ -= unscaledDeltaTime;
		if (vibrationTimer_ <= 0.0f) {
			StopVibration();
		}
	}
}

void GamePad::ClearState() {
	buttons_ = 0;
	leftStick_ = Vector2::Zero;
	rightStick_ = Vector2::Zero;
	leftTrigger_ = 0.0f;
	rightTrigger_ = 0.0f;
	vibrating_ = false;
	vibrationTimer_ = 0.0f;
}

KeyState GamePad::GetButtonState(PadButton button) const {
	const uint16_t mask = static_cast<uint16_t>(button);
	return MakeKeyState((buttons_ & mask) != 0, (preButtons_ & mask) != 0);
}

void GamePad::SetVibration(float left, float right, float duration) {
	if (!connected_) {
		return;
	}
	XINPUT_VIBRATION vibration{};
	vibration.wLeftMotorSpeed = ToMotorSpeed(left);   // 低周波（重い振動）.
	vibration.wRightMotorSpeed = ToMotorSpeed(right); // 高周波（細かい振動）.
	XInputSetState(userIndex_, &vibration);

	vibrating_ = true;
	vibrationTimer_ = duration;
}

void GamePad::StopVibration() {
	XINPUT_VIBRATION vibration{}; // 両モーター0で停止.
	XInputSetState(userIndex_, &vibration);
	vibrating_ = false;
	vibrationTimer_ = 0.0f;
}

} // namespace Cake
