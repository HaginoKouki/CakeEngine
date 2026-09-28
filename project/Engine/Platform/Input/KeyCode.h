#pragma once
/*====================================
 *
 * キーボードのキーを識別する列挙型。
 * 値はPCのスキャンコード（Set 1）で、拡張キーは最上位ビットを立てた形。
 * 仮想キーコード(VK_)と違い物理的なキー位置を表すため、
 * JIS/US/AZERTY等の配列が変わってもWASDの位置関係が崩れない。
 *
 * ====================================*/
#include <cstdint>

namespace Cake {

enum class KeyCode : uint8_t {
	None = 0x00,

	// --- 英字 ---
	A = 0x1E, B = 0x30, C = 0x2E, D = 0x20, E = 0x12, F = 0x21, G = 0x22,
	H = 0x23, I = 0x17, J = 0x24, K = 0x25, L = 0x26, M = 0x32, N = 0x31,
	O = 0x18, P = 0x19, Q = 0x10, R = 0x13, S = 0x1F, T = 0x14, U = 0x16,
	V = 0x2F, W = 0x11, X = 0x2D, Y = 0x15, Z = 0x2C,

	// --- 数字（最上段。テンキーとは別物） ---
	Alpha1 = 0x02, Alpha2 = 0x03, Alpha3 = 0x04, Alpha4 = 0x05, Alpha5 = 0x06,
	Alpha6 = 0x07, Alpha7 = 0x08, Alpha8 = 0x09, Alpha9 = 0x0A, Alpha0 = 0x0B,

	// --- ファンクション ---
	F1 = 0x3B, F2 = 0x3C, F3 = 0x3D, F4 = 0x3E, F5 = 0x3F, F6 = 0x40,
	F7 = 0x41, F8 = 0x42, F9 = 0x43, F10 = 0x44, F11 = 0x57, F12 = 0x58,

	// --- 制御・編集 ---
	Escape = 0x01,
	Tab = 0x0F,
	Space = 0x39,
	Enter = 0x1C,
	BackSpace = 0x0E,
	CapsLock = 0x3A,

	// --- 修飾キー（左右を区別できる） ---
	LShift = 0x2A, RShift = 0x36,
	LControl = 0x1D, RControl = 0x9D,
	LAlt = 0x38, RAlt = 0xB8,
	LWin = 0xDB, RWin = 0xDC,
	Apps = 0xDD, // メニューキー.

	// --- 記号（US配列基準。JISでは刻印と一致しないものがある） ---
	Minus = 0x0C, Equals = 0x0D,
	LBracket = 0x1A, RBracket = 0x1B,
	Semicolon = 0x27, Apostrophe = 0x28,
	Grave = 0x29, // JISの半角/全角キーもここに来る.
	Backslash = 0x2B,
	Comma = 0x33, Period = 0x34, Slash = 0x35,

	// --- 矢印 ---
	Up = 0xC8, Down = 0xD0, Left = 0xCB, Right = 0xCD,

	// --- 編集ブロック ---
	Insert = 0xD2, Delete = 0xD3,
	Home = 0xC7, End = 0xCF,
	PageUp = 0xC9, PageDown = 0xD1,

	// --- テンキー ---
	Numpad0 = 0x52, Numpad1 = 0x4F, Numpad2 = 0x50, Numpad3 = 0x51, Numpad4 = 0x4B,
	Numpad5 = 0x4C, Numpad6 = 0x4D, Numpad7 = 0x47, Numpad8 = 0x48, Numpad9 = 0x49,
	NumpadPeriod = 0x53,
	NumpadPlus = 0x4E, NumpadMinus = 0x4A,
	NumpadMultiply = 0x37, NumpadDivide = 0xB5,
	NumpadEnter = 0x9C,

	// --- ロック・その他 ---
	NumLock = 0x45,
	ScrollLock = 0x46,
	Pause = 0xC5,
	PrintScreen = 0xB7, // WM_KEYUPしか来ないため押下判定は取れない.

	// --- 日本語配列固有 ---
	Kana = 0x70,
	Convert = 0x79,   // 変換.
	NoConvert = 0x7B, // 無変換.
	Yen = 0x7D,
};

} // namespace Cake
