#include "Mouse.h"

#include <cstring>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "Mouse";
} // namespace

void Mouse::Initialize(HWND hwnd) {
	hwnd_ = hwnd;

	// マウスの生の移動量を受け取る登録.
	// RIDEV_NOLEGACYを付けないので、WM_MOUSEMOVE等の通常メッセージも従来通り届く.
	// hwndTarget指定＋dwFlags=0なので、フォアグラウンド時のみWM_INPUTが来る.
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01; // Generic Desktop Controls.
	rid.usUsage = 0x02;     // Mouse.
	rid.dwFlags = 0;
	rid.hwndTarget = hwnd;

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "Raw Inputの登録に失敗。マウス移動量が取得できません。");
	}
}

void Mouse::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	// ボタン番号を決める。XBUTTONは1つのメッセージにX1/X2が相乗りしている.
	int button = -1;
	bool down = false;

	switch (msg) {
		// DBLCLKはウィンドウクラスにCS_DBLCLKSが無ければ来ないが、
		// 付けたときに押下を取りこぼさないよう拾っておく.
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			// 左ボタンが押された.
			button = 0;
			down = true;
			break;
		case WM_LBUTTONUP:
			// 左ボタンが離された.
			button = 0;
			down = false;
			break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			// 右ボタンが押された.
			button = 1;
			down = true;
			break;
		case WM_RBUTTONUP:
			// 右ボタンが離された.
			button = 1;
			down = false;
			break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			// 中ボタンが押された.
			button = 2;
			down = true;
			break;
		case WM_MBUTTONUP:
			// 中ボタンが離された.
			button = 2;
			down = false;
			break;
		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
			// Xボタンが押された.
			button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
			down = true;
			break;
		case WM_XBUTTONUP:
			// Xボタンが離された.
			button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
			down = false;
			break;

		case WM_MOUSEWHEEL:
			// ホイールが回転した.
			// WHEEL_DELTA(120)で割ってnotch単位にする.
			liveWheel_ += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
			return;

		case WM_INPUT: {
			// 生の入力が来た.
			// 必要なバッファサイズを問い合わせる.
			UINT size = 0;
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0) {
				return;
			}
			// マウスのみ登録しているのでRAWINPUT1個分で必ず足りる.
			if (size == 0 || size > sizeof(RAWINPUT)) {
				return;
			}
			BYTE buffer[sizeof(RAWINPUT)];
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size) {
				return;
			}

			auto* raw = reinterpret_cast<RAWINPUT*>(buffer);
			if (raw->header.dwType != RIM_TYPEMOUSE) {
				return;
			}
			// ペンタブやリモートデスクトップは絶対座標で届くので、相対移動のみ加算する.
			if ((raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0) {
				liveDelta_.x += static_cast<float>(raw->data.mouse.lLastX);
				liveDelta_.y += static_cast<float>(raw->data.mouse.lLastY);
			}
			return;
		}

		default:
			return;
	}

	
	if (button < 0 || button >= kButtonCount) {
		return;
	}

	if (down) {
		// 押している間はウィンドウ外へドラッグしても離した通知が届くようキャプチャする.
		// これが無いと、枠外でボタンを離したときに押しっぱなしで張り付く.
		if (pressedCount_ == 0 && GetCapture() == nullptr) {
			// キャプチャ開始.
			SetCapture(hwnd_);
		}
		++pressedCount_;
		liveButtons_[button] = 0x80;
	} else {
		// 離したときはキャプチャを解除する.
		if (pressedCount_ > 0) {
			--pressedCount_;
		}
		if (pressedCount_ == 0 && GetCapture() == hwnd_) {
			// キャプチャ終了.
			ReleaseCapture();
		}
		liveButtons_[button] = 0;
	}
}

void Mouse::Clear() {
	std::memset(liveButtons_, 0, sizeof(liveButtons_));
	liveWheel_ = 0.0f;
	liveDelta_ = Vector2::Zero;

	if (pressedCount_ > 0 && GetCapture() == hwnd_) {
		ReleaseCapture();
	}
	pressedCount_ = 0;
}

void Mouse::Update() {
	std::memcpy(preButtons_, buttons_, sizeof(buttons_));
	std::memcpy(buttons_, liveButtons_, sizeof(buttons_));

	// ホイールと移動量はフレーム間の累積値なので、取り込んだらリセットする.
	wheel_ = liveWheel_;
	liveWheel_ = 0.0f;
	delta_ = liveDelta_;
	liveDelta_ = Vector2::Zero;

	// カーソル位置はフレーム内で一貫させるため、ここで1回だけ確定する.
	GetCursorPos(&position_);
}

KeyState Mouse::GetMouseButtonState(MouseButton button) const {
	// キャストで作られた不正な値を弾くため、こちらは範囲チェックを残す.
	const size_t index = static_cast<size_t>(button);
	if (index >= kButtonCount) {
		return KeyState::None;
	}
	return MakeKeyState(buttons_[index] != 0, preButtons_[index] != 0);
}

} // namespace Cake
