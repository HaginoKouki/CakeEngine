#pragma once
/*====================================
 *
 * ImGuiフレームワークのDirect3D12対応初期化・フレーム管理を行うクラス。
 * BeginFrame/EndFrameで毎フレームのImGui描画開始・終了処理を実施。
 * エディタUI（マテリアルエディタ、パフォーマンスプロファイラ等）の描画基盤となる。
 *
 * ====================================*/
#ifdef USE_IMGUI

#include <string>
#include <array>
#include <algorithm>
#include <d3d12.h>

#include <imgui.h>
#include "externals/IconFontCppHeaders/IconsFontAwesome7.h"

#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Transform.h"
#include "Engine/Graphics/Device/CommandManager.h"
#include "Engine/Graphics/Descriptor/SrvManager.h"
#include "Engine/Editor/Settings/EditorPreferences.h"


namespace Cake {

class DirectXDevice;
class CommandManager;
class SwapChain;

class ImGuiManager {
private:
	enum Theme {
		kDark,
		kThemeCount
	};

	SrvManager* srvManager_ = nullptr;
	HWND hwnd_ = nullptr;         // DPI の問い合わせに使う.
	float uiScale_ = 1.0f;        // 実際に適用中の倍率.
	float systemDpiScale_ = 1.0f; // OS から取得した拡大率のキャッシュ.

	// ImGui のテクスチャ用 SRV 枠（初期化時に SrvManager から kSrvImGuiCount 個確保する）.
	// バックエンドがテクスチャを作るたびにコールバックから貸し出し、破棄されたら空きに戻す.
	struct SrvSlot {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
		bool inUse = false;
	};
	static constexpr uint32_t kSrvImGuiCount = 4;
	std::array<SrvSlot, kSrvImGuiCount> srvSlots_{};

	const float kBaseStyleScale = 0.889f;
	const float kBaseFontSize = 14.0f;
	const float kLoadFontSize = 36.0f;
	const char* kFallBackFont = "EngineResources/fonts/Inter/Inter_18pt-Regular.ttf";
	const char* kEnFont = "EngineResources/fonts/Inter/Inter_18pt-Regular.ttf";
	const char* kJpFont = "EngineResources/fonts/NotoSansJP/NotoSansJP-Regular.ttf";
	const char* kIconFont = "EngineResources/fonts/FontAwesome7Free-Solid-900.otf";
	const char* kLogFont = "EngineResources/fonts/HackGen/HackGenConsole-Regular.ttf";

	ImGuiStyle defaultStyle_;

	ImFont* mainFont_ = nullptr;
	ImFont* consoleFont_ = nullptr;

public:
	~ImGuiManager();

	void Initialize(
		HWND hwnd,
		DirectXDevice* device,
		CommandManager* commandManager,
		SwapChain* swapChain,
		SrvManager* srvManager
	);

	void BeginFrame();              // NewFrame 3点セット
	void EndFrame(CommandManager*); // Render → RenderDrawData

	void BeginDockSpace();

	// 倍率を直接指定する（実適用値の範囲へクランプされる）.
	void ApplyUIScale(float scale);

	// プリファレンスの内容から倍率を決めて適用する.
	void ApplyUIScaleFromPreferences(const EditorPreferences& preferences);

	// 設定と実際の倍率を毎フレーム突き合わせる。差があるときだけ組み直す.
	void RefreshSystemDpi(const EditorPreferences& preferences);

	float GetUIScale() const { return uiScale_; }
	float GetSystemDpiScale() const { return systemDpiScale_; }
	ImFont* GetMainFont() { return mainFont_; }
	ImFont* GetConsoleFont() { return consoleFont_; }

	// 配色を切り替える。現在のUIスケールは内部で再適用される.
	void ApplyTheme(EditorTheme theme);

private:
	float FetchSystemDpiScale() const;
	float ResolveUIScale(const EditorPreferences& preferences) const;

	void ApplyLightTheme();
	void ApplyDarkTheme();
};

} // namespace Cake

#endif // USE_IMGUI
