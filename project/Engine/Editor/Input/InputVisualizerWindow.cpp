#include "InputVisualizerWindow.h"

#ifdef USE_IMGUI

#include <algorithm>
#include <initializer_list>
#include <vector>

namespace Cake {
namespace {

// ===== 配置データ =====
// 座標・サイズの単位は「標準キー1個 = 1.0」。実ピクセルはkeyUnit_を掛けて出す.

constexpr float kKeyboardWidth = 22.5f; // テンキーまで含めた全体幅.
constexpr float kKeyboardHeight = 6.25f;

// パッドとマウスはピクセル基準（keyUnit_/30を掛けて拡縮する）.
constexpr float kGamePadWidth = 380.0f;
constexpr float kGamePadHeight = 250.0f;
constexpr float kMouseWidth = 110.0f;
constexpr float kMouseHeight = 190.0f;

struct KeyPlacement {
	KeyCode code;
	const char* label;
	float x, y;
	float w, h;
};

// 行を組み立てるときの1キー分の指定.
struct KeyDef {
	KeyCode code;
	const char* label;
	float width = 1.0f;
};

class LayoutBuilder {
public:
	// (x, y)から右へキーを並べる。codeがNoneのものは隙間として幅だけ送る.
	void Row(float x, float y, std::initializer_list<KeyDef> keys) {
		for (const KeyDef& key : keys) {
			if (key.code != KeyCode::None) {
				placements.push_back({key.code, key.label, x, y, key.width, 1.0f});
			}
			x += key.width;
		}
	}
	// 行に乗らない縦長キー（テンキーの+、JISのEnterなど）用.
	void Key(KeyCode code, const char* label, float x, float y, float w, float h) {
		placements.push_back({code, label, x, y, w, h});
	}

	std::vector<KeyPlacement> placements;
};

std::vector<KeyPlacement> BuildKeyboardLayout(KeyboardLayoutType type) {
	using K = KeyCode;
	const bool jis = (type == KeyboardLayoutType::JIS);
	LayoutBuilder b;

	// --- ファンクション行 ---
	b.Row(0.0f, 0.0f, {
		{K::Escape, "Esc"}, {K::None, nullptr, 1.0f},
		{K::F1, "F1"}, {K::F2, "F2"}, {K::F3, "F3"}, {K::F4, "F4"}, {K::None, nullptr, 0.5f},
		{K::F5, "F5"}, {K::F6, "F6"}, {K::F7, "F7"}, {K::F8, "F8"}, {K::None, nullptr, 0.5f},
		{K::F9, "F9"}, {K::F10, "F10"}, {K::F11, "F11"}, {K::F12, "F12"},
	});
	b.Row(15.25f, 0.0f, {{K::PrintScreen, "PrtSc"}, {K::ScrollLock, "ScrLk"}, {K::Pause, "Pause"}});

	// --- 数字行 ---
	if (jis) {
		// ¥キーがある分、BackSpaceが1uまで縮む.
		b.Row(0.0f, 1.25f, {
			{K::Grave, "Zen"}, {K::Alpha1, "1"}, {K::Alpha2, "2"}, {K::Alpha3, "3"}, {K::Alpha4, "4"},
			{K::Alpha5, "5"}, {K::Alpha6, "6"}, {K::Alpha7, "7"}, {K::Alpha8, "8"}, {K::Alpha9, "9"},
			{K::Alpha0, "0"}, {K::Minus, "-"}, {K::Equals, "^"}, {K::Yen, "Yen"}, {K::BackSpace, "BS"},
		});
	} else {
		b.Row(0.0f, 1.25f, {
			{K::Grave, "`"}, {K::Alpha1, "1"}, {K::Alpha2, "2"}, {K::Alpha3, "3"}, {K::Alpha4, "4"},
			{K::Alpha5, "5"}, {K::Alpha6, "6"}, {K::Alpha7, "7"}, {K::Alpha8, "8"}, {K::Alpha9, "9"},
			{K::Alpha0, "0"}, {K::Minus, "-"}, {K::Equals, "="}, {K::BackSpace, "BackSpace", 2.0f},
		});
	}

	// --- QWERTY行 ---
	// JISはEnterがL字（2段ぶち抜き）なので、この行の右端は空けて後からKeyで置く.
	b.Row(0.0f, 2.25f, {
		{K::Tab, "Tab", 1.5f},
		{K::Q, "Q"}, {K::W, "W"}, {K::E, "E"}, {K::R, "R"}, {K::T, "T"},
		{K::Y, "Y"}, {K::U, "U"}, {K::I, "I"}, {K::O, "O"}, {K::P, "P"},
		{K::LBracket, jis ? "@" : "["}, {K::RBracket, jis ? "[" : "]"},
		{jis ? K::None : K::Backslash, "\\", 1.5f},
	});

	// --- ホームポジション行 ---
	if (jis) {
		b.Row(0.0f, 3.25f, {
			{K::CapsLock, "Caps", 1.75f},
			{K::A, "A"}, {K::S, "S"}, {K::D, "D"}, {K::F, "F"}, {K::G, "G"},
			{K::H, "H"}, {K::J, "J"}, {K::K, "K"}, {K::L, "L"},
			{K::Semicolon, ";"}, {K::Apostrophe, ":"}, {K::Backslash, "]"},
		});
		// L字Enterの近似。2段ぶんの高さで右端に立てる.
		b.Key(K::Enter, "Enter", 13.75f, 2.25f, 1.25f, 2.0f);
	} else {
		b.Row(0.0f, 3.25f, {
			{K::CapsLock, "Caps", 1.75f},
			{K::A, "A"}, {K::S, "S"}, {K::D, "D"}, {K::F, "F"}, {K::G, "G"},
			{K::H, "H"}, {K::J, "J"}, {K::K, "K"}, {K::L, "L"},
			{K::Semicolon, ";"}, {K::Apostrophe, "'"}, {K::Enter, "Enter", 2.25f},
		});
	}

	// --- Shift行 ---
	b.Row(0.0f, 4.25f, {
		{K::LShift, "Shift", 2.25f},
		{K::Z, "Z"}, {K::X, "X"}, {K::C, "C"}, {K::V, "V"}, {K::B, "B"},
		{K::N, "N"}, {K::M, "M"}, {K::Comma, ","}, {K::Period, "."}, {K::Slash, "/"},
		{K::RShift, "Shift", 2.75f},
	});

	// --- 最下段 ---
	if (jis) {
		b.Row(0.0f, 5.25f, {
			{K::LControl, "Ctrl", 1.25f}, {K::LWin, "Win", 1.25f}, {K::LAlt, "Alt", 1.25f},
			{K::NoConvert, "Muhen", 1.25f}, {K::Space, "Space", 3.25f}, {K::Convert, "Henkan", 1.25f},
			{K::Kana, "Kana", 1.25f},
			{K::RAlt, "Alt", 1.0f}, {K::RWin, "Win", 1.0f}, {K::Apps, "Menu", 1.0f},
			{K::RControl, "Ctrl", 1.25f},
		});
	} else {
		b.Row(0.0f, 5.25f, {
			{K::LControl, "Ctrl", 1.25f}, {K::LWin, "Win", 1.25f}, {K::LAlt, "Alt", 1.25f},
			{K::Space, "Space", 6.25f},
			{K::RAlt, "Alt", 1.25f}, {K::RWin, "Win", 1.25f}, {K::Apps, "Menu", 1.25f},
			{K::RControl, "Ctrl", 1.25f},
		});
	}

	// --- 編集ブロック＋矢印 ---
	b.Row(15.25f, 1.25f, {{K::Insert, "Ins"}, {K::Home, "Home"}, {K::PageUp, "PgUp"}});
	b.Row(15.25f, 2.25f, {{K::Delete, "Del"}, {K::End, "End"}, {K::PageDown, "PgDn"}});
	b.Key(K::Up, "Up", 16.25f, 4.25f, 1.0f, 1.0f);
	b.Row(15.25f, 5.25f, {{K::Left, "Lt"}, {K::Down, "Dn"}, {K::Right, "Rt"}});

	// --- テンキー ---
	b.Row(18.5f, 1.25f, {{K::NumLock, "Num"}, {K::NumpadDivide, "/"}, {K::NumpadMultiply, "*"}, {K::NumpadMinus, "-"}});
	b.Row(18.5f, 2.25f, {{K::Numpad7, "7"}, {K::Numpad8, "8"}, {K::Numpad9, "9"}});
	b.Row(18.5f, 3.25f, {{K::Numpad4, "4"}, {K::Numpad5, "5"}, {K::Numpad6, "6"}});
	b.Row(18.5f, 4.25f, {{K::Numpad1, "1"}, {K::Numpad2, "2"}, {K::Numpad3, "3"}});
	b.Row(18.5f, 5.25f, {{K::Numpad0, "0", 2.0f}, {K::NumpadPeriod, "."}});
	b.Key(K::NumpadPlus, "+", 21.5f, 2.25f, 1.0f, 2.0f);
	b.Key(K::NumpadEnter, "Ent", 21.5f, 4.25f, 1.0f, 2.0f);

	return b.placements;
}

// 配列ごとに1度だけ組み立てて使い回す.
const std::vector<KeyPlacement>& GetKeyboardLayout(KeyboardLayoutType type) {
	static const std::vector<KeyPlacement> us = BuildKeyboardLayout(KeyboardLayoutType::US);
	static const std::vector<KeyPlacement> jis = BuildKeyboardLayout(KeyboardLayoutType::JIS);
	return (type == KeyboardLayoutType::JIS) ? jis : us;
}

// ===== 描画ヘルパ =====

ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t) {
	return ImVec4(
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t,
		a.w + (b.w - a.w) * t
	);
}

// PadButtonのビットマスクを0〜15の添字へ変換する.
int PadBitIndex(PadButton button) {
	uint16_t mask = static_cast<uint16_t>(button);
	int index = 0;
	while (mask > 1) {
		mask >>= 1;
		++index;
	}
	return index;
}

// 矩形の中央にラベルを描く。はみ出す場合はフォントを縮めて収める.
void DrawCenteredLabel(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, const char* label, ImU32 color) {
	if (label == nullptr || label[0] == '\0') {
		return;
	}
	const float maxWidth = std::max<float>((max.x - min.x) - 4.0f, 1.0f);
	ImVec2 textSize = ImGui::CalcTextSize(label);

	float scale = 1.0f;
	if (textSize.x > maxWidth) {
		scale = maxWidth / textSize.x;
		textSize.x *= scale;
		textSize.y *= scale;
	}
	const ImVec2 pos(
		(min.x + max.x - textSize.x) * 0.5f,
		(min.y + max.y - textSize.y) * 0.5f
	);
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, pos, color, label);
}

} // namespace

void InputVisualizerWindow::Initialize(InputManager* input) {
	input_ = input;
}

void InputVisualizerWindow::Draw(const char* title, bool* open) {
	if (input_ == nullptr) {
		return;
	}

	// ウィンドウを閉じている間も減衰は進めておく（開き直したときに焼き付かない）.
	UpdateGlow(ImGui::GetIO().DeltaTime);

	if (open != nullptr && !*open) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(760.0f, 620.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(title, open)) {
		ImGui::End();
		return;
	}

	// --- 設定 ---
	if (ImGui::CollapsingHeader("Settings")) {
		const char* layoutNames[] = {"US (ANSI)", "JIS"};
		int layoutIndex = static_cast<int>(layout_);
		if (ImGui::Combo("Layout", &layoutIndex, layoutNames, IM_ARRAYSIZE(layoutNames))) {
			layout_ = static_cast<KeyboardLayoutType>(layoutIndex);
		}
		ImGui::SliderFloat("Key Size", &keyUnit_, 14.0f, 48.0f, "%.0f px");
		ImGui::SliderFloat("Fade", &fadeTime_, 0.0f, 2.0f, "%.2f s");
		ImGui::ColorEdit3("Glow Color", &glowColor_.x);
		ImGui::Checkbox("Keyboard", &showKeyboard_);
		ImGui::SameLine();
		ImGui::Checkbox("GamePad", &showGamePad_);
		ImGui::SameLine();
		ImGui::Checkbox("Mouse", &showMouse_);
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const float scale = keyUnit_ / 30.0f; // パッド／マウスをキーの大きさに合わせる倍率.

	// --- キーボード ---
	if (showKeyboard_) {
		const ImVec2 origin = ImGui::GetCursorScreenPos();
		DrawKeyboard(drawList, origin);
		// ImDrawListは領域を占有しないので、Dummyで同じ大きさの場所取りをする.
		ImGui::Dummy(ImVec2(kKeyboardWidth * keyUnit_, kKeyboardHeight * keyUnit_));
		ImGui::Spacing();
	}

	// --- ゲームパッド ---
	if (showGamePad_) {
		ImGui::Separator();

		ImGui::SetNextItemWidth(100.0f);
		ImGui::Combo("##Pad", &padIndex_, "Pad 0\0Pad 1\0Pad 2\0Pad 3\0");
		ImGui::SameLine();
		const bool connected = input_->IsPadConnected(padIndex_);
		ImGui::TextColored(
			connected ? ImVec4(0.4f, 1.0f, 0.5f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
			connected ? "Connected" : "Disconnected"
		);

		const ImVec2 origin = ImGui::GetCursorScreenPos();
		DrawGamePad(drawList, origin);
		ImGui::Dummy(ImVec2(kGamePadWidth * scale, kGamePadHeight * scale));
	}

	// --- マウス ---
	if (showMouse_) {
		if (showGamePad_) {
			ImGui::SameLine();
		}
		const ImVec2 origin = ImGui::GetCursorScreenPos();
		DrawMouse(drawList, origin);
		ImGui::Dummy(ImVec2(kMouseWidth * scale, kMouseHeight * scale));
	}

	// --- 数値の読み取り（絵では分からないアナログ値）---
	if (showGamePad_ || showMouse_) {
		ImGui::Separator();
	}
	if (showGamePad_) {
		const Vector2 leftStick = input_->GetPadLeftStick(padIndex_);
		const Vector2 rightStick = input_->GetPadRightStick(padIndex_);
		ImGui::Text(
			"L(%+.2f, %+.2f)  R(%+.2f, %+.2f)  LT %.2f  RT %.2f",
			leftStick.x, leftStick.y, rightStick.x, rightStick.y,
			input_->GetPadLeftTrigger(padIndex_), input_->GetPadRightTrigger(padIndex_)
		);
	}
	if (showMouse_) {
		const Vector2 position = input_->GetMousePositionWindow();
		const Vector2 velocity = input_->GetMouseVelocity();
		ImGui::Text(
			"Mouse pos(%.0f, %.0f)  delta(%+.0f, %+.0f)  wheel %+.1f",
			position.x, position.y, velocity.x, velocity.y, input_->GetMouseWheel()
		);
	}

	ImGui::End();
}

void InputVisualizerWindow::UpdateGlow(float deltaTime) {
	// fadeTime_が0なら即消し。それ以外は「1.0から0になるまでfadeTime_秒」の割合で減らす.
	const float decay = (fadeTime_ > 0.0f) ? (deltaTime / fadeTime_) : 1.0f;

	// 先に全部を減衰させてから、押されているものだけ1.0へ戻す.
	// こうすると「押しっぱなし＝1.0固定」「離した瞬間から減衰開始」が自然に決まる.
	for (float& glow : keyGlow_) {
		glow = std::max<float>(glow - decay, 0.0f);
	}
	for (float& glow : padGlow_) {
		glow = std::max<float>(glow - decay, 0.0f);
	}
	for (float& glow : mouseGlow_) {
		glow = std::max<float>(glow - decay, 0.0f);
	}
	wheelGlow_ = std::max<float>(wheelGlow_ - decay, 0.0f);

	// キーボード。スキャンコード0(None)は存在しないキーなので飛ばす.
	for (int scanCode = 1; scanCode < 256; ++scanCode) {
		if (IsPressed(input_->GetRawKeyState(static_cast<KeyCode>(scanCode)))) {
			keyGlow_[scanCode] = 1.0f;
		}
	}

	// ゲームパッド。トリガーも仮想ボタンとしてビットが立つ.
	constexpr PadButton kPadButtons[] = {
		PadButton::Up, PadButton::Down, PadButton::Left, PadButton::Right,
		PadButton::Start, PadButton::Back,
		PadButton::LeftThumb, PadButton::RightThumb,
		PadButton::LeftShoulder, PadButton::RightShoulder,
		PadButton::LeftTrigger, PadButton::RightTrigger,
		PadButton::A, PadButton::B, PadButton::X, PadButton::Y,
	};
	for (PadButton button : kPadButtons) {
		if (IsPressed(input_->GetPadButtonState(button, padIndex_))) {
			padGlow_[PadBitIndex(button)] = 1.0f;
		}
	}

	// マウス.
	constexpr MouseButton kMouseButtons[] = {
		MouseButton::Left, MouseButton::Right, MouseButton::Middle,
		MouseButton::X1, MouseButton::X2,
	};
	for (MouseButton button : kMouseButtons) {
		if (IsPressed(input_->GetMouseButtonState(button))) {
			mouseGlow_[static_cast<size_t>(button)] = 1.0f;
		}
	}

	// ホイールは押下ではなく1フレームだけ値が来るので、回った瞬間に光らせる.
	const float wheel = input_->GetMouseWheel();
	if (wheel != 0.0f) {
		wheelGlow_ = 1.0f;
		wheelDirection_ = (wheel > 0.0f) ? 1.0f : -1.0f;
	}
}

void InputVisualizerWindow::DrawKeyboard(ImDrawList* drawList, const ImVec2& origin) {
	const std::vector<KeyPlacement>& layout = GetKeyboardLayout(layout_);
	const float unit = keyUnit_;
	const float gap = std::max<float>(unit * 0.06f, 1.5f); // キー同士の隙間.
	const float rounding = unit * 0.16f;

	for (const KeyPlacement& key : layout) {
		const ImVec2 mini(origin.x + key.x * unit + gap, origin.y + key.y * unit + gap);
		const ImVec2 maxi(origin.x + (key.x + key.w) * unit - gap, origin.y + (key.y + key.h) * unit - gap);
		const float glow = keyGlow_[static_cast<size_t>(key.code)];

		drawList->AddRectFilled(mini, maxi, GlowFill(glow), rounding);
		drawList->AddRect(mini, maxi, GlowBorder(glow), rounding, 0, 1.0f);
		DrawHalo(drawList, mini, maxi, rounding, glow);
		DrawCenteredLabel(drawList, mini, maxi, key.label, LabelColor(glow));
	}
}

void InputVisualizerWindow::DrawGamePad(ImDrawList* drawList, const ImVec2& origin) {
	const float scale = keyUnit_ / 30.0f;

	// パッド内のローカル座標（ピクセル基準）をスクリーン座標へ.
	auto ToScreen = [&](float x, float y) {
		return ImVec2(origin.x + x * scale, origin.y + y * scale);
	};
	auto Glow = [&](PadButton button) {
		return padGlow_[PadBitIndex(button)];
	};

	// --- 角丸の箱（ショルダー、Back/Start、十字キーの各腕）---
	auto DrawBox = [&](float x0, float y0, float x1, float y1, const char* label, float glow) {
		const ImVec2 min = ToScreen(x0, y0);
		const ImVec2 max = ToScreen(x1, y1);
		const float rounding = 4.0f * scale;
		drawList->AddRectFilled(min, max, GlowFill(glow), rounding);
		drawList->AddRect(min, max, GlowBorder(glow), rounding, 0, 1.0f);
		DrawHalo(drawList, min, max, rounding, glow);
		DrawCenteredLabel(drawList, min, max, label, LabelColor(glow));
	};

	// --- トリガー（踏み込み量ぶんだけ左から塗る）---
	auto DrawTrigger = [&](PadButton button, float value, const char* label, float x0, float x1) {
		const ImVec2 min = ToScreen(x0, 4.0f);
		const ImVec2 max = ToScreen(x1, 26.0f);
		const float rounding = 4.0f * scale;
		const float glow = Glow(button);

		drawList->AddRectFilled(min, max, GlowFill(0.0f), rounding);
		if (value > 0.0f) {
			// 塗りを内側でクリップするため、角丸は同じ値のままで幅だけ縮める.
			drawList->PushClipRect(min, max, true);
			const ImVec2 fillMax(min.x + (max.x - min.x) * value, max.y);
			drawList->AddRectFilled(min, fillMax, GlowFill(std::max<float>(value, glow)), rounding);
			drawList->PopClipRect();
		}
		drawList->AddRect(min, max, GlowBorder(glow), rounding, 0, 1.0f);
		DrawHalo(drawList, min, max, rounding, glow);
		DrawCenteredLabel(drawList, min, max, label, LabelColor(value));
	};

	// --- スティック ---
	auto DrawStick = [&](PadButton thumb, const Vector2& tilt, float cx, float cy) {
		const ImVec2 center = ToScreen(cx, cy);
		const float radius = 32.0f * scale;
		const float glow = Glow(thumb);
		const float tiltLength = std::min<float>(Vector2::Length(tilt), 1.0f);

		drawList->AddCircleFilled(center, radius, GlowFill(0.0f), 32);
		drawList->AddCircle(center, radius, GlowBorder(glow), 32, 1.0f);
		// 中心の十字。倒していないときの基準位置が分かるように置く.
		const ImU32 crossColor = GlowBorder(0.0f);
		drawList->AddLine(ImVec2(center.x - radius * 0.2f, center.y), ImVec2(center.x + radius * 0.2f, center.y), crossColor);
		drawList->AddLine(ImVec2(center.x, center.y - radius * 0.2f), ImVec2(center.x, center.y + radius * 0.2f), crossColor);

		// スティックのyは上が+なので、スクリーン座標へは符号を反転して落とす.
		const ImVec2 knob(
			center.x + tilt.x * radius * 0.62f,
			center.y - tilt.y * radius * 0.62f
		);
		const float knobRadius = radius * 0.34f;
		drawList->AddLine(center, knob, GlowBorder(tiltLength), 2.0f * scale);
		drawList->AddCircleFilled(knob, knobRadius, GlowFill(std::max<float>(glow, tiltLength)), 24);
		drawList->AddCircle(knob, knobRadius, GlowBorder(glow), 24, 1.5f);
	};

	// --- ABXY（色つき）---
	auto DrawFaceButton = [&](PadButton button, const char* label, float cx, float cy, const ImVec4& color) {
		const ImVec2 center = ToScreen(cx, cy);
		const float radius = 15.0f * scale;
		const float glow = Glow(button);
		// 消灯時は同じ色相のまま暗く落としておくと、どのボタンか色で判別できる.
		const ImVec4 dim(color.x * 0.28f, color.y * 0.28f, color.z * 0.28f, 1.0f);

		drawList->AddCircleFilled(center, radius, ImGui::GetColorU32(Lerp(dim, color, glow)), 24);
		drawList->AddCircle(center, radius, GlowBorder(glow), 24, 1.0f);
		if (glow > 0.01f) {
			ImVec4 halo = color;
			halo.w = 0.4f * glow;
			drawList->AddCircle(center, radius + 3.0f * scale, ImGui::GetColorU32(halo), 24, 3.0f * scale);
		}
		const ImVec2 mini(center.x - radius, center.y - radius);
		const ImVec2 maxi(center.x + radius, center.y + radius);
		DrawCenteredLabel(drawList, mini, maxi, label, LabelColor(glow));
	};

	// ショルダー／トリガー.
	DrawTrigger(PadButton::LeftTrigger, input_->GetPadLeftTrigger(padIndex_), "LT", 45.0f, 115.0f);
	DrawTrigger(PadButton::RightTrigger, input_->GetPadRightTrigger(padIndex_), "RT", 265.0f, 335.0f);
	DrawBox(45.0f, 32.0f, 115.0f, 54.0f, "LB", Glow(PadButton::LeftShoulder));
	DrawBox(265.0f, 32.0f, 335.0f, 54.0f, "RB", Glow(PadButton::RightShoulder));

	// Back / Start.
	DrawBox(168.0f, 100.0f, 196.0f, 118.0f, "Back", Glow(PadButton::Back));
	DrawBox(204.0f, 100.0f, 232.0f, 118.0f, "Start", Glow(PadButton::Start));

	// 十字キー（中心は押せないので枠だけ）.
	DrawBox(129.0f, 184.0f, 151.0f, 206.0f, nullptr, 0.0f);
	DrawBox(129.0f, 160.0f, 151.0f, 184.0f, nullptr, Glow(PadButton::Up));
	DrawBox(129.0f, 206.0f, 151.0f, 230.0f, nullptr, Glow(PadButton::Down));
	DrawBox(105.0f, 184.0f, 129.0f, 206.0f, nullptr, Glow(PadButton::Left));
	DrawBox(151.0f, 184.0f, 175.0f, 206.0f, nullptr, Glow(PadButton::Right));

	// ABXY（Xbox配置）.
	DrawFaceButton(PadButton::Y, "Y", 300.0f, 78.0f, ImVec4(1.00f, 0.80f, 0.10f, 1.0f));
	DrawFaceButton(PadButton::X, "X", 268.0f, 110.0f, ImVec4(0.20f, 0.55f, 1.00f, 1.0f));
	DrawFaceButton(PadButton::B, "B", 332.0f, 110.0f, ImVec4(1.00f, 0.28f, 0.25f, 1.0f));
	DrawFaceButton(PadButton::A, "A", 300.0f, 142.0f, ImVec4(0.30f, 0.85f, 0.35f, 1.0f));

	// スティック.
	DrawStick(PadButton::LeftThumb, input_->GetPadLeftStick(padIndex_), 80.0f, 110.0f);
	DrawStick(PadButton::RightThumb, input_->GetPadRightStick(padIndex_), 250.0f, 195.0f);
}

void InputVisualizerWindow::DrawMouse(ImDrawList* drawList, const ImVec2& origin) {
	const float scale = keyUnit_ / 30.0f;

	auto ToScreen = [&](float x, float y) {
		return ImVec2(origin.x + x * scale, origin.y + y * scale);
	};
	auto DrawPart = [&](float x0, float y0, float x1, float y1, float rounding, const char* label, float glow) {
		const ImVec2 min = ToScreen(x0, y0);
		const ImVec2 max = ToScreen(x1, y1);
		const float r = rounding * scale;
		drawList->AddRectFilled(min, max, GlowFill(glow), r);
		drawList->AddRect(min, max, GlowBorder(glow), r, 0, 1.0f);
		DrawHalo(drawList, min, max, r, glow);
		DrawCenteredLabel(drawList, min, max, label, LabelColor(glow));
	};

	// 本体.
	DrawPart(15.0f, 20.0f, 95.0f, 170.0f, 32.0f, nullptr, 0.0f);
	// 左右ボタン.
	DrawPart(19.0f, 24.0f, 53.0f, 78.0f, 14.0f, "L", mouseGlow_[static_cast<size_t>(MouseButton::Left)]);
	DrawPart(57.0f, 24.0f, 91.0f, 78.0f, 14.0f, "R", mouseGlow_[static_cast<size_t>(MouseButton::Right)]);
	// ホイール（押し込みは中ボタン、回転はwheelGlow_で光らせる）.
	const float wheelGlow = std::max<float>(mouseGlow_[static_cast<size_t>(MouseButton::Middle)], wheelGlow_);
	DrawPart(48.0f, 28.0f, 62.0f, 66.0f, 7.0f, nullptr, wheelGlow);
	// 回した向きを矢印代わりの三角で示す.
	if (wheelGlow_ > 0.01f) {
		const ImVec2 center = ToScreen(55.0f, 47.0f);
		const float size = 6.0f * scale * (wheelDirection_ > 0.0f ? -1.0f : 1.0f);
		drawList->AddTriangleFilled(
			ImVec2(center.x - size, center.y - size),
			ImVec2(center.x + size, center.y - size),
			ImVec2(center.x, center.y + size),
			LabelColor(wheelGlow_)
		);
	}
	// サイドボタン.
	DrawPart(4.0f, 82.0f, 16.0f, 104.0f, 4.0f, nullptr, mouseGlow_[static_cast<size_t>(MouseButton::X1)]);
	DrawPart(4.0f, 108.0f, 16.0f, 130.0f, 4.0f, nullptr, mouseGlow_[static_cast<size_t>(MouseButton::X2)]);
}

ImU32 InputVisualizerWindow::GlowFill(float glow) const {
	const ImVec4 base = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
	return ImGui::GetColorU32(Lerp(base, glowColor_, glow));
}

ImU32 InputVisualizerWindow::GlowBorder(float glow) const {
	const ImVec4 base = ImGui::GetStyle().Colors[ImGuiCol_Border];
	// 押下中は白寄りにして輪郭を立たせる.
	const ImVec4 hot = Lerp(glowColor_, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f);
	return ImGui::GetColorU32(Lerp(base, hot, glow));
}

ImU32 InputVisualizerWindow::LabelColor(float glow) const {
	// 光ると下地が明るくなるので、文字は逆に暗くして読めるようにする.
	const ImVec4 base = ImGui::GetStyle().Colors[ImGuiCol_Text];
	return ImGui::GetColorU32(Lerp(base, ImVec4(0.05f, 0.06f, 0.09f, 1.0f), glow));
}

void InputVisualizerWindow::DrawHalo(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float rounding, float glow) const {
	if (glow <= 0.01f) {
		return;
	}
	ImVec4 halo = glowColor_;
	halo.w = 0.35f * glow;
	const float offset = 2.5f;
	drawList->AddRect(
		ImVec2(min.x - offset, min.y - offset),
		ImVec2(max.x + offset, max.y + offset),
		ImGui::GetColorU32(halo),
		rounding + offset, 0, 3.0f
	);
}

} // namespace Cake

#endif // USE_IMGUI
