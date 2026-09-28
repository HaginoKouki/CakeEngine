#include "Keyboard.h"

#include <cstdint>
#include <cstring>

namespace Cake {
namespace {

// lParamから物理キー位置（スキャンコード）を取り出す.
// DIK_定数の実体はスキャンコードなので、拡張ビットを0x80へ畳めばDIK_と一致する.
// 例: 拡張付き0x1D → 0x9D(DIK_RCONTROL)、拡張付き0x48 → 0xC8(DIK_UP).
uint8_t ToScanCode(WPARAM wParam, LPARAM lParam) {
	uint8_t scan = static_cast<uint8_t>((lParam >> 16) & 0xFF);

	// スクリーンキーボードやSendInput経由だとスキャンコードが0で届くことがある.
	// その場合は仮想キーコードから引き直す.
	if (scan == 0) {
		scan = static_cast<uint8_t>(MapVirtualKey(static_cast<UINT>(wParam), MAPVK_VK_TO_VSC));
	}

	const bool extended = ((lParam >> 24) & 0x1) != 0;
	return extended ? static_cast<uint8_t>(scan | 0x80) : scan;
}

} // namespace

void KeyBoard::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		// AltとF10はWM_SYSKEYDOWN/UPで来るので、両方拾う必要がある.
		// キーリピートで何度もWM_KEYDOWNが来るが、同じ値を書くだけなので無害.
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			liveKey_[ToScanCode(wParam, lParam)] = 0x80;
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			liveKey_[ToScanCode(wParam, lParam)] = 0;
			break;
	}
}

void KeyBoard::Clear() {
	// liveKey_のみ消す。key_/preKey_は次のUpdateで自然に追従する.
	std::memset(liveKey_, 0, sizeof(liveKey_));
}

void KeyBoard::Update() {
	// 前フレームの確定状態を退避してから、WndProcが書き込んだ最新状態を取り込む.
	// この2段構えがないとpreKey_==key_になり、Pressedを検出できなくなる.
	std::memcpy(preKey_, key_, sizeof(key_));
	std::memcpy(key_, liveKey_, sizeof(key_));
}

KeyState KeyBoard::GetKeyState(KeyCode key) const {
	// KeyCodeはuint8_t基底なので添字は必ず0〜255に収まる。範囲チェックは不要.
	const size_t index = static_cast<size_t>(key);
	return MakeKeyState(key_[index] != 0, preKey_[index] != 0);
}

} // namespace Cake
