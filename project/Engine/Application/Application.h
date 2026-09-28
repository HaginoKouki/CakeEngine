#pragma once
#include <Windows.h>
#include <cstdint>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/SceneRenderer.h"
#include "Engine/Scene/SceneManager.h"
#ifdef ENABLE_EDITOR
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Settings/EditorCache.h"
#include "Engine/Editor/Settings/EditorPreferences.h"
#endif
#include "Engine/Application/WindowApp.h"
#include "Engine/Application/ProjectSettings.h"

namespace Cake {

class Application {
private:
	struct ComScope {
		ComScope() { AssertHRESULT(CoInitializeEx(0, COINIT_MULTITHREADED), "COMの初期化"); }
		~ComScope() { CoUninitialize(); }
	};
	struct D3DResourceLeakChecker {
		~D3DResourceLeakChecker();
	};
	// ===== 宣言順＝初期化順。破棄は逆順に起きる =====
	ComScope comScope_;
	D3DResourceLeakChecker leakChecker_;

	ProjectSettings settings_;
#ifdef ENABLE_EDITOR
	// ウィンドウ生成より前に読む必要があるため、Editor ではなく Application が持つ.
	EditorPreferences preferences_;
	EditorCache cache_;
#endif
	WindowApp windowApp_;

	Platform platform_;
	Graphics graphics_;
	Asset asset_;
	Renderer renderer_;

	SceneRenderer sceneRenderer_;
	SceneManager sceneManager_;

#ifdef ENABLE_EDITOR
	// ImGui のシャットダウンと RT の解放がデバイス破棄より先に走る必要があるため、
	// graphics_ より後に宣言すること.
	Editor editor_;
#endif

	uint32_t clientWidth_ = 0;
	uint32_t clientHeight_ = 0;
	uint32_t lastDepthWidth_ = 0;
	uint32_t lastDepthHeight_ = 0;

public:
	void Initialize(HINSTANCE hInstance);

	// メインループそのもの.
	void Run();

private:
	// 毎フレーム行われる処理.
	void RunFrame();

	// ウィンドウサイズに変更が無いか調べ、あれば描画リソース等をリサイズする.
	void CheckResize();
	// 描画リソース等をリサイズする.
	void Resize(uint32_t width, uint32_t height);
};

} // namespace Cake
