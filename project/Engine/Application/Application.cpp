#include "Application.h"

#include <string>
#include <filesystem>

#include <strsafe.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#include <dxgidebug.h>

#include "Engine/Foundation/Utility/Convert.h"
#include "Engine/Scene/Component/ComponentRegistration.h"
#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"
#include "Game/GameModule.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "Application";

LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = {0};
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

	HANDLE dumpFileHandle = CreateFile(
		filePath, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		0, CREATE_ALWAYS, 0, 0
	);

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{0};
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(
		GetCurrentProcess(), processId,
		dumpFileHandle, MiniDumpNormal,
		&minidumpInformation, nullptr, nullptr
	);

	return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace

Application::D3DResourceLeakChecker::~D3DResourceLeakChecker() {
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
	}
}

void Application::Initialize(HINSTANCE hInstance) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

#ifdef USE_IMGUI
	// DPI 対応として起動する。ウィンドウ生成前に呼ぶこと.
	ImGui_ImplWin32_EnableDpiAwareness();
#endif

	// クラッシュダンプの設定.
	SetUnhandledExceptionFilter(ExportDump);

	// 設定の読み込み。失敗しても既定値で続行する.
	if (!settings_.Load()) {
		DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, "既定のプロジェクト設定で起動します");
#ifdef ENABLE_EDITOR
		settings_.Save(); // 初回起動でファイルを作っておく（エディタから編集できるようにする）.
#endif
	}

	const WindowPlacement* placement = nullptr;
#ifdef ENABLE_EDITOR
	preferences_.Load();
	cache_.Load();

	WindowPlacement restored{};
	if (cache_.HasWindowRect()) {
		restored.x = cache_.windowX;
		restored.y = cache_.windowY;
		restored.width = cache_.windowWidth;
		restored.height = cache_.windowHeight;
		restored.maximized = cache_.maximized;
		placement = &restored;
	}
#endif

	// WindowAppの初期化.
	windowApp_.Initialize(
		ConvertString(settings_.windowTitle.c_str()).c_str(),
		settings_.windowWidth, settings_.windowHeight,
		placement
	);
	HWND hwnd = windowApp_.GetHwnd();

#ifndef ENABLE_EDITOR
	// フルスクリーンはゲーム本番のみ。エディタはウィンドウでないと作業できない.
	if (settings_.fullscreen) {
		windowApp_.SetFullscreen(true);
	}
#endif

	// 実際に開いたクライアント領域を、ここで唯一の基準として確定する.
	{
		RECT client{};
		GetClientRect(hwnd, &client);
		clientWidth_ = static_cast<uint32_t>(client.right - client.left);
		clientHeight_ = static_cast<uint32_t>(client.bottom - client.top);
	}
	// 最小化状態で起動した場合などの保険。0 のままではスワップチェーンを作れない.
	if (clientWidth_ == 0 || clientHeight_ == 0) {
		clientWidth_ = settings_.windowWidth;
		clientHeight_ = settings_.windowHeight;
	}

	// Platform層の初期化.
	platform_.Initialize(hInstance, hwnd);
	// InputManagerを登録.
	windowApp_.SetInputManager(&platform_.GetInput());

	// Graphics層の初期化.
	graphics_.Initialize(hwnd, clientWidth_, clientHeight_);
	// ユーザー定義シェーダーの登録.
	// マテリアル読み込みでシェーダ名を解決するため、AssetContext より前に登録する.
	Game::RegisterGameShaders(*graphics_.GetShaderLibrary());

	// Asset層の初期化.
	AssetInitializeDesc assetDesc;
	assetDesc.device = graphics_.GetDevice()->GetD3DDevice();
	assetDesc.srvHeap = graphics_.GetSrvHeap();
	assetDesc.commandManager = graphics_.GetCommandManager();
	assetDesc.shaderLibrary = graphics_.GetShaderLibrary();
	assetDesc.rootPath = "Assets";

	asset_.Initialize(assetDesc);

	// Render層の初期化.
	RendererInitDesc rendererDesc;
	rendererDesc.device = graphics_.GetDevice()->GetD3DDevice();
	rendererDesc.swapChain = graphics_.GetSwapChain();
	rendererDesc.depthBuffer = graphics_.GetDepthBuffer();
	rendererDesc.srvHeap = graphics_.GetSrvHeap();
	rendererDesc.pipelineState = graphics_.GetPipelineState();
	rendererDesc.textureManager = asset_.GetTextureManager();
	rendererDesc.materialManager = asset_.GetMaterialManager();
	rendererDesc.modelManager = asset_.GetModelManager();
	rendererDesc.clientWidth = clientWidth_;
	rendererDesc.clientHeight = clientHeight_;

	renderer_.Initialize(rendererDesc);

	// 組み込みコンポーネントの登録.
	RegisterAllComponents();
	// ユーザー定義コンポーネントの登録.
	// シーン読み込みより前に登録する.
	Game::RegisterGameComponents();
	// 登録の締め切り。エンジン側とゲーム側の両方がそろってから依存宣言を検査する.
	// これより後の Register は受け付けられない.
	TypeRegistry::GetInstance().FinalizeRegistration();

	// Scene層の初期化.
	sceneRenderer_.Initialize(&renderer_);
	sceneManager_.Initialize(&platform_, asset_.GetAssetDatabase());

	// 初期シーンの読み込み。
	// エディタでは前回開いていたシーンを優先する（作業の続きから始められるようにする）.
	std::string scenePath = settings_.GetStartupScene();
#ifdef ENABLE_EDITOR
	if (!cache_.openScenePath.empty()) {
		std::error_code ec;
		if (std::filesystem::exists(cache_.openScenePath, ec)) {
			scenePath = cache_.openScenePath;
		} else {
			DebugLog::GetInstance().Log(
				LogLevel::Warn, kLogCategory,
				"前回のシーンが見つかりません: " + cache_.openScenePath
			);
		}
	}
#endif
	if (!scenePath.empty()) {
		sceneManager_.LoadScene(scenePath);
	}

#ifdef ENABLE_EDITOR
	// Editor層の初期化.
	editor_.Initialize(
		hwnd, platform_, graphics_, asset_, renderer_, sceneRenderer_, sceneManager_,
		settings_, preferences_, cache_
	);
#endif

	// 初期化にかかった時間を1フレーム目へ流さない.
	platform_.GetTime().Reset();

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}
void Application::Run() {
	while (windowApp_.ProcessMessage()) {
		// メインループ.
		RunFrame();
	}

	// GPUが参照を終えるのを待つ.
	graphics_.GetCommandManager()->WaitForGPU();

#ifdef ENABLE_EDITOR
	// 次回起動用に状態を書き出す。Editor はこの後の破棄まで生きているので触ってよい.
	editor_.CaptureCache(cache_);

	WindowPlacement placement{};
	if (windowApp_.GetPlacement(placement)) {
		cache_.windowX = placement.x;
		cache_.windowY = placement.y;
		cache_.windowWidth = placement.width;
		cache_.windowHeight = placement.height;
		cache_.maximized = placement.maximized;
	}
	cache_.Save();
	preferences_.Save();
#endif
}

void Application::RunFrame() {
	// 前のフレーム以降に出たログ（ワーカーの分も含む）を、画面表示用の一覧へ移す.
	DebugLog::GetInstance().PumpMainThread();

	// Platform層のフレーム開始.
	platform_.BeginFrame();
	Time& time = platform_.GetTime();

#ifdef ENABLE_EDITOR
	/* Debugビルドのみ */
	// エディタ用のレンダーターゲットを作り直す.
	editor_.PrepareRenderTargets();
#endif

	// ウィンドウサイズに変更がないか調べ、変更されていた場合リサイズする.
	CheckResize();

#ifndef ENABLE_EDITOR
	/* Releaseビルドのみ */
	//
	if (clientWidth_ != lastDepthWidth_ || clientHeight_ != lastDepthHeight_) {
		graphics_.ResizeDepthBuffer(clientWidth_, clientHeight_);
		lastDepthWidth_ = clientWidth_;
		lastDepthHeight_ = clientHeight_;
	}
#endif

	// Render層のフレーム開始.
	renderer_.BeginFrame(graphics_.GetCommandManager());

#ifdef ENABLE_EDITOR
	/* Debugビルドのみ */
	editor_.BeginFrame(time);
	editor_.Update(time);
	editor_.Draw();
	editor_.EndFrame();
#else
	/* Releaseビルドのみ */
	// バックバッファの準備.
	renderer_.PrepareBackbufferToGame(clientWidth_, clientHeight_, settings_.GetAspectRatio());

	// シーンの更新.
	sceneManager_.Update(time.GetDeltaTime(), time.GetUnscaledDeltaTime());

	// シーンを描画.
	CameraView view{};
	if (sceneRenderer_.TryMakeMainCameraView(
			sceneManager_.GetScene(), static_cast<uint32_t>(renderer_.GetDrawWidth()), static_cast<uint32_t>(renderer_.GetDrawHeight()), view
		)) {
		sceneRenderer_.Render(sceneManager_.GetScene(), view);
	}
#endif

	// Render層のフレーム終了.
	renderer_.EndFrame(graphics_.GetCommandManager());

#ifdef ENABLE_EDITOR
	/* Debugビルドのみ */
	graphics_.GetPerformanceProfiler()->Resolve();
#endif
}

void Application::CheckResize() {
	RECT rect{};
	GetClientRect(windowApp_.GetHwnd(), &rect);
	uint32_t width = static_cast<uint32_t>(rect.right - rect.left);
	uint32_t height = static_cast<uint32_t>(rect.bottom - rect.top);

	// 最小化やゼロサイズはスキップ（ResizeBuffersが失敗するため）.
	if (width == 0 || height == 0) {
		return;
	}
	// 変化がなければ何もしない.
	if (width == clientWidth_ && height == clientHeight_) {
		return;
	}

	Resize(width, height);
}
void Application::Resize(uint32_t width, uint32_t height) {
	// 各バッファを作り直す.
	graphics_.Resize(width, height);
	renderer_.Resize(width, height);

	// 保存サイズを更新.
	clientWidth_ = width;
	clientHeight_ = height;
}

} // namespace Cake
