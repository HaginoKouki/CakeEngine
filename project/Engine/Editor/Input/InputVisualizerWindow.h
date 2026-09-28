#pragma once
/*====================================
 *
 * 入力状態をキーボード／ゲームパッド／マウスの絵で可視化するデバッグウィンドウ。
 * InputManagerの状態を毎フレーム読み、押されているキーやボタンを光らせる。
 * 光量は押下中1.0で、離すとfadeTime_秒かけて減衰するため直前の入力も目で追える。
 * 入力の取りこぼし・意図しない同時押し・スティックのデッドゾーンの効き具合を
 * ログを出さずに確認するためのもの。
 *
 * キー位置はKeyCode（スキャンコード）基準なので、配列を変えても位置は崩れない。
 * Layout設定で変わるのは刻印（ラベル）とJIS固有キーの有無だけ。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef USE_IMGUI

#include "externals/imgui/imgui.h"

#include "Engine/Platform/Input/InputManager.h"

namespace Cake {

// キーボードの刻印セット。物理位置はどちらも同じスキャンコードを使う.
enum class KeyboardLayoutType {
	US = 0,  // ANSI配列.
	JIS = 1, // 日本語配列（¥・無変換・変換・かなが増える）.
};

class InputVisualizerWindow {
private:
	InputManager* input_ = nullptr; // 非所有.

	// --- 表示設定 ---
	KeyboardLayoutType layout_ = KeyboardLayoutType::JIS;
	int padIndex_ = 0;                                 // 表示対象のパッド番号(0〜3).
	float keyUnit_ = 26.0f;                            // 標準キー1個分のピクセル数.
	float fadeTime_ = 0.35f;                           // 離してから消えるまでの秒数.
	ImVec4 glowColor_ = {0.20f, 0.75f, 1.00f, 1.00f};  // 光る色.
	bool showKeyboard_ = true;
	bool showGamePad_ = true;
	bool showMouse_ = true;

	// --- 光量(0〜1) ---
	float keyGlow_[256] = {};  // 添字はスキャンコード。KeyCodeをそのままキャストして引く.
	float padGlow_[16] = {};   // 添字はPadButtonのビット位置(0〜15).
	float mouseGlow_[5] = {};  // 添字はMouseButtonの値(0〜4).
	float wheelGlow_ = 0.0f;
	float wheelDirection_ = 0.0f; // 最後に回した向き(+1:奥 / -1:手前).

public:
	void Initialize(InputManager* input);

	/// <param name="title">ウィンドウ名</param>
	/// <param name="open">閉じるボタン用。nullptrなら閉じられない</param>
	void Draw(const char* title = "Input Visualizer", bool* open = nullptr);

private:
	/// 押下状態を光量へ反映し、押されていないものを減衰させる.
	void UpdateGlow(float deltaTime);

	void DrawKeyboard(ImDrawList* drawList, const ImVec2& origin);
	void DrawGamePad(ImDrawList* drawList, const ImVec2& origin);
	void DrawMouse(ImDrawList* drawList, const ImVec2& origin);

	// 光量から色を作る.
	ImU32 GlowFill(float glow) const;
	ImU32 GlowBorder(float glow) const;
	ImU32 LabelColor(float glow) const;

	// 押下中だけ外側へ薄い輪を重ね、光がにじんで見えるようにする.
	void DrawHalo(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float rounding, float glow) const;
};

} // namespace Cake

#endif // USE_IMGUI
