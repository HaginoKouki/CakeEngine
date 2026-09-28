#include "ImGuiManager.h"

#ifdef USE_IMGUI

#include <format>

#include "Engine/Graphics/RenderTarget/SwapChain.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Editor/imgui/Icons.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "ImGuiManager";
}
namespace {
// HSVの明度(V)をずらして、Hover/Active用の派生色を作る.
ImVec4 Brighten(const ImVec4& base, float amount) {
	float h, s, v;
	ImGui::ColorConvertRGBtoHSV(base.x, base.y, base.z, h, s, v);
	v = std::clamp(v + amount, 0.0f, 1.0f);
	ImVec4 out;
	ImGui::ColorConvertHSVtoRGB(h, s, v, out.x, out.y, out.z);
	out.w = base.w;
	return out;
}

// アルファだけ差し替えたコピー（選択背景など半透明が欲しい所用）.
ImVec4 WithAlpha(const ImVec4& base, float alpha) {
	return ImVec4(base.x, base.y, base.z, alpha);
}
} // namespace

void ImGuiManager::Initialize(
	HWND hwnd,
	ID3D12Device* device,
	SwapChain* swapChain,
	DescriptorHeap* srvHeap
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	srvHeap_ = srvHeap;
	hwnd_ = hwnd;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// 英語用フォント（Inter）を追加.
	io.Fonts->AddFontFromFileTTF(kEnFont, kLoadFontSize, nullptr, io.Fonts->GetGlyphRangesDefault());

	// 日本語用フォント（NotoSansJP）をマージ.
	ImFontConfig jpConfig;
	jpConfig.MergeMode = true; // マージモードを有効化
	// 英語用フォントに日本語フォントの文字データを上書き・追加する
	io.Fonts->AddFontFromFileTTF(kJpFont, kLoadFontSize, &jpConfig, io.Fonts->GetGlyphRangesJapanese());

	// FontAwesomeをマージ
	static const ImWchar kIconRanges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
	ImFontConfig iconConfig{};
	iconConfig.MergeMode = true;                 // 直前のフォントに合成する.
	iconConfig.PixelSnapH = true;                // 横位置をピクセルに吸着させる.
	iconConfig.GlyphMinAdvanceX = kBaseFontSize; // アイコンを等幅化（ツールバーの桁揃え用）.
	io.Fonts->AddFontFromFileTTF(kIconFont, kLoadFontSize, &iconConfig, kIconRanges);

	// フォントを確定.
	io.Fonts->Build();


	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // ドッキング有効化.
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // OSウィンドウ外へ引き出す機能。DX12は追加実装が要るので今回はOFF.

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(
		device,
		swapChain->GetBufferCount(),
		swapChain->GetRTVFormat(),
		srvHeap->GetHeap(),
		srvHeap->GetCPUHandle(0),
		srvHeap->GetGPUHandle(0)
	);

	ApplyDarkTheme();

	// ImGui 既定のスタイルを「スケール 1.0 の基準」として控える.
	defaultStyle_ = ImGui::GetStyle();

	// OS の拡大率を控える。プリファレンス適用前の暫定値として使う.
	systemDpiScale_ = FetchSystemDpiScale();

	ApplyTheme(EditorTheme::Dark);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void ImGuiManager::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::EndFrame(CommandManager* commandManager) {
	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList();
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::BeginDockSpace() {
	// メインビューポート全体を覆う"土台"を作り、その上にDockSpaceを敷く.
	// これ一発でホストウィンドウの定型処理を全部やってくれる.
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

ImGuiManager::~ImGuiManager() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::ApplyUIScale(float scale) {
	uiScale_ = std::clamp(scale, EditorPreferences::kMinUIScale, EditorPreferences::kMaxUIScale);

	ImGuiIO& io = ImGui::GetIO();

	// 1. 文字（フォント）のグローバルスケール。
	// 「基本サイズ / ロードサイズ」をベースに、指定倍率を掛ける。
	// 36px で生成した輪郭を維持したまま、見た目だけを変える.
	io.FontGlobalScale = (kBaseFontSize / kLoadFontSize) * uiScale_;

	// 2. ボタン・ウィンドウ・隙間のサイズを連動させる。
	// ScaleAllSizes は呼ぶたびに乗算されるため、必ず一度基準へ戻してから掛ける.
	ImGuiStyle& style = ImGui::GetStyle();

	// defaultStyle_ には「基準を控えた時点の配色」も入っている。
	// 丸ごと代入するとテーマ切り替えの結果が巻き戻るので、現在の配色だけ退避して書き戻す.
	const ImGuiStyle current = style;

	style = defaultStyle_;
	for (int i = 0; i < ImGuiCol_COUNT; ++i) {
		style.Colors[i] = current.Colors[i];
	}

	style.ScaleAllSizes(uiScale_ * kBaseStyleScale);
}

void ImGuiManager::ApplyUIScaleFromPreferences(const EditorPreferences& preferences) {
	systemDpiScale_ = FetchSystemDpiScale();
	ApplyUIScale(ResolveUIScale(preferences));
}

void ImGuiManager::ApplyTheme(EditorTheme theme) {
	// テーマ関数は角丸や枠線の太さ（＝サイズ）も書き換える。
	// スケール済みの値の上に適用すると倍率が混ざるため、一度基準サイズへ戻してから適用する.
	ImGui::GetStyle() = defaultStyle_;

	switch (theme) {
		case EditorTheme::Light:
			ApplyLightTheme();
			break;
		case EditorTheme::Dark:
		default:
			ApplyDarkTheme();
			break;
	}

	// テーマが決めたサイズを、新しい「スケール 1.0 の基準」として控え直す.
	defaultStyle_ = ImGui::GetStyle();

	// 現在の倍率を再適用する.
	ApplyUIScale(uiScale_);
}

float ImGuiManager::FetchSystemDpiScale() const {
	if (hwnd_ == nullptr) {
		return 1.0f;
	}
	// プロセスが DPI 非対応の場合、この値は常に 1.0 になる（Windows 側が画面ごと引き伸ばすため）.
	const float scale = ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd_);
	return (scale > 0.0f) ? scale : 1.0f;
}

float ImGuiManager::ResolveUIScale(const EditorPreferences& preferences) const {
	// 置き換え方式。追従中は手動値を混ぜない.
	return preferences.followSystemDpi ? systemDpiScale_ : preferences.uiScale;
}

void ImGuiManager::RefreshSystemDpi(const EditorPreferences& preferences) {
	// 設定画面に現在値を出すため、追従が切れていてもキャッシュは毎回更新する.
	systemDpiScale_ = FetchSystemDpiScale();

	// 「OSの値が変化したか」ではなく「目標倍率と実適用倍率が一致しているか」で判定する。
	// 変化検出にすると、起動時に適用し損ねた状態がそのまま固定されてしまう.
	const float target = ResolveUIScale(preferences);
	if (std::fabs(target - uiScale_) < 0.001f) {
		return; // 既に目的の倍率。スタイルを組み直さない.
	}

	ApplyUIScale(target);
}

void ImGuiManager::ApplyLightTheme() {
	ImGuiStyle& Style = ImGui::GetStyle();
	ImGui::StyleColorsLight();

	const float Hue = 0.0f;              // [0,1] range.
	const float Saturation = 1.0f;       // [0,6] range.
	const float SaturationAccent = 1.0f; // [0,6] range.
	const float Transparency = 1.0f;     // Unityは基本的に不透明なので1.0fに調整
	const float BorderSize = 1.0f;       // Unityの境界線用に1.0fに変更

	Style.FrameBorderSize = BorderSize;
	Style.ImageBorderSize = 0.0f;
	Style.TabBorderSize = BorderSize;
	Style.TabBarBorderSize = 1.0f;
	Style.WindowRounding = 0.0f;
	Style.ChildRounding = 0.0f;
	Style.FrameRounding = 1.0f;
	Style.GrabRounding = 1.0f;
	Style.TabRounding = 1.0f;

	Style.DockingSeparatorSize = 2.0f;

	const ImVec4 TransparentColor = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	const ImVec4 BackgroundColor = ImVec4(0.94f, 0.94f, 0.95f, 1.00f);
	const ImVec4 BackgroundTransparentColor = ImVec4(0.94f, 0.94f, 0.95f, 0.00f);
	const ImVec4 BackgroundChildColor = ImVec4(0.98f, 0.98f, 0.99f, 1.00f);
	const ImVec4 BackgroundChildTransparentColor = ImVec4(0.98f, 0.98f, 0.99f, 0.55f);
	const ImVec4 BackgroundDimmedColor = ImVec4(0.30f, 0.30f, 0.32f, 0.45f);
	const ImVec4 TitleColor = ImVec4(0.82f, 0.82f, 0.84f, 1.00f);
	const ImVec4 TitleTransparentColor = ImVec4(0.82f, 0.82f, 0.84f, 0.60f);
	const ImVec4 HeaderColor = ImVec4(0.87f, 0.87f, 0.89f, 1.00f);
	const ImVec4 HeaderTransparentColor = ImVec4(0.87f, 0.87f, 0.89f, 0.70f);
	const ImVec4 TextColor = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	const ImVec4 TextDimmedColor = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
	const ImVec4 BorderColor = ImVec4(0.72f, 0.72f, 0.75f, 1.00f);
	const ImVec4 Accent1Color = ImVec4(0.26f, 0.52f, 0.88f, 1.00f);
	const ImVec4 Accent2Color = ImVec4(0.17f, 0.40f, 0.75f, 1.00f);
	const ImVec4 Accent1AlternativeColor = Brighten(Accent1Color, 0.08f);
	const ImVec4 Accent2AlternativeColor = Brighten(Accent2Color, 0.08f);

	Style.Colors[ImGuiCol_Text] = TextColor;
	Style.Colors[ImGuiCol_TextDisabled] = TextDimmedColor;
	Style.Colors[ImGuiCol_WindowBg] = BackgroundColor;
	Style.Colors[ImGuiCol_ChildBg] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_PopupBg] = BackgroundChildColor;
	Style.Colors[ImGuiCol_Border] = BorderColor;
	Style.Colors[ImGuiCol_BorderShadow] = TransparentColor;
	Style.Colors[ImGuiCol_FrameBg] = WithAlpha(HeaderColor, 0.85f);
	Style.Colors[ImGuiCol_FrameBgHovered] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_FrameBgActive] = Accent2AlternativeColor;
	Style.Colors[ImGuiCol_TitleBg] = TitleColor;
	Style.Colors[ImGuiCol_TitleBgActive] = TitleTransparentColor;
	Style.Colors[ImGuiCol_TitleBgCollapsed] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_MenuBarBg] = BackgroundChildColor;
	Style.Colors[ImGuiCol_ScrollbarBg] = TitleTransparentColor;
	Style.Colors[ImGuiCol_ScrollbarGrab] = HeaderColor;
	Style.Colors[ImGuiCol_ScrollbarGrabHovered] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_ScrollbarGrabActive] = Accent2AlternativeColor;
	Style.Colors[ImGuiCol_CheckMark] = Accent1Color;
	Style.Colors[ImGuiCol_SliderGrab] = Accent1Color;
	Style.Colors[ImGuiCol_SliderGrabActive] = Accent2Color;
	Style.Colors[ImGuiCol_Button] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_ButtonHovered] = Accent1Color;
	Style.Colors[ImGuiCol_ButtonActive] = Accent2Color;
	Style.Colors[ImGuiCol_Header] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_HeaderHovered] = Accent1Color;
	Style.Colors[ImGuiCol_HeaderActive] = Accent2Color;
	Style.Colors[ImGuiCol_Separator] = BorderColor;
	Style.Colors[ImGuiCol_SeparatorHovered] = Accent1Color;
	Style.Colors[ImGuiCol_SeparatorActive] = Accent2Color;
	Style.Colors[ImGuiCol_ResizeGrip] = HeaderColor;
	Style.Colors[ImGuiCol_ResizeGripHovered] = Accent1Color;
	Style.Colors[ImGuiCol_ResizeGripActive] = Accent2Color;
	Style.Colors[ImGuiCol_InputTextCursor] = TextColor;
	Style.Colors[ImGuiCol_Tab] = TransparentColor;
	Style.Colors[ImGuiCol_TabHovered] = Accent1Color;
	Style.Colors[ImGuiCol_TabSelected] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_TabSelectedOverline] = Accent1Color;
	Style.Colors[ImGuiCol_TabDimmed] = TransparentColor;
	Style.Colors[ImGuiCol_TabDimmedSelected] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_TabDimmedSelectedOverline] = TransparentColor;
	Style.Colors[ImGuiCol_DockingPreview] = WithAlpha(Accent1Color, 0.55f);
	Style.Colors[ImGuiCol_DockingEmptyBg] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_PlotLines] = TextDimmedColor;
	Style.Colors[ImGuiCol_PlotLinesHovered] = Accent1Color;
	Style.Colors[ImGuiCol_PlotHistogram] = TextDimmedColor;
	Style.Colors[ImGuiCol_PlotHistogramHovered] = Accent1Color;
	Style.Colors[ImGuiCol_TableHeaderBg] = HeaderColor;
	Style.Colors[ImGuiCol_TableBorderStrong] = BorderColor;
	Style.Colors[ImGuiCol_TableBorderLight] = WithAlpha(BorderColor, 0.50f);
	Style.Colors[ImGuiCol_TableRowBg] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_TableRowBgAlt] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_TextLink] = Accent1Color;
	Style.Colors[ImGuiCol_TextSelectedBg] = WithAlpha(Accent1Color, 0.35f);
	Style.Colors[ImGuiCol_TreeLines] = TextDimmedColor;
	Style.Colors[ImGuiCol_DragDropTarget] = Accent1Color;
	Style.Colors[ImGuiCol_DragDropTargetBg] = TransparentColor;
	Style.Colors[ImGuiCol_UnsavedMarker] = Accent1Color;
	Style.Colors[ImGuiCol_NavCursor] = Accent1Color;
	Style.Colors[ImGuiCol_NavWindowingHighlight] = Accent1Color;
	Style.Colors[ImGuiCol_NavWindowingDimBg] = BackgroundDimmedColor;
	Style.Colors[ImGuiCol_ModalWindowDimBg] = BackgroundDimmedColor;
}

void ImGuiManager::ApplyDarkTheme() {
	ImGuiStyle& Style = ImGui::GetStyle();
	ImGui::StyleColorsDark();

	auto AdjustHueAndSaturation = [](const ImVec4& Color, const float HueShift, const float SaturationScale) {
		ImVec4 ResultColor = Color;
		ImGui::ColorConvertRGBtoHSV(ResultColor.x, ResultColor.y, ResultColor.z, ResultColor.x, ResultColor.y, ResultColor.z);

		ResultColor.x += HueShift;
		ResultColor.y *= SaturationScale;

		ImGui::ColorConvertHSVtoRGB(ResultColor.x, ResultColor.y, ResultColor.z, ResultColor.x, ResultColor.y, ResultColor.z);
		return ResultColor;
	};

	const float Hue = 0.0f;              // [0,1] range.
	const float Saturation = 1.0f;       // [0,6] range.
	const float SaturationAccent = 1.0f; // [0,6] range.
	const float Transparency = 1.0f;     // Unityは基本的に不透明なので1.0fに調整
	const float BorderSize = 1.0f;       // Unityの境界線用に1.0fに変更

	Style.FrameBorderSize = BorderSize;
	Style.ImageBorderSize = 0.0f;
	Style.TabBorderSize = BorderSize;
	Style.TabBarBorderSize = 1.0f;
	Style.WindowRounding = 0.0f;
	Style.ChildRounding = 0.0f;
	Style.FrameRounding = 1.0f;
	Style.GrabRounding = 1.0f;
	Style.TabRounding = 1.0f;

	Style.DockingSeparatorSize = 2.0f;

	// --- Unity 2020+ Dark Themeをイメージした配色定義 ---
	ImVec4 TextColor{0.850f, 0.850f, 0.850f, 1.000f};            // 若干オフホワイトな文字色
	ImVec4 TextDimmedColor{0.500f, 0.500f, 0.500f, 1.000f};      // 無効化テキスト（グレー）
	ImVec4 BackgroundColor{0.157f, 0.157f, 0.157f, 1.000f};      // メインウィンドウ背景（#282828）
	ImVec4 BackgroundChildColor{0.118f, 0.118f, 0.118f, 1.000f}; // ビュー、ゲーム画面等の背景（#1E1E1E）
	ImVec4 BackgroundDimmedColor{0.000f, 0.000f, 0.000f, 0.500f};
	ImVec4 TitleColor{0.118f, 0.118f, 0.118f, 1.000f};  // タブや境界線のダークグレー
	ImVec4 HeaderColor{0.220f, 0.220f, 0.220f, 1.000f}; // 通常のボタンやヘッダー（#383838）

	// アクセントカラー（Unityの選択ハイライト：ブルー）
	ImVec4 Accent1Color{0.173f, 0.365f, 0.533f, 1.000f};            // 選択中・ホバー（#2C5D88）
	ImVec4 Accent2Color{0.149f, 0.314f, 0.459f, 1.000f};            // クリック時（#265075）
	ImVec4 Accent1AlternativeColor{0.263f, 0.263f, 0.263f, 1.000f}; // ボタン等のホバー（#434343）
	ImVec4 Accent2AlternativeColor{0.188f, 0.188f, 0.188f, 1.000f}; // ボタン等のアクティブ（#303030）
	ImVec4 TransparentColor{0.0f, 0.0f, 0.0f, 0.0f};

	TextColor = AdjustHueAndSaturation(TextColor, Hue, Saturation);
	TextDimmedColor = AdjustHueAndSaturation(TextDimmedColor, Hue, Saturation);
	BackgroundColor = AdjustHueAndSaturation(BackgroundColor, Hue, Saturation);
	BackgroundChildColor = AdjustHueAndSaturation(BackgroundChildColor, Hue, Saturation);
	BackgroundDimmedColor = AdjustHueAndSaturation(BackgroundDimmedColor, Hue, Saturation);
	TitleColor = AdjustHueAndSaturation(TitleColor, Hue, Saturation);
	HeaderColor = AdjustHueAndSaturation(HeaderColor, Hue, Saturation);
	Accent1Color = AdjustHueAndSaturation(Accent1Color, Hue, SaturationAccent);
	Accent2Color = AdjustHueAndSaturation(Accent2Color, Hue, SaturationAccent);
	Accent1AlternativeColor = AdjustHueAndSaturation(Accent1AlternativeColor, Hue, SaturationAccent);
	Accent2AlternativeColor = AdjustHueAndSaturation(Accent2AlternativeColor, Hue, SaturationAccent);

	ImVec4 BackgroundTransparentColor = BackgroundColor;
	BackgroundTransparentColor.w *= Transparency;

	ImVec4 BackgroundChildTransparentColor = BackgroundChildColor;
	BackgroundChildTransparentColor.w *= Transparency;

	ImVec4 TitleTransparentColor = TitleColor;
	TitleTransparentColor.w *= Transparency;

	ImVec4 HeaderTransparentColor = HeaderColor;
	HeaderTransparentColor.w *= Transparency;

	Style.Colors[ImGuiCol_Text] = TextColor;
	Style.Colors[ImGuiCol_TextDisabled] = TextDimmedColor;
	Style.Colors[ImGuiCol_WindowBg] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_ChildBg] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_PopupBg] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_Border] = TitleColor;
	Style.Colors[ImGuiCol_BorderShadow] = TransparentColor;
	Style.Colors[ImGuiCol_FrameBg] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_FrameBgHovered] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_FrameBgActive] = Accent2AlternativeColor;
	Style.Colors[ImGuiCol_TitleBg] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_TitleBgActive] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_TitleBgCollapsed] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_MenuBarBg] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_ScrollbarBg] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_ScrollbarGrab] = HeaderColor;
	Style.Colors[ImGuiCol_ScrollbarGrabHovered] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_ScrollbarGrabActive] = Accent2AlternativeColor;
	Style.Colors[ImGuiCol_CheckMark] = Accent1Color;
	Style.Colors[ImGuiCol_SliderGrab] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_SliderGrabActive] = Accent1Color;
	Style.Colors[ImGuiCol_Button] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_ButtonHovered] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_ButtonActive] = Accent2AlternativeColor;
	Style.Colors[ImGuiCol_Header] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_HeaderHovered] = Accent1Color;
	Style.Colors[ImGuiCol_HeaderActive] = Accent2Color;
	Style.Colors[ImGuiCol_Separator] = TitleColor;
	Style.Colors[ImGuiCol_SeparatorHovered] = Accent1Color;
	Style.Colors[ImGuiCol_SeparatorActive] = Accent2Color;
	Style.Colors[ImGuiCol_ResizeGrip] = TransparentColor;
	Style.Colors[ImGuiCol_ResizeGripHovered] = Accent1Color;
	Style.Colors[ImGuiCol_ResizeGripActive] = Accent2Color;
	Style.Colors[ImGuiCol_InputTextCursor] = TextColor;
	Style.Colors[ImGuiCol_TabHovered] = Accent1AlternativeColor;
	Style.Colors[ImGuiCol_Tab] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_TabSelected] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_TabSelectedOverline] = TransparentColor;
	Style.Colors[ImGuiCol_TabDimmed] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_TabDimmedSelected] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_TabDimmedSelectedOverline] = TransparentColor;
	Style.Colors[ImGuiCol_DockingPreview] = Accent1Color;
	Style.Colors[ImGuiCol_DockingEmptyBg] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_PlotLines] = Accent1Color;
	Style.Colors[ImGuiCol_PlotLinesHovered] = Accent2Color;
	Style.Colors[ImGuiCol_PlotHistogram] = Accent1Color;
	Style.Colors[ImGuiCol_PlotHistogramHovered] = Accent2Color;
	Style.Colors[ImGuiCol_TableHeaderBg] = HeaderTransparentColor;
	Style.Colors[ImGuiCol_TableBorderStrong] = TitleColor;
	Style.Colors[ImGuiCol_TableBorderLight] = HeaderColor;
	Style.Colors[ImGuiCol_TableRowBg] = BackgroundTransparentColor;
	Style.Colors[ImGuiCol_TableRowBgAlt] = BackgroundChildTransparentColor;
	Style.Colors[ImGuiCol_TextLink] = Accent1Color;
	Style.Colors[ImGuiCol_TextSelectedBg] = Accent1Color;
	Style.Colors[ImGuiCol_TreeLines] = TextDimmedColor;
	Style.Colors[ImGuiCol_DragDropTarget] = Accent1Color;
	Style.Colors[ImGuiCol_DragDropTargetBg] = TransparentColor;
	Style.Colors[ImGuiCol_UnsavedMarker] = Accent1Color;
	Style.Colors[ImGuiCol_NavCursor] = Accent1Color;
	Style.Colors[ImGuiCol_NavWindowingHighlight] = Accent1Color;
	Style.Colors[ImGuiCol_NavWindowingDimBg] = BackgroundDimmedColor;
	Style.Colors[ImGuiCol_ModalWindowDimBg] = BackgroundDimmedColor;
}

} // namespace Cake

#endif // USE_IMGUI
