#pragma once
/*====================================
 *
 * キーボード、マウス、ゲームパッド等の入力を一元管理するマネージャ。
 * 毎フレーム入力状態を更新し、GameScene等から
 * 「キーが押されているか」「この1フレームで新たに押されたか」等の判定を提供する。
 *
 * ====================================*/
#include <Windows.h>
#include <array>
#include <memory>

#include "Engine/Platform/Time/Time.h"
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Platform/Input/KeyState.h"
#include "Engine/Platform/Input/KeyCode.h"
#include "Engine/Platform/Input/Keyboard.h"
#include "Engine/Platform/Input/Mouse.h"
#include "Engine/Platform/Input/GamePad.h"

namespace Cake {

class InputManager {
private:
	HWND hwnd_ = nullptr;

	// ゲームからの問い合わせに受け付けるか.
	bool gameInputEnabled_ = true;

	std::unique_ptr<KeyBoard> keyboard_;
	std::unique_ptr<Mouse> mouse_;
	std::array<GamePad, GamePad::kMaxCount> gamePads_;

public:
	InputManager() = default;
	~InputManager();

	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	void Initialize(HINSTANCE hInstance, HWND hwnd); // 旧コンストラクタの中身をここへ

	void Update(float unscaledDeltaTime);

	/// WndProcから呼ぶ。入力関連のメッセージを各デバイスへ振り分ける.
	void HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	/// <param name="keyCode">チェックするキーのコード</param>
	/// <returns>キーの状態</returns>
	KeyState GetGameKeyState(KeyCode key) const;
	KeyState GetRawKeyState(KeyCode key) const;

	/// <returns>デスクトップ上のマウスの位置</returns>
	Cake::Vector2 GetMousePositionDesktop() const;
	/// <returns>ウィンドウ上のマウスの位置（初期化に使用したウィンドウ）</returns>
	Cake::Vector2 GetMousePositionWindow() const;
	/// <param name="hwnd">対象のウィンドウ</param>
	/// <returns>ウィンドウ上のマウスの位置</returns>
	Cake::Vector2 GetMousePositionWindow(HWND hwnd) const;
	/// <returns>今フレームのマウスの移動量</returns>
	Cake::Vector2 GetMouseVelocity() const;
	/// <param name="button">チェックするマウスボタンのインデックス</param>
	/// <returns>マウスボタンの状態</returns>
	KeyState GetMouseButtonState(MouseButton button) const;
	/// <returns>今フレームのマウスホイールのスクロール量(notch)</returns>
	float GetMouseWheel() const;

	/// <param name="padIndex">パッド番号(0〜3)</param>
	bool IsPadConnected(int padIndex = 0) const;
	KeyState GetPadButtonState(PadButton button, int padIndex = 0) const;
	/// <returns>左スティックの傾き(長さ0〜1、上が+y)</returns>
	Vector2 GetPadLeftStick(int padIndex = 0) const;
	Vector2 GetPadRightStick(int padIndex = 0) const;
	/// <returns>トリガーの踏み込み量(0〜1)</returns>
	float GetPadLeftTrigger(int padIndex = 0) const;
	float GetPadRightTrigger(int padIndex = 0) const;
	/// <param name="duration">振動の継続秒数</param>
	void SetPadVibration(float left, float right, float duration = 0.2f, int padIndex = 0);

	/* ゲーム入力の制御 */
	void SetGameInputEnabled(bool enabled) { gameInputEnabled_ = enabled; }
	bool IsGameInputEnabled() const { return gameInputEnabled_; }

private:
	static bool IsValidPadIndex(int padIndex) {
		return padIndex >= 0 && padIndex < GamePad::kMaxCount;
	}
};

} // namespace Cake
