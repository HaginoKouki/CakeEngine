#pragma once
/*====================================
 *
 * Win32メッセージとRaw Inputによるマウス入力クラス。
 * ボタン・ホイール・カーソル位置はWndProcのメッセージから、
 * カメラ操作用の相対移動量はWM_INPUT（Raw Input）から取得する。
 * 相対移動量にポインタ加速や画面端のクランプがかからないのがRaw Inputを使う理由。
 *
 * ====================================*/
#include <Windows.h>

#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Platform/Input/KeyState.h"

namespace Cake {

// ボタン番号。値はWndProcで割り当てる順序と一致させている.
enum class MouseButton : uint8_t {
	Left = 0,
	Right = 1,
	Middle = 2,
	X1 = 3, // 主に「戻る」に割り当てられるサイドボタン.
	X2 = 4, // 主に「進む」.
};

class Mouse {
public:
	static constexpr int kButtonCount = 8;

private:
	HWND hwnd_ = nullptr;

	// WndProcから随時書き込まれる生の状態.
	BYTE liveButtons_[kButtonCount] = {};
	float liveWheel_ = 0.0f;            // フレーム間の累積notch.
	Vector2 liveDelta_ = Vector2::Zero; // フレーム間の累積移動量.
	int pressedCount_ = 0;              // 同時押し数。マウスキャプチャの管理用.

	// フレーム内で固定されるスナップショット.
	BYTE buttons_[kButtonCount] = {};
	BYTE preButtons_[kButtonCount] = {};
	float wheel_ = 0.0f;
	Vector2 delta_ = Vector2::Zero;
	POINT position_ = {};

public:
	Mouse() = default;
	~Mouse() = default;

	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;

	/// Raw Inputデバイスを登録する.
	void Initialize(HWND hwnd);

	/// WndProcから呼ぶ。ボタン・ホイール・Raw Inputの移動量を記録する.
	void HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	/// フォーカスを失ったときに呼ぶ。押しっぱなしの張り付きを防ぐ.
	void Clear();

	/// フレーム先頭で呼ぶ。生の状態をスナップショットへ取り込む.
	void Update();

	/// <returns>デスクトップ上のマウスの位置</returns>
	POINT GetMousePosition() const { return position_; }
	/// <returns>今フレームのマウスの移動量（ポインタ加速なしの生カウント）</returns>
	Vector2 GetMouseVelocity() const { return delta_; }
	/// <param name="button">0=左 1=右 2=中 3=X1 4=X2</param>
	KeyState GetMouseButtonState(MouseButton button) const;
	/// <returns>今フレームのマウスホイールのスクロール量(notch)</returns>
	float GetMouseWheel() const { return wheel_; }
};

} // namespace Cake
