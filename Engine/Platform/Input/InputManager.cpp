#include "InputManager.h"

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "InputManager";
}

#pragma region InputManager

void InputManager::Initialize(HINSTANCE hInstance, HWND hwnd) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	hwnd_ = hwnd;

	keyboard_ = std::make_unique<KeyBoard>();
	mouse_ = std::make_unique<Mouse>();
	mouse_->Initialize(hwnd);

	for (int i = 0; i < GamePad::kMaxCount; ++i) {
		gamePads_[i].Initialize(static_cast<DWORD>(i));
	}

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}
InputManager::~InputManager() {
	if (hwnd_) {
		hwnd_ = nullptr;
	}
}

void InputManager::Update(float unscaledDeltaTime) {
	if (keyboard_)
		keyboard_->Update();
	if (mouse_)
		mouse_->Update();
	for (auto& pad : gamePads_) {
		pad.Update(unscaledDeltaTime);
	}
}
void InputManager::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (keyboard_) {
				keyboard_->HandleMessage(msg, wParam, lParam);
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_INPUT:
			if (mouse_) {
				mouse_->HandleMessage(msg, wParam, lParam);
			}
			break;

		// フォーカスを失うと離した通知が届かないので、両方の張り付きを防ぐ.
		case WM_KILLFOCUS:
			if (keyboard_) {
				keyboard_->Clear();
			}
			if (mouse_) {
				mouse_->Clear();
			}
			break;
	}
}

#pragma region キーボード
KeyState InputManager::GetGameKeyState(KeyCode key) const {
	if (gameInputEnabled_) {
		return keyboard_->GetKeyState(key);
	}
	return KeyState::None;
}
KeyState InputManager::GetRawKeyState(KeyCode key) const {
	return keyboard_->GetKeyState(key);
}
#pragma endregion

#pragma region マウス
Cake::Vector2 InputManager::GetMousePositionDesktop() const {
	POINT p = mouse_->GetMousePosition();
	return {static_cast<float>(p.x), static_cast<float>(p.y)};
}
Cake::Vector2 InputManager::GetMousePositionWindow() const {
	return GetMousePositionWindow(hwnd_);
}
Cake::Vector2 InputManager::GetMousePositionWindow(HWND hwnd) const {
	POINT p = mouse_->GetMousePosition();
	ScreenToClient(hwnd, &p);
	return {static_cast<float>(p.x), static_cast<float>(p.y)};
}
Cake::Vector2 InputManager::GetMouseVelocity() const {
	return mouse_->GetMouseVelocity();
}
KeyState InputManager::GetMouseButtonState(MouseButton button) const {
	return mouse_->GetMouseButtonState(button);
}
float InputManager::GetMouseWheel() const {
	return mouse_->GetMouseWheel();
}
#pragma endregion

#pragma region ゲームパッド
bool InputManager::IsPadConnected(int padIndex) const {
	return IsValidPadIndex(padIndex) && gamePads_[padIndex].IsConnected();
}
KeyState InputManager::GetPadButtonState(PadButton button, int padIndex) const {
	if (!IsValidPadIndex(padIndex))
		return KeyState::None;
	return gamePads_[padIndex].GetButtonState(button);
}
Vector2 InputManager::GetPadLeftStick(int padIndex) const {
	if (!IsValidPadIndex(padIndex))
		return Vector2::Zero;
	return gamePads_[padIndex].GetLeftStick();
}
Vector2 InputManager::GetPadRightStick(int padIndex) const {
	if (!IsValidPadIndex(padIndex))
		return Vector2::Zero;
	return gamePads_[padIndex].GetRightStick();
}
float InputManager::GetPadLeftTrigger(int padIndex) const {
	if (!IsValidPadIndex(padIndex))
		return 0.0f;
	return gamePads_[padIndex].GetLeftTrigger();
}
float InputManager::GetPadRightTrigger(int padIndex) const {
	if (!IsValidPadIndex(padIndex))
		return 0.0f;
	return gamePads_[padIndex].GetRightTrigger();
}
void InputManager::SetPadVibration(float left, float right, float duration, int padIndex) {
	if (!IsValidPadIndex(padIndex))
		return;
	gamePads_[padIndex].SetVibration(left, right, duration);
}
#pragma endregion

#pragma endregion

} // namespace Cake
