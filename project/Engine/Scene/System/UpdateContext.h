#pragma once
/*====================================
 *
 * コンポーネントの Update に渡される、そのフレームの外部情報一式。
 *
 * コンポーネントは engine の各マネージャを直接知らない。
 * 「今フレームに使ってよいもの」だけをここに集めて参照渡しすることで、
 * 依存の入口を1本に絞る。
 *
 * 【入力について】
 * input は必ず有効（SceneManager が生成時に保証する）。
 * キー判定はゲーム用の経路（GetGameKeyState）を通るため、
 * エディタで Game ウィンドウが非フォーカスの間は自動的に None が返る。
 * Raw 系（GetRawKeyState 等）はエディタ専用で、ゲーム側からは使わないこと。
 *
 * 【増やすとき】
 * SoundManager や物理など、コンポーネントに触らせたいものが増えたら
 * ここへメンバを足す。Update のシグネチャは変わらないので既存コードは無傷。
 *
 * 【寿命】
 * このフレームの Update の間だけ有効。メンバに保持してはいけない。
 *
 * ====================================*/
#include "Engine/Platform/Input/KeyCode.h"
#include "Engine/Platform/Input/KeyState.h"

namespace Cake {

class Scene;
class InputManager;
class Time;
class SoundManager;

struct UpdateContext {
	Scene& scene;
	InputManager& input;
	Time& time;
	SoundManager& sound;

	float deltaTime = 0.0f;         // タイムスケール適用済み.
	float unscaledDeltaTime = 0.0f; // ポーズ中も進む。UI演出など向け.

	// --- よく使うキー判定の短縮形。実装は .cpp（InputManager.h を持ち込まないため）---
	KeyState GetKey(KeyCode key) const;
	bool IsKeyHeld(KeyCode key) const; //!< 押されている（押した瞬間を含む）.
	bool IsKeyDown(KeyCode key) const; //!< 押した瞬間だけ true.
	bool IsKeyUp(KeyCode key) const;   //!< 離した瞬間だけ true.
};

} // namespace Cake
