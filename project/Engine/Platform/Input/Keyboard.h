#pragma once
/*====================================
 *
 * Win32メッセージ（WM_KEYDOWN等）によるキーボード入力クラス。
 * WndProcから受け取った押下/解放をliveKey_へ記録し、
 * Update()でフレーム内不変のスナップショット(key_/preKey_)へ取り込む。
 * 配列の添字は仮想キーコードではなくスキャンコードで、DIK_定数と一致する。
 *
 * ====================================*/
#include <Windows.h>

#include "Engine/Platform/Input/KeyState.h"
#include "Engine/Platform/Input/KeyCode.h"

namespace Cake {

class KeyBoard {
private:
	// WndProcから随時書き込まれる生の状態。ゲームループとは非同期に更新される.
	BYTE liveKey_[256] = {};
	// フレーム内で固定されるスナップショット.
	BYTE key_[256] = {};
	BYTE preKey_[256] = {};

public:
	KeyBoard() = default;
	~KeyBoard() = default;

	KeyBoard(const KeyBoard&) = delete;
	KeyBoard& operator=(const KeyBoard&) = delete;

	/// WndProcから呼ぶ。キーの押下/解放をliveKey_へ記録する.
	void HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	/// フォーカスを失ったときに呼ぶ。押しっぱなしの張り付きを防ぐ.
	void Clear();

	/// フレーム先頭で呼ぶ。liveKey_をスナップショットへ取り込む.
	void Update();

	/// <param name="key">判定するキー</param>
	KeyState GetKeyState(KeyCode key) const;
};

} // namespace Cake
