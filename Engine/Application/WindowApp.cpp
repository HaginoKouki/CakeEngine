#include "WindowApp.h"
#include <algorithm>
#include <cassert>

#ifdef USE_IMGUI
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
#endif

#include "Engine/Foundation/Utility/Convert.h"
#include "Engine/Platform/Input/InputManager.h"

namespace Cake {
namespace {

// 保存時と違うモニタ構成で起動した場合に、画面外へ復元されるのを防ぐ.
void ClampToMonitor(WindowPlacement& placement) {
	RECT rect = {
		placement.x, placement.y,
		placement.x + placement.width, placement.y + placement.height
	};
	HMONITOR monitor = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);

	MONITORINFO info{};
	info.cbSize = sizeof(info);
	if (!GetMonitorInfo(monitor, &info)) {
		return;
	}

	const LONG workWidth = info.rcWork.right - info.rcWork.left;
	const LONG workHeight = info.rcWork.bottom - info.rcWork.top;

	placement.width = (std::min)(placement.width, static_cast<int>(workWidth));
	placement.height = (std::min)(placement.height, static_cast<int>(workHeight));
	placement.x = std::clamp(placement.x, static_cast<int>(info.rcWork.left), static_cast<int>(info.rcWork.right) - placement.width);
	placement.y = std::clamp(placement.y, static_cast<int>(info.rcWork.top), static_cast<int>(info.rcWork.bottom) - placement.height);
}

} // namespace

void WindowApp::Initialize(
	const wchar_t* title,
	uint32_t clientWidth,
	uint32_t clientHeight,
	const WindowPlacement* placement
) {
	clientWidth_ = clientWidth;
	clientHeight_ = clientHeight;

	// ウィンドウクラスの登録.
	wc_.lpfnWndProc = WindowProc;
	wc_.lpszClassName = L"CG2WindowClass";
	wc_.hInstance = GetModuleHandle(nullptr);
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc_);

	// ウィンドウサイズの計算.
	RECT wrc = {0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight)};
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	int posX = CW_USEDEFAULT;
	int posY = CW_USEDEFAULT;
	int width = wrc.right - wrc.left;
	int height = wrc.bottom - wrc.top;
	bool maximized = false;

	// キャッシュがあれば、そちらを優先する。
	// 保存されているのは AdjustWindowRect 済みの値なので、ここで再計算してはいけない.
	if (placement != nullptr && placement->IsValid()) {
		WindowPlacement restored = *placement;
		ClampToMonitor(restored);
		posX = restored.x;
		posY = restored.y;
		width = restored.width;
		height = restored.height;
		maximized = restored.maximized;
	}

	// ウィンドウの生成と表示.
	hwnd_ = CreateWindow(
		wc_.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW,
		posX, posY,
		width, height,
		nullptr, nullptr,
		wc_.hInstance,
		this // WindowProcからインスタンスを引くために渡す.
	);
	assert(hwnd_ != nullptr);
	ShowWindow(hwnd_, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);

	// 実際のクライアント領域を控え直す。
	// 最大化復元では指定値と食い違うため、これを使わないと解像度がずれる.
	RECT client{};
	GetClientRect(hwnd_, &client);
	clientWidth_ = static_cast<uint32_t>(client.right - client.left);
	clientHeight_ = static_cast<uint32_t>(client.bottom - client.top);

	CapturePlacement();
}

void WindowApp::Finalize() {
	CloseWindow(hwnd_);
	hwnd_ = nullptr;
}

void WindowApp::SetTitle(const std::string& utf8Title) {
	if (hwnd_ == nullptr) {
		return;
	}
	SetWindowTextW(hwnd_, ConvertString(utf8Title.c_str()).c_str());
}

void WindowApp::SetFullscreen(bool enable) {
	if (hwnd_ == nullptr || fullscreen_ == enable) {
		return;
	}

	if (enable) {
		// 戻す先を先に控える（fullscreen_ を立てる前でないと CapturePlacement が素通りする）.
		CapturePlacement();
		windowedStyle_ = GetWindowLongPtr(hwnd_, GWL_STYLE);

		HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info{};
		info.cbSize = sizeof(info);
		if (!GetMonitorInfo(monitor, &info)) {
			return;
		}

		SetWindowLongPtr(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(
			hwnd_, HWND_TOP,
			info.rcMonitor.left, info.rcMonitor.top,
			info.rcMonitor.right - info.rcMonitor.left,
			info.rcMonitor.bottom - info.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER
		);
		fullscreen_ = true;
	} else {
		SetWindowLongPtr(
			hwnd_, GWL_STYLE,
			windowedStyle_ != 0 ? windowedStyle_ : static_cast<LONG_PTR>(WS_OVERLAPPEDWINDOW)
		);
		fullscreen_ = false;

		if (lastPlacement_.IsValid()) {
			SetWindowPos(
				hwnd_, nullptr,
				lastPlacement_.x, lastPlacement_.y,
				lastPlacement_.width, lastPlacement_.height,
				SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER
			);
		}
		ShowWindow(hwnd_, lastPlacement_.maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
	}
}

bool WindowApp::GetPlacement(WindowPlacement& out) {
	CapturePlacement(); // HWND が生きていれば最新へ更新する.
	if (!lastPlacement_.IsValid()) {
		return false;
	}
	out = lastPlacement_;
	return true;
}

void WindowApp::CapturePlacement() {
	// フルスクリーン中の矩形を控えると、次回起動で枠なし全画面のサイズが
	// 通常ウィンドウとして復元されてしまう.
	if (hwnd_ == nullptr || fullscreen_) {
		return;
	}

	WINDOWPLACEMENT wp{};
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(hwnd_, &wp)) {
		return;
	}

	// rcNormalPosition は最大化中でも「元のサイズ」を返す。GetWindowRect では取れない.
	lastPlacement_.x = wp.rcNormalPosition.left;
	lastPlacement_.y = wp.rcNormalPosition.top;
	lastPlacement_.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	lastPlacement_.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	lastPlacement_.maximized = (wp.showCmd == SW_SHOWMAXIMIZED);
}

bool WindowApp::ProcessMessage() {
	MSG msg{};
	// キューが空になるまで処理する。1件ずつだと入力とWM_QUITの検出が遅れる.
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		// WM_QUITはウィンドウ宛てではないので、Dispatchせず即座に抜ける.
		if (msg.message == WM_QUIT) {
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

LRESULT CALLBACK WindowApp::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// CreateWindowで渡したthisを保存し、以降のメッセージで取り出せるようにする.
	if (msg == WM_NCCREATE) {
		auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}
	auto* self = reinterpret_cast<WindowApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return 1;
	}
#endif

	// 入力を記録する.
	if (self && self->input_) {
		self->input_->HandleMessage(msg, wParam, lParam);
	}

	switch (msg) {
		case WM_DESTROY:
			// WM_QUIT を受け取る頃には HWND が消えているため、ここで控えておく。
			// キャッシュへ書けるウィンドウ位置はこのタイミングが最後.
			if (self) {
				self->CapturePlacement();
			}
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

} // namespace Cake
