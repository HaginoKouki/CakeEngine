#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>

namespace Cake {

class InputManager;

// ウィンドウの位置と大きさ。枠を含む「ウィンドウ矩形」であり、クライアント矩形ではない.
struct WindowPlacement {
	int x = 0;
	int y = 0;
	int width = 0; // 0 なら未指定.
	int height = 0;
	bool maximized = false;

	bool IsValid() const { return width > 0 && height > 0; }
};

class WindowApp {
private:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	WNDCLASS wc_{};
	HWND hwnd_ = nullptr;
	uint32_t clientWidth_ = 0;
	uint32_t clientHeight_ = 0;

	InputManager* input_ = nullptr; // 所有しない。Engineが持つものを参照する.

	// 最後に確認したウィンドウ矩形。WM_DESTROY でも控えるので、
	// HWND が消えた後でも終了時のキャッシュ保存に使える.
	WindowPlacement lastPlacement_{};

	bool fullscreen_ = false;
	LONG_PTR windowedStyle_ = 0; // フルスクリーン解除時に戻すスタイル.

public:
	// placement が有効ならその位置・大きさで開く（clientWidth/Height は無視される）.
	void Initialize(
		const wchar_t* title,
		uint32_t clientWidth,
		uint32_t clientHeight,
		const WindowPlacement* placement = nullptr
	);
	void Finalize();

	// メッセージ処理。WM_QUITが来たらfalseを返す.
	bool ProcessMessage();

	/// WndProcが受けた入力メッセージの転送先を設定する。Engine初期化後に呼ぶ.
	void SetInputManager(InputManager* input) { input_ = input; }

	// UTF-8 の文字列でタイトルを差し替える.
	void SetTitle(const std::string& utf8Title);

	// ボーダレス全画面を切り替える.
	void SetFullscreen(bool enable);
	bool IsFullscreen() const { return fullscreen_; }

	// 現在（HWND が生きていれば最新、死んでいれば破棄直前）の矩形を返す.
	// 一度も控えられていなければ false.
	bool GetPlacement(WindowPlacement& out);

	HWND GetHwnd() const { return hwnd_; }
	HINSTANCE GetHInstance() const { return wc_.hInstance; }
	uint32_t GetClientWidth() const { return clientWidth_; }
	uint32_t GetClientHeight() const { return clientHeight_; }

private:
	// ウィンドウ矩形を lastPlacement_ へ控える。フルスクリーン中は何もしない.
	void CapturePlacement();
};

} // namespace Cake
