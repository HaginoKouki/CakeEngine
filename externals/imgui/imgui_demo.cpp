// dear imgui, v1.92.9 WIP
// (demo code)

// Help:
// - Read FAQ at http://dearimgui.com/faq
// - Call and read ImGui::ShowDemoWindow() in imgui_demo.cpp. All applications in examples/ are doing that.
// - Need help integrating Dear ImGui in your codebase?
//   - Read Getting Started https://github.com/ocornut/imgui/wiki/Getting-Started
//   - Read 'Programmer guide' in imgui.cpp for notes on how to setup Dear ImGui in your codebase.
// Read top of imgui.cpp and imgui.h for many details, documentation, comments, links.
// Get the latest version at https://github.com/ocornut/imgui

// How to easily locate code?
// - Use Tools->Item Picker to debug break in code by clicking any widgets: https://github.com/ocornut/imgui/wiki/Debug-Tools
// - Browse pthom's online imgui_explorer: web version the demo w/ source code browser: https://pthom.github.io/imgui_explorer
// - Find a visible string and search for it in the code!

//---------------------------------------------------
// PLEASE DO NOT REMOVE THIS FILE FROM YOUR PROJECT!
//---------------------------------------------------
// Message to the person tempted to delete this file when integrating Dear ImGui into their codebase:
// Think again! It is the most useful reference code that you and other coders will want to refer to and call.
// Have the ImGui::ShowDemoWindow() function wired in an always-available debug menu of your game/app!
// Also include Metrics! ItemPicker! DebugLog! and other debug features.
// Removing this file from your project is hindering access to documentation for everyone in your team,
// likely leading you to poorer usage of the library.
// Everything in this file will be stripped out by the linker if you don't call ImGui::ShowDemoWindow().
// If you want to link core Dear ImGui in your shipped builds but want a thorough guarantee that the demo will not be
// linked, you can setup your imconfig.h with #define IMGUI_DISABLE_DEMO_WINDOWS and those functions will be empty.
// In another situation, whenever you have Dear ImGui available you probably want this to be available for reference.
// Thank you,
// -Your beloved friend, imgui_demo.cpp (which you won't delete)

//--------------------------------------------
// ABOUT THE MEANING OF THE 'static' KEYWORD:
//--------------------------------------------
// In this demo code, we frequently use 'static' variables inside functions.
// A static variable persists across calls. It is essentially a global variable but declared inside the scope of the function.
// Think of "static int n = 0;" as "global int n = 0;" !
// We do this IN THE DEMO because we want:
// - to gather code and data in the same place.
// - to make the demo source code faster to read, faster to change, smaller in size.
// - it is also a convenient way of storing simple UI related information as long as your function
//   doesn't need to be reentrant or used in multiple threads.
// This might be a pattern you will want to use in your code, but most of the data you would be working
// with in a complex codebase is likely going to be stored outside your functions.

//-----------------------------------------
// ABOUT THE CODING STYLE OF OUR DEMO CODE
//-----------------------------------------
// The Demo code in this file is designed to be easy to copy-and-paste into your application!
// Because of this:
// - We never omit the ImGui:: prefix when calling functions, even though most code here is in the same namespace.
// - We try to declare static variables in the local scope, as close as possible to the code using them.
// - We never use any of the helpers/facilities used internally by Dear ImGui, unless available in the public API.
// - We never use maths operators on ImVec2/ImVec4. For our other sources files we use them, and they are provided
//   by imgui.h using the IMGUI_DEFINE_MATH_OPERATORS define. For your own sources file they are optional
//   and require you either enable those, either provide your own via IM_VEC2_CLASS_EXTRA in imconfig.h.
//   Because we can't assume anything about your support of maths operators, we cannot use them in imgui_demo.cpp.

// Navigating this file:
// - In Visual Studio: Ctrl+Comma ("Edit.GoToAll") can follow symbols inside comments, whereas Ctrl+F12 ("Edit.GoToImplementation") cannot.
// - In Visual Studio w/ Visual Assist installed: Alt+G ("VAssistX.GoToImplementation") can also follow symbols inside comments.
// - In VS Code, CLion, etc.: Ctrl+Click can follow symbols inside comments.
// - You can search/grep for all sections listed in the index to find the section.

/*

Index of this file:

// [SECTION] Forward Declarations
// [SECTION] Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// [SECTION] DemoWindowMenuBar()
// [SECTION] Helpers: ExampleTreeNode, ExampleMemberInfo (for use by Property Editor & Multi-Select demos)
// [SECTION] Helpers: ExampleImageViewer
// [SECTION] DemoWindowWidgetsBasic()
// [SECTION] DemoWindowWidgetsBullets()
// [SECTION] DemoWindowWidgetsCollapsingHeaders()
// [SECTION] DemoWindowWidgetsComboBoxes()
// [SECTION] DemoWindowWidgetsColorAndPickers()
// [SECTION] DemoWindowWidgetsDataTypes()
// [SECTION] DemoWindowWidgetsDisableBlocks()
// [SECTION] DemoWindowWidgetsDragAndDrop()
// [SECTION] DemoWindowWidgetsDragsAndSliders()
// [SECTION] DemoWindowWidgetsFonts()
// [SECTION] DemoWindowWidgetsImages()
// [SECTION] DemoWindowWidgetsListBoxes()
// [SECTION] DemoWindowWidgetsMultiComponents()
// [SECTION] DemoWindowWidgetsPlotting()
// [SECTION] DemoWindowWidgetsProgressBars()
// [SECTION] DemoWindowWidgetsQueryingStatuses()
// [SECTION] DemoWindowWidgetsSelectables()
// [SECTION] DemoWindowWidgetsSelectionAndMultiSelect()
// [SECTION] DemoWindowWidgetsTabs()
// [SECTION] DemoWindowWidgetsText()
// [SECTION] DemoWindowWidgetsTextFilter()
// [SECTION] DemoWindowWidgetsTextInput()
// [SECTION] DemoWindowWidgetsTooltips()
// [SECTION] DemoWindowWidgetsTreeNodes()
// [SECTION] DemoWindowWidgetsVerticalSliders()
// [SECTION] DemoWindowWidgets()
// [SECTION] DemoWindowLayout()
// [SECTION] DemoWindowPopups()
// [SECTION] DemoWindowTables()
// [SECTION] DemoWindowInputs()
// [SECTION] About Window / ShowAboutWindow()
// [SECTION] Style Editor / ShowStyleEditor()
// [SECTION] User Guide / ShowUserGuide()
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
// [SECTION] Example App: Image Viewer / ShowExampleAppImageViewer()
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
// [SECTION] Example App: Manipulating window titles / ShowExampleAppWindowTitles()
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
// [SECTION] Example App: Assets Browser / ShowExampleAppAssetsBrowser()

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE

// System includes
#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#include <stdint.h>         // intptr_t
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h>       // PRId64/PRIu64, not avail in some MinGW headers.
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten/version.h>     // __EMSCRIPTEN_MAJOR__ etc.
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to an 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                     // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                           // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"        // warning: 'xx' is deprecated: The POSIX name for this..   // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type
#pragma clang diagnostic ignored "-Wformat"                         // warning: format specifies type 'int' but the argument has type 'unsigned int'
#pragma clang diagnostic ignored "-Wformat-security"                // warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor    // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"                  // warning: macro is not used                               // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                   // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"            // warning: 'xxx' is an unsafe pointer used for buffer access
#pragma clang diagnostic ignored "-Wswitch-default"                 // warning: 'switch' missing 'default' label
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wfloat-equal"                      // warning: comparing floating-point with '==' or '!=' is unsafe
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"              // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat"                           // warning: format '%p' expects argument of type 'int'/'void*', but argument X has type 'unsigned int'/'ImGuiWindow*'
#pragma GCC diagnostic ignored "-Wformat-security"                  // warning: format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"                 // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                       // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"           // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#pragma GCC diagnostic ignored "-Wstrict-overflow"                  // warning: assuming signed overflow does not occur when simplifying division / ..when changing X +- C1 cmp C2 to X cmp C2 -+ C1
#pragma GCC diagnostic ignored "-Wcast-qual"                        // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

// Play it nice with Windows users (Update: May 2018, Notepad now supports Unix-style carriage returns!)
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

// Helpers
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf    _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf   _vsnprintf
#endif

// Format specifiers for 64-bit values (hasn't been decently standardized before VS2013)
#if !defined(PRId64) && defined(_MSC_VER)
#define PRId64 "I64d"
#define PRIu64 "I64u"
#elif !defined(PRId64)
#define PRId64 "lld"
#define PRIu64 "llu"
#endif

// Helpers macros
// We normally try to not use many helpers in imgui_demo.cpp in order to make code easier to copy and paste,
// but making an exception here as those are largely simplifying code...
// In other imgui sources we can use nicer internal functions from imgui_internal.h (ImMin/ImMax) but not in the demo.
#define IM_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define IM_MAX(A, B)            (((A) >= (B)) ? (A) : (B))
#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// Enforce cdecl calling convention for functions called by the standard library,
// in case compilation settings changed the default to e.g. __vectorcall
#ifndef IMGUI_CDECL
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

// Forward Declarations
struct ImGuiDemoWindowData;
static void ShowExampleAppMainMenuBar();
static void ShowExampleAppAssetsBrowser(bool* p_open);
static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleAppDockSpace(bool* p_open);
static void ShowExampleAppDocuments(bool* p_open);
static void ShowExampleAppImageViewer(bool* p_open);
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open, ImGuiDemoWindowData* demo_data);
static void ShowExampleAppSimpleOverlay(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppFullscreen(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleMenuFile();

// We split the contents of the big ShowDemoWindow() function into smaller functions
// (because the link time of very large functions tends to grow non-linearly)
static void DemoWindowMenuBar(ImGuiDemoWindowData* demo_data);
static void DemoWindowWidgets(ImGuiDemoWindowData* demo_data);
static void DemoWindowLayout();
static void DemoWindowPopups();
static void DemoWindowTables();
static void DemoWindowColumns();
static void DemoWindowInputs();

// Helper tree functions used by Property Editor & Multi-Select demos
struct ExampleTreeNode;
static ExampleTreeNode* ExampleTree_CreateNode(const char* name, int uid, ExampleTreeNode* parent);
static void             ExampleTree_DestroyNode(ExampleTreeNode* node);

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void ShowDockingDisabledMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("エラー: ドッキングが有効になっていません! デモの「設定」を参照してください。");
    ImGui::Text("コード内で io.ConfigFlags |= ImGuiConfigFlags_DockingEnable を設定するか、");
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::SmallButton("ここをクリック"))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

// Helper to wire demo markers located in code to an interactive browser (e.g. https://pthom.github.io/imgui_explorer)
#if IMGUI_VERSION_NUM >= 19263
namespace ImGui { extern IMGUI_API void DemoMarker(const char* file, int line, const char* section); }
#define IMGUI_DEMO_MARKER(section)  do { ImGui::DemoMarker("imgui_demo.cpp", __LINE__, section); } while (0)
#endif

// Sneakily forward declare functions which aren't worth putting in public API yet
namespace ImGui
{
    IMGUI_API void ShowFontAtlas(ImFontAtlas* atlas);
    IMGUI_API void TreeNodeSetOpen(ImGuiID storage_id, bool is_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------

// Data to be shared across different functions of the demo.
struct ImGuiDemoWindowData
{
    // Examples Apps (accessible from the "Examples" menu)
    bool ShowMainMenuBar = false;
    bool ShowAppAssetsBrowser = false;
    bool ShowAppConsole = false;
    bool ShowAppCustomRendering = false;
    bool ShowAppDocuments = false;
    bool ShowAppDockSpace = false;
    bool ShowAppImageViewer = false;
    bool ShowAppLog = false;
    bool ShowAppLayout = false;
    bool ShowAppPropertyEditor = false;
    bool ShowAppSimpleOverlay = false;
    bool ShowAppAutoResize = false;
    bool ShowAppConstrainedResize = false;
    bool ShowAppFullscreen = false;
    bool ShowAppLongText = false;
    bool ShowAppWindowTitles = false;

    // Dear ImGui Tools (accessible from the "Tools" menu)
    bool ShowMetrics = false;
    bool ShowDebugLog = false;
    bool ShowIDStackTool = false;
    bool ShowStyleEditor = false;
    bool ShowAbout = false;

    // Other data
    bool DisableSections = false;
    ExampleTreeNode* DemoTree = NULL;

    ~ImGuiDemoWindowData() { if (DemoTree) ExampleTree_DestroyNode(DemoTree); }
};

// Demonstrate most Dear ImGui features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does.
// You may then search for keywords in the code when you are interested by a specific feature.
void ImGui::ShowDemoWindow(bool* p_open)
{
    // Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
    // Most functions would normally just assert/crash if the context is missing.
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing Dear ImGui context. Refer to examples app!");

    // Verify ABI compatibility between caller code and compiled version of Dear ImGui. This helps detects some build issues.
    IMGUI_CHECKVERSION();

    // Stored data
    static ImGuiDemoWindowData demo_data;

    // Examples Apps (accessible from the "Examples" menu)
    if (demo_data.ShowMainMenuBar)          { ShowExampleAppMainMenuBar(); }
    if (demo_data.ShowAppDockSpace)         { ShowExampleAppDockSpace(&demo_data.ShowAppDockSpace); } // Important: Process the Docking app first, as explicit DockSpace() nodes needs to be submitted early (read comments near the DockSpace function)
    if (demo_data.ShowAppDocuments)         { ShowExampleAppDocuments(&demo_data.ShowAppDocuments); } // ...process the Document app next, as it may also use a DockSpace()
    if (demo_data.ShowAppAssetsBrowser)     { ShowExampleAppAssetsBrowser(&demo_data.ShowAppAssetsBrowser); }
    if (demo_data.ShowAppConsole)           { ShowExampleAppConsole(&demo_data.ShowAppConsole); }
    if (demo_data.ShowAppCustomRendering)   { ShowExampleAppCustomRendering(&demo_data.ShowAppCustomRendering); }
    if (demo_data.ShowAppImageViewer)       { ShowExampleAppImageViewer(&demo_data.ShowAppImageViewer); }
    if (demo_data.ShowAppLog)               { ShowExampleAppLog(&demo_data.ShowAppLog); }
    if (demo_data.ShowAppLayout)            { ShowExampleAppLayout(&demo_data.ShowAppLayout); }
    if (demo_data.ShowAppPropertyEditor)    { ShowExampleAppPropertyEditor(&demo_data.ShowAppPropertyEditor, &demo_data); }
    if (demo_data.ShowAppSimpleOverlay)     { ShowExampleAppSimpleOverlay(&demo_data.ShowAppSimpleOverlay); }
    if (demo_data.ShowAppAutoResize)        { ShowExampleAppAutoResize(&demo_data.ShowAppAutoResize); }
    if (demo_data.ShowAppConstrainedResize) { ShowExampleAppConstrainedResize(&demo_data.ShowAppConstrainedResize); }
    if (demo_data.ShowAppFullscreen)        { ShowExampleAppFullscreen(&demo_data.ShowAppFullscreen); }
    if (demo_data.ShowAppLongText)          { ShowExampleAppLongText(&demo_data.ShowAppLongText); }
    if (demo_data.ShowAppWindowTitles)      { ShowExampleAppWindowTitles(&demo_data.ShowAppWindowTitles); }

    // Dear ImGui Tools (accessible from the "Tools" menu)
    if (demo_data.ShowMetrics)              { ImGui::ShowMetricsWindow(&demo_data.ShowMetrics); }
    if (demo_data.ShowDebugLog)             { ImGui::ShowDebugLogWindow(&demo_data.ShowDebugLog); }
    if (demo_data.ShowIDStackTool)          { ImGui::ShowIDStackToolWindow(&demo_data.ShowIDStackTool); }
    if (demo_data.ShowAbout)                { ImGui::ShowAboutWindow(&demo_data.ShowAbout); }
    if (demo_data.ShowStyleEditor)
    {
        ImGui::Begin("Dear ImGui スタイルエディター", &demo_data.ShowStyleEditor);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }

    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;
    static bool no_docking = false;
    static bool unsaved_document = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Dear ImGui デモ", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Most framed widgets share a common width settings. Remaining width is used for the label.
    // The width of the frame may be changed with PushItemWidth() or SetNextItemWidth().
    // - Positive value for absolute size, negative value for right-alignment.
    // - The default value is about GetWindowWidth() * 0.65f.
    // - See 'Demo->Layout->Widgets Width' for details.
    // Here we change the frame width based on how much width we want to give to the label.
    const float label_width_base = ImGui::GetFontSize() * 12;               // Some amount of width for label, based on font size.
    const float label_width_max = ImGui::GetContentRegionAvail().x * 0.40f; // ...but always leave some room for framed widgets.
    const float label_width = IM_MIN(label_width_base, label_width_max);
    ImGui::PushItemWidth(-label_width);                                     // Right-align: framed items will leave 'label_width' available for the label.
    //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.40f);       // e.g. Use 40% width for framed widgets, leaving 60% width for labels.
    //ImGui::PushItemWidth(-ImGui::GetContentRegionAvail().x * 0.40f);      // e.g. Use 40% width for labels, leaving 60% width for framed widgets.
    //ImGui::PushItemWidth(ImGui::GetFontSize() * -12);                     // e.g. Use XXX width for labels, leaving the rest for framed widgets.

    // Menu Bar
    DemoWindowMenuBar(&demo_data);

    ImGui::Text("dear imgui からこんにちは! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("ヘルプ"))
    {
        IMGUI_DEMO_MARKER("Help");
        ImGui::SeparatorText("このデモについて:");
        ImGui::BulletText("以下のセクションでは、このライブラリの様々な機能を紹介しています。");
        ImGui::BulletText("上の「サンプル」メニューから、さらに多くのデモを見ることができます。");
        ImGui::BulletText("上の「ツール」メニューからは、バージョン情報・スタイルエディター・\n"
                          "メトリクス/デバッガー (Dear ImGui 汎用デバッグツール) を開けます。");
        ImGui::BulletText("Web デモ (ソースコードブラウザー付き): ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("https://pthom.github.io/imgui_explorer");

        ImGui::SeparatorText("プログラマー向けガイド:");
        ImGui::BulletText("imgui_demo.cpp の ShowDemoWindow() のコードを見てください。<- 今ここです!");
        ImGui::BulletText("imgui.cpp のコメントを読んでください。");
        ImGui::BulletText("examples/ フォルダーのサンプルアプリケーションを見てください。");
        ImGui::BulletText("FAQ はこちら: ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("https://www.dearimgui.com/faq/");
        ImGui::BulletText("キーボードで操作するには 'io.ConfigFlags |= NavEnableKeyboard' を設定します。");
        ImGui::BulletText("ゲームパッドで操作するには 'io.ConfigFlags |= NavEnableGamepad' を設定します。");

        ImGui::SeparatorText("ユーザー向けガイド:");
        ImGui::ShowUserGuide();
    }

    if (ImGui::CollapsingHeader("設定"))
    {
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::TreeNode("設定##2"))
        {
            IMGUI_DEMO_MARKER("Configuration");
            ImGui::SeparatorText("全般");
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
            ImGui::SameLine(); HelpMarker("キーボード操作を有効にします。");
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad",     &io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
            ImGui::SameLine(); HelpMarker("ゲームパッド操作を有効にします。バックエンドが io.BackendFlags |= ImGuiBackendFlags_HasGamepad を設定している必要があります。\n\n詳細は imgui.cpp の説明を読んでください。");
            ImGui::CheckboxFlags("io.ConfigFlags: NoMouse",              &io.ConfigFlags, ImGuiConfigFlags_NoMouse);
            ImGui::SameLine(); HelpMarker("dear imgui のマウス入力と操作を無効にします。");

            // The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to fix it:
            if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
            {
                if (fmodf((float)ImGui::GetTime(), 0.40f) < 0.20f)
                {
                    ImGui::SameLine();
                    ImGui::Text("<<スペースキーで解除>>");
                }
                // Prevent both being checked
                if (ImGui::IsKeyPressed(ImGuiKey_Space) || (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard))
                    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }

            ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange",  &io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
            ImGui::SameLine(); HelpMarker("マウスカーソルの形状と表示をバックエンド側で変更しないようにします。");
            ImGui::CheckboxFlags("io.ConfigFlags: NoKeyboard", &io.ConfigFlags, ImGuiConfigFlags_NoKeyboard);
            ImGui::SameLine(); HelpMarker("dear imgui のキーボード入力と操作を無効にします。");

            ImGui::Checkbox("io.ConfigInputTrickleEventQueue", &io.ConfigInputTrickleEventQueue);
            ImGui::SameLine(); HelpMarker("入力キューのトリクル処理を有効にします。同じフレーム内で送られた一部のイベント (例: ボタンの押下と解放) を複数フレームに分散させ、低フレームレートでの操作性を改善します。");
            ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
            ImGui::SameLine(); HelpMarker("マウスカーソルを Dear ImGui 自身で描画します。アプリの GPU 描画経路で描かれたカーソルはハードウェアカーソルより遅れて感じられますが、他の描画内容とはよく同期します。\n\nデスクトップアプリでは両方を使い分けることもあります (例: リサイズやドラッグ中だけソフトウェアカーソルを使う)。");

            ImGui::SeparatorText("キーボード/ゲームパッドナビゲーション");
            ImGui::Checkbox("io.ConfigNavSwapGamepadButtons", &io.ConfigNavSwapGamepadButtons);
            ImGui::Checkbox("io.ConfigNavMoveSetMousePos", &io.ConfigNavMoveSetMousePos);
            ImGui::SameLine(); HelpMarker("方向キーやタブによる移動時にマウスカーソルをワープさせます。仮想マウスの操作が難しいテレビやコンソール機で便利です。");
            ImGui::Checkbox("io.ConfigNavCaptureKeyboard", &io.ConfigNavCaptureKeyboard);
            ImGui::Checkbox("io.ConfigNavEscapeClearFocusItem", &io.ConfigNavEscapeClearFocusItem);
            ImGui::SameLine(); HelpMarker("Escape キーで項目のフォーカスを解除します。");
            ImGui::Checkbox("io.ConfigNavEscapeClearFocusWindow", &io.ConfigNavEscapeClearFocusWindow);
            ImGui::SameLine(); HelpMarker("Escape キーでウィンドウのフォーカスを解除します。");
            ImGui::Checkbox("io.ConfigNavCursorVisibleAuto", &io.ConfigNavCursorVisibleAuto);
            ImGui::SameLine(); HelpMarker("方向キーでの移動時にナビゲーションカーソルを表示し、マウスクリックで隠します。");
            ImGui::Checkbox("io.ConfigNavCursorVisibleAlways", &io.ConfigNavCursorVisibleAlways);
            ImGui::SameLine(); HelpMarker("ナビゲーションカーソルを常に表示します。");

            ImGui::SeparatorText("ドッキング");
            ImGui::CheckboxFlags("io.ConfigFlags: DockingEnable", &io.ConfigFlags, ImGuiConfigFlags_DockingEnable);
            ImGui::SameLine();
            if (io.ConfigDockingWithShift)
                HelpMarker("ウィンドウのタイトルバーやタブからドラッグしてドッキング/解除します。SHIFT を押している間だけドッキングが有効になります。\n\nウィンドウメニューボタン (左上のボタン) からドラッグすると、ノード全体 (すべてのウィンドウ) を切り離せます。");
            else
                HelpMarker("ウィンドウのタイトルバーやタブからドラッグしてドッキング/解除します。SHIFT を押している間はドッキングが無効になります。\n\nウィンドウメニューボタン (左上のボタン) からドラッグすると、ノード全体 (すべてのウィンドウ) を切り離せます。");
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGui::Indent();
                ImGui::Checkbox("io.ConfigDockingNoSplit", &io.ConfigDockingNoSplit);
                ImGui::SameLine(); HelpMarker("簡易ドッキングモード: ウィンドウの分割を無効にし、複数ウィンドウをタブバーにまとめる操作だけに限定します。");
                ImGui::Checkbox("io.ConfigDockingNoDockingOver", &io.ConfigDockingNoDockingOver);
                ImGui::SameLine(); HelpMarker("簡易ドッキングモード: 同じタブバーへの統合を無効にし、ウィンドウの分割だけに限定します。");
                ImGui::Checkbox("io.ConfigDockingWithShift", &io.ConfigDockingWithShift);
                ImGui::SameLine(); HelpMarker("Shift を押している間だけドッキングを有効にします (広い範囲にドロップでき、見た目のノイズも減ります)");
                ImGui::Checkbox("io.ConfigDockingAlwaysTabBar", &io.ConfigDockingAlwaysTabBar);
                ImGui::SameLine(); HelpMarker("単独のフローティングウィンドウにもドッキングノードとタブバーを作ります。");
                ImGui::Checkbox("io.ConfigDockingTransparentPayload", &io.ConfigDockingTransparentPayload);
                ImGui::SameLine(); HelpMarker("ドッキング中はウィンドウやビューポートを透明にし、ドッキング枠だけをターゲットのビューポートに表示します。複数ビューポートの描画を同期できない場合に便利です。ConfigViewportsNoAutoMerge と併用するとよいでしょう。");
                ImGui::Unindent();
            }

            ImGui::SeparatorText("マルチビューポート");
            ImGui::CheckboxFlags("io.ConfigFlags: ViewportsEnable", &io.ConfigFlags, ImGuiConfigFlags_ViewportsEnable);
            ImGui::SameLine(); HelpMarker("[beta] ベータ版のマルチビューポート対応を有効にします。詳細は ImGuiPlatformIO を参照してください。");
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::Indent();
                ImGui::Checkbox("io.ConfigViewportsNoAutoMerge", &io.ConfigViewportsNoAutoMerge);
                ImGui::SameLine(); HelpMarker("有効にすると、すべてのフローティングウィンドウが常に専用のビューポートを作ります。無効の場合は、重なっているメインのホストビューポートに統合されます。");
                ImGui::Checkbox("io.ConfigViewportsNoTaskBarIcon", &io.ConfigViewportsNoTaskBarIcon);
                ImGui::SameLine(); HelpMarker("(注意: プラットフォームバックエンドによっては、既存のビューポートにこの変更が反映されず、ビューポートの作り直しが必要な場合があります)");
                ImGui::Checkbox("io.ConfigViewportsNoDecoration", &io.ConfigViewportsNoDecoration);
                ImGui::SameLine(); HelpMarker("(注意: プラットフォームバックエンドによっては、既存のビューポートにこの変更が反映されず、ビューポートの作り直しが必要な場合があります)");
                ImGui::Checkbox("io.ConfigViewportsNoDefaultParent", &io.ConfigViewportsNoDefaultParent);
                ImGui::SameLine(); HelpMarker("(注意: プラットフォームバックエンドによっては、既存のビューポートにこの変更が反映されず、ビューポートの作り直しが必要な場合があります)");
                ImGui::Checkbox("io.ConfigViewportsPlatformFocusSetsImGuiFocus", &io.ConfigViewportsPlatformFocusSetsImGuiFocus);
                ImGui::SameLine(); HelpMarker("プラットフォームウィンドウがフォーカスされたとき (Alt+Tab やプラットフォームのタイトルバーのクリックなど)、対応する imgui ウィンドウにもフォーカスを適用します (他のプラットフォームウィンドウにある imgui ウィンドウのフォーカスやアクティブ ID が解除されることがあります)。基本的には有効が望ましいですが、ウィンドウマネージャーによっては過剰にフォーカスを移すもの (マウスホバーや位置/サイズ変更だけで反応するもの) があるため、無効にもできます。");
                ImGui::Unindent();
            }

            //ImGui::SeparatorText("DPI/Scaling");
            //ImGui::Checkbox("io.ConfigDpiScaleFonts", &io.ConfigDpiScaleFonts);
            //ImGui::SameLine(); HelpMarker("Experimental: Automatically update style.FontScaleDpi when Monitor DPI changes. This will scale fonts but NOT style sizes/padding for now.");
            //ImGui::Checkbox("io.ConfigDpiScaleViewports", &io.ConfigDpiScaleViewports);
            //ImGui::SameLine(); HelpMarker("Experimental: Scale Dear ImGui and Platform Windows when Monitor DPI changes.");

            ImGui::SeparatorText("ウィンドウ");
            ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
            ImGui::SameLine(); HelpMarker("ウィンドウの端と左下隅からのリサイズを有効にします。\nマウスカーソルの表示を適切にするには ImGuiBackendFlags_HasMouseCursors が必要です。");
            ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
            ImGui::Checkbox("io.ConfigWindowsCopyContentsWithCtrlC", &io.ConfigWindowsCopyContentsWithCtrlC); // [EXPERIMENTAL]
            ImGui::SameLine(); HelpMarker("*実験的機能* Ctrl+C でフォーカス中のウィンドウの内容をクリップボードにコピーします。\n\n実験的な理由:\n- (1) Begin/End の入れ子で既知の問題があります。\n- (2) テキスト出力の品質が一定ではありません。\n- (3) テキストは見た目の並びではなく送信順に出力されます。");
            ImGui::Checkbox("io.ConfigScrollbarScrollByPage", &io.ConfigScrollbarScrollByPage);
            ImGui::SameLine(); HelpMarker("スクロールバーのつまみの外側をクリックしたときにページ単位でスクロールします。\n無効の場合は常にクリック位置までスクロールします。\n有効の場合は Shift+クリックでクリック位置までスクロールします。");

            ImGui::SeparatorText("ウィジェット");
            ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
            ImGui::SameLine(); HelpMarker("カーソルの点滅を有効にします (気が散ると感じる人もいるため任意です)。");
            ImGui::Checkbox("io.ConfigInputTextEnterKeepActive", &io.ConfigInputTextEnterKeepActive);
            ImGui::SameLine(); HelpMarker("Enter キーで項目を再度アクティブにし、テキストを全選択します (単一行のみ)。");
            ImGui::Checkbox("io.ConfigDragClickToInputText", &io.ConfigDragClickToInputText);
            ImGui::SameLine(); HelpMarker("マウスのクリックとリリースだけで (動かさずに) DragXXX 系ウィジェットをテキスト入力に切り替えられるようにします。");
            ImGui::Checkbox("io.ConfigMacOSXBehaviors", &io.ConfigMacOSXBehaviors);
            ImGui::SameLine(); HelpMarker("Cmd キーと Ctrl キーを入れ替え、macOS 風の各種挙動を有効にします。");
            ImGui::Text("描画に関するオプションは Style->Rendering も参照してください。");

            ImGui::SeparatorText("設定の保存");
            ImGui::Checkbox("io.ConfigIniSettingsSaveLastUsedDate", &io.ConfigIniSettingsSaveLastUsedDate);

            // Also read: https://github.com/ocornut/imgui/wiki/Error-Handling
            ImGui::SeparatorText("エラー処理");

            ImGui::Checkbox("io.ConfigErrorRecovery", &io.ConfigErrorRecovery);
            ImGui::SameLine(); HelpMarker(
                "回復可能なエラーの扱いを設定するオプションです。\n"
                "- エラー回復は完全でも保証されたものでもありません! 開発を楽にするための機能です。\n"
                "- 通常のアプリケーション実行中にこれを当てにしてはいけません。\n"
                "- 想定される用途: スクリプト言語や特定の例外ハンドラーから発生したエラーからの回復を容易にすること。\n"
                "- imgui の API を直接呼ぶ場合、開発時には最低限アサートかツールチップを有効にしておいてください。"
                "そうしないと、間違いを見つけて直すのが非常に難しくなります!");
            ImGui::Checkbox("io.ConfigErrorRecoveryEnableAssert", &io.ConfigErrorRecoveryEnableAssert);
            ImGui::Checkbox("io.ConfigErrorRecoveryEnableDebugLog", &io.ConfigErrorRecoveryEnableDebugLog);
            ImGui::Checkbox("io.ConfigErrorRecoveryEnableTooltip", &io.ConfigErrorRecoveryEnableTooltip);
            if (!io.ConfigErrorRecoveryEnableAssert && !io.ConfigErrorRecoveryEnableDebugLog && !io.ConfigErrorRecoveryEnableTooltip)
                io.ConfigErrorRecoveryEnableAssert = io.ConfigErrorRecoveryEnableDebugLog = io.ConfigErrorRecoveryEnableTooltip = true;

            // Also read: https://github.com/ocornut/imgui/wiki/Debug-Tools
            ImGui::SeparatorText("デバッグ");
            ImGui::Checkbox("io.ConfigDebugIsDebuggerPresent", &io.ConfigDebugIsDebuggerPresent);
            ImGui::SameLine(); HelpMarker("IM_DEBUG_BREAK() を呼ぶ各種ツールを有効にします。\n\nデバッガーが接続されている必要があります。接続されていないと IM_DEBUG_BREAK() のオプションはアプリのクラッシュのように見えます。");
            ImGui::Checkbox("io.ConfigDebugHighlightIdConflicts", &io.ConfigDebugHighlightIdConflicts);
            ImGui::SameLine(); HelpMarker("複数の項目の識別子が衝突している場合に、ハイライトしてエラーメッセージを表示します。");
            ImGui::BeginDisabled();
            ImGui::Checkbox("io.ConfigDebugBeginReturnValueOnce", &io.ConfigDebugBeginReturnValueOnce);
            ImGui::EndDisabled();
            ImGui::SameLine(); HelpMarker("Begin()/BeginChild() の最初の呼び出しが false を返すようになります。\n\nこのオプションは無効化されています。意味を持たせるにはアプリ起動時に設定する必要があるためです。無効の状態で表示しているのは、この機能を見つけやすくするためです。");
            ImGui::Checkbox("io.ConfigDebugBeginReturnValueLoop", &io.ConfigDebugBeginReturnValueLoop);
            ImGui::SameLine(); HelpMarker("Begin()/BeginChild() の一部の呼び出しが false を返すようになります。\n\nウィンドウの深さを順に巡回して繰り返します。実行中はウィンドウがちらつくはずです。");
            ImGui::Checkbox("io.ConfigDebugIgnoreFocusLoss", &io.ConfigDebugIgnoreFocusLoss);
            ImGui::SameLine(); HelpMarker("io.AddFocusEvent(false) の処理を無効にするオプションです。フォーカスを失うと入力データが消える挙動がデバッガーの操作を妨げる場合に役立ちます。");
            ImGui::Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
            ImGui::SameLine(); HelpMarker(".ini データに追加のコメントを付けて保存するオプションです (特にドッキングで便利ですが、保存が遅くなります)。");

            ImGui::TreePop();
            ImGui::Spacing();
        }

        if (ImGui::TreeNode("バックエンドフラグ"))
        {
            IMGUI_DEMO_MARKER("Configuration/Backend Flags");
            HelpMarker(
                "これらのフラグは、バックエンド (imgui_impl_xxx ファイル) が自身の対応機能を示すために設定します。\n"
                "バックエンドとのやり取りを壊さないよう、ここでは読み取り専用の項目として表示しています。");

            // Make a local copy to avoid modifying actual backend flags.
            // FIXME: Maybe we need a BeginReadonly() equivalent to keep label bright?
            ImGui::BeginDisabled();
            ImGui::CheckboxFlags("io.BackendFlags: HasGamepad",             &io.BackendFlags, ImGuiBackendFlags_HasGamepad);
            ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors",        &io.BackendFlags, ImGuiBackendFlags_HasMouseCursors);
            ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos",         &io.BackendFlags, ImGuiBackendFlags_HasSetMousePos);
            ImGui::CheckboxFlags("io.BackendFlags: PlatformHasViewports",   &io.BackendFlags, ImGuiBackendFlags_PlatformHasViewports);
            ImGui::CheckboxFlags("io.BackendFlags: HasMouseHoveredViewport",&io.BackendFlags, ImGuiBackendFlags_HasMouseHoveredViewport);
            ImGui::CheckboxFlags("io.BackendFlags: HasParentViewport",      &io.BackendFlags, ImGuiBackendFlags_HasParentViewport);
            ImGui::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset",   &io.BackendFlags, ImGuiBackendFlags_RendererHasVtxOffset);
            ImGui::CheckboxFlags("io.BackendFlags: RendererHasTextures",    &io.BackendFlags, ImGuiBackendFlags_RendererHasTextures);
            ImGui::CheckboxFlags("io.BackendFlags: RendererHasViewports",   &io.BackendFlags, ImGuiBackendFlags_RendererHasViewports);
            ImGui::EndDisabled();

            ImGui::TreePop();
            ImGui::Spacing();
        }

        if (ImGui::TreeNode("スタイル・フォント"))
        {
            IMGUI_DEMO_MARKER("Configuration/Style, Fonts");
            ImGui::Checkbox("スタイルエディター", &demo_data.ShowStyleEditor);
            ImGui::SameLine();
            HelpMarker("同じ内容は「ツール->スタイルエディター」からも、ShowStyleEditor() 関数を呼び出しても利用できます。");
            ImGui::TreePop();
            ImGui::Spacing();
        }

        if (ImGui::TreeNode("キャプチャ/ログ出力"))
        {
            IMGUI_DEMO_MARKER("Configuration/Capture, Logging");
            HelpMarker(
                "ログ API はすべてのテキスト出力をリダイレクトするため、ウィンドウやブロックの内容を簡単に取り込めます。"
                "ツリーノードを自動的に展開することもできます。\n"
                "下にある項目をこのウィンドウ内で開いてから、「Log To」ボタンのいずれかを押してみてください。");
            ImGui::LogButtons();

            HelpMarker("ImGui::LogText() を呼べば、画面に表示せずにログへ直接出力することもできます。");
            if (ImGui::Button("「Hello, world!」をクリップボードにコピー"))
            {
                ImGui::LogToClipboard();
                ImGui::LogText("Hello, world!");
                ImGui::LogFinish();
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("ウィンドウオプション"))
    {
        IMGUI_DEMO_MARKER("Window options");
        if (ImGui::BeginTable("split", 3))
        {
            ImGui::TableNextColumn(); ImGui::Checkbox("タイトルバーなし", &no_titlebar);
            ImGui::TableNextColumn(); ImGui::Checkbox("スクロールバーなし", &no_scrollbar);
            ImGui::TableNextColumn(); ImGui::Checkbox("メニューなし", &no_menu);
            ImGui::TableNextColumn(); ImGui::Checkbox("移動不可", &no_move);
            ImGui::TableNextColumn(); ImGui::Checkbox("リサイズ不可", &no_resize);
            ImGui::TableNextColumn(); ImGui::Checkbox("折りたたみ不可", &no_collapse);
            ImGui::TableNextColumn(); ImGui::Checkbox("閉じるボタンなし", &no_close);
            ImGui::TableNextColumn(); ImGui::Checkbox("ナビゲーションなし", &no_nav);
            ImGui::TableNextColumn(); ImGui::Checkbox("背景なし", &no_background);
            ImGui::TableNextColumn(); ImGui::Checkbox("最前面に出さない", &no_bring_to_front);
            ImGui::TableNextColumn(); ImGui::Checkbox("ドッキング不可", &no_docking);
            ImGui::TableNextColumn(); ImGui::Checkbox("未保存のドキュメント", &unsaved_document);
            ImGui::EndTable();
        }
    }

    // All demo contents
    DemoWindowWidgets(&demo_data);
    DemoWindowLayout();
    DemoWindowPopups();
    DemoWindowTables();
    DemoWindowInputs();

    // End of ShowDemoWindow()
    ImGui::PopItemWidth();
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowMenuBar()
//-----------------------------------------------------------------------------

static void DemoWindowMenuBar(ImGuiDemoWindowData* demo_data)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("メニュー"))
        {
            IMGUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("サンプル"))
        {
            IMGUI_DEMO_MARKER("Menu/Examples");
            ImGui::MenuItem("メインメニューバー", NULL, &demo_data->ShowMainMenuBar);

            ImGui::SeparatorText("ミニアプリ");
            ImGui::MenuItem("アセットブラウザー", NULL, &demo_data->ShowAppAssetsBrowser);
            ImGui::MenuItem("コンソール", NULL, &demo_data->ShowAppConsole);
            ImGui::MenuItem("カスタム描画", NULL, &demo_data->ShowAppCustomRendering);
            ImGui::MenuItem("ドキュメント", NULL, &demo_data->ShowAppDocuments);
            ImGui::MenuItem("ドックスペース", NULL, &demo_data->ShowAppDockSpace);
            ImGui::MenuItem("画像ビューアー", NULL, &demo_data->ShowAppImageViewer);
            ImGui::MenuItem("ログ", NULL, &demo_data->ShowAppLog);
            ImGui::MenuItem("プロパティエディター", NULL, &demo_data->ShowAppPropertyEditor);
            ImGui::MenuItem("シンプルなレイアウト", NULL, &demo_data->ShowAppLayout);
            ImGui::MenuItem("シンプルなオーバーレイ", NULL, &demo_data->ShowAppSimpleOverlay);

            ImGui::SeparatorText("各種テクニック");
            ImGui::MenuItem("自動リサイズウィンドウ", NULL, &demo_data->ShowAppAutoResize);
            ImGui::MenuItem("リサイズ制限付きウィンドウ", NULL, &demo_data->ShowAppConstrainedResize);
            ImGui::MenuItem("全画面ウィンドウ", NULL, &demo_data->ShowAppFullscreen);
            ImGui::MenuItem("長いテキストの表示", NULL, &demo_data->ShowAppLongText);
            ImGui::MenuItem("ウィンドウタイトルの操作", NULL, &demo_data->ShowAppWindowTitles);

            ImGui::EndMenu();
        }
        //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("ツール"))
        {
            IMGUI_DEMO_MARKER("Menu/Tools");
            ImGuiIO& io = ImGui::GetIO();
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
            const bool has_debug_tools = true;
#else
            const bool has_debug_tools = false;
#endif
            ImGui::MenuItem("メトリクス/デバッガー", NULL, &demo_data->ShowMetrics, has_debug_tools);
            if (ImGui::BeginMenu("デバッグオプション"))
            {
                ImGui::BeginDisabled(!has_debug_tools);
                ImGui::Checkbox("ID の衝突を強調表示", &io.ConfigDebugHighlightIdConflicts);
                ImGui::EndDisabled();
                ImGui::Checkbox("エラー回復時にアサート", &io.ConfigErrorRecoveryEnableAssert);
                ImGui::TextDisabled("(詳細はデモ->設定を参照)");
                ImGui::EndMenu();
            }
            ImGui::MenuItem("デバッグログ", NULL, &demo_data->ShowDebugLog, has_debug_tools);
            ImGui::MenuItem("ID スタックツール", NULL, &demo_data->ShowIDStackTool, has_debug_tools);
            bool is_debugger_present = io.ConfigDebugIsDebuggerPresent;
            if (ImGui::MenuItem("アイテムピッカー", NULL, false, has_debug_tools))// && is_debugger_present))
                ImGui::DebugStartItemPicker();
            if (!is_debugger_present)
                ImGui::SetItemTooltip("io.ConfigDebugIsDebuggerPresent=true が設定されている必要があります。\n\nそうでない場合、うっかりアプリをクラッシュさせないよう一部の追加機能を無効にしています。");
            ImGui::MenuItem("スタイルエディター", NULL, &demo_data->ShowStyleEditor);
            ImGui::MenuItem("Dear ImGui について", NULL, &demo_data->ShowAbout);

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers: ExampleTreeNode, ExampleMemberInfo (for use by Property Editor & Multi-Select demos)
//-----------------------------------------------------------------------------

// Simple representation for a tree
// (this is designed to be simple to understand for our demos, not to be fancy or efficient etc.)
struct ExampleTreeNode
{
    // Tree structure
    char                        Name[28] = "";
    int                         UID = 0;
    ExampleTreeNode*            Parent = NULL;
    ImVector<ExampleTreeNode*>  Childs;
    int                         IndexInParent = 0;  // Maintaining this allows us to implement linear traversal more easily

    // Leaf Data
    bool                        HasData = false;    // All leaves have data
    bool                        DataMyBool = true;
    int                         DataMyInt = 128;
    ImVec2                      DataMyVec2 = ImVec2(0.0f, 3.141592f);
};

// Simple representation of struct metadata/serialization data.
// (this is a minimal version of what a typical advanced application may provide)
struct ExampleMemberInfo
{
    const char*     Name;       // Member name
    ImGuiDataType   DataType;   // Member type
    int             DataCount;  // Member count (1 when scalar)
    int             Offset;     // Offset inside parent structure
};

// Metadata description of ExampleTreeNode struct.
static const ExampleMemberInfo ExampleTreeNodeMemberInfos[]
{
    { "MyName",     ImGuiDataType_String,  1, offsetof(ExampleTreeNode, Name) },
    { "MyBool",     ImGuiDataType_Bool,    1, offsetof(ExampleTreeNode, DataMyBool) },
    { "MyInt",      ImGuiDataType_S32,     1, offsetof(ExampleTreeNode, DataMyInt) },
    { "MyVec2",     ImGuiDataType_Float,   2, offsetof(ExampleTreeNode, DataMyVec2) },
};

static ExampleTreeNode* ExampleTree_CreateNode(const char* name, int uid, ExampleTreeNode* parent)
{
    ExampleTreeNode* node = IM_NEW(ExampleTreeNode);
    snprintf(node->Name, IM_COUNTOF(node->Name), "%s", name);
    node->UID = uid;
    node->Parent = parent;
    node->IndexInParent = parent ? parent->Childs.Size : 0;
    if (parent)
        parent->Childs.push_back(node);
    return node;
}

static void ExampleTree_DestroyNode(ExampleTreeNode* node)
{
    for (ExampleTreeNode* child_node : node->Childs)
        ExampleTree_DestroyNode(child_node);
    IM_DELETE(node);
}

// Create example tree data
// (warning: this can allocates MANY MANY more times than other code in all of Dear ImGui + demo combined)
// (a real application managing one million nodes would likely store its tree data differently)
static ExampleTreeNode* ExampleTree_CreateDemoTree()
{
    //     20 root nodes ->    211 total nodes,   ~261 allocs.
    //   1000 root nodes ->   ~11K total nodes,   ~14K allocs.
    //  10000 root nodes ->  ~123K total nodes,  ~154K allocs.
    // 100000 root nodes -> ~1338K total nodes, ~1666K allocs.
    const int ROOT_ITEMS_COUNT = 20;

    static const char* category_names[] = { "リンゴ", "バナナ", "サクランボ", "キウイ", "マンゴー", "オレンジ", "洋ナシ", "パイナップル", "イチゴ", "スイカ" };
    const int category_count = IM_COUNTOF(category_names);
    const size_t NAME_MAX_LEN = sizeof(ExampleTreeNode::Name);
    char name_buf[NAME_MAX_LEN];
    int uid = 0;
    ExampleTreeNode* node_L0 = ExampleTree_CreateNode("<ROOT>", ++uid, NULL);
    for (int idx_L0 = 0; idx_L0 < ROOT_ITEMS_COUNT; idx_L0++)
    {
        snprintf(name_buf, IM_COUNTOF(name_buf), "%s %d", category_names[idx_L0 / (ROOT_ITEMS_COUNT / category_count)], idx_L0 % (ROOT_ITEMS_COUNT / category_count));
        ExampleTreeNode* node_L1 = ExampleTree_CreateNode(name_buf, ++uid, node_L0);
        const int number_of_childs = (int)strlen(node_L1->Name);
        for (int idx_L1 = 0; idx_L1 < number_of_childs; idx_L1++)
        {
            snprintf(name_buf, IM_COUNTOF(name_buf), "子 %d", idx_L1);
            ExampleTreeNode* node_L2 = ExampleTree_CreateNode(name_buf, ++uid, node_L1);
            node_L2->HasData = true;
            if (idx_L1 == 0)
            {
                snprintf(name_buf, IM_COUNTOF(name_buf), "孫 %d", 0);
                ExampleTreeNode* node_L3 = ExampleTree_CreateNode(name_buf, ++uid, node_L2);
                node_L3->HasData = true;
            }
        }
    }
    return node_L0;
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers: ExampleImageViewer
//-----------------------------------------------------------------------------

struct ExampleImageViewerData
{
    ImU32   ImageBgColor = IM_COL32(100, 100, 100, 255);
    ImU32   GridColor = IM_COL32(255, 255, 255, 100);
    bool    GridEnabled = true;
    bool    ViewReset = true;
    ImVec2  ViewOffset; // in image space
    float   Zoom = 10.0f;
    float   ZoomMin = 1.0f;
    float   ZoomMax = 10000.0f;
};

static void ExampleImageViewer_DrawOptions(ExampleImageViewerData* data)
{
    ImGui::SetNextItemShortcut(ImGuiKey_G, ImGuiInputFlags_Tooltip); // | ImGuiInputFlags_RouteGlobal
    ImGui::Checkbox("グリッド", &data->GridEnabled);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 10.0f);
    float zoom_100 = data->Zoom * 100.0f;
    if (ImGui::DragFloat("##Zoom", &zoom_100, 5.0f, data->ZoomMin * 100.0f, data->ZoomMax * 100.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp))
        data->Zoom = zoom_100 / 100.0f;
}

static void ExampleImageViewer_DrawCanvas(ExampleImageViewerData* data, ImVec2 canvas_size, ImTextureRef image_tex_ref, int image_w, int image_h)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    IM_ASSERT(canvas_size.x >= 0.0f && canvas_size.y >= 0.0f);

    // Layout canvas
    ImGui::InvisibleButton("##Canvas", canvas_size);
    ImVec2 canvas_min = ImGui::GetItemRectMin();
    ImVec2 canvas_max = ImGui::GetItemRectMax();

    if (data->ViewReset)
        data->ViewOffset = ImVec2((canvas_size.x * 0.5f / data->Zoom) - 0.5f, (canvas_size.y * 0.5f / data->Zoom) - 0.5f); // Add half a pixel padding
    data->ViewReset = false;

    // Handle inputs
    if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY))
        if (io.MouseWheel != 0.0f)
            data->Zoom = IM_CLAMP(data->Zoom * (1.0f + io.MouseWheel * 0.10f), data->ZoomMin, data->ZoomMax);
    float zoom = data->Zoom; // (float)(int)ViewZoom;
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
    {
        data->ViewOffset.x -= io.MouseDelta.x / zoom;
        data->ViewOffset.y -= io.MouseDelta.y / zoom;
    }

    // Display image
    ImVec2 image_min, image_max;
    image_min.x = (float)(int)((canvas_min.x - (data->ViewOffset.x * zoom)) + (canvas_size.x * 0.5f));
    image_min.y = (float)(int)((canvas_min.y - (data->ViewOffset.y * zoom)) + (canvas_size.y * 0.5f));
    image_max.x = (float)(int)(image_min.x + image_w * zoom);
    image_max.y = (float)(int)(image_min.y + image_h * zoom);
    draw_list->AddRect(ImVec2(canvas_min.x - 1.0f, canvas_min.y - 1.0f), ImVec2(canvas_max.x + 1.0f, canvas_max.y + 1.0f), IM_COL32(255, 255, 255, 255));
    draw_list->PushClipRect(canvas_min, canvas_max, true);
    draw_list->AddRectFilled(image_min, image_max, data->ImageBgColor);
    if (platform_io.DrawCallback_SetSamplerNearest != NULL)
        draw_list->AddCallback(platform_io.DrawCallback_SetSamplerNearest);
    draw_list->AddImage(image_tex_ref, image_min, image_max);
    if (platform_io.DrawCallback_SetSamplerLinear != NULL)
        draw_list->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerLinear);

    // Display grid lines for visible pixels
    if (data->GridEnabled && zoom > 6.0f)
    {
        const float step = (float)zoom;
        for (int px = (int)((canvas_min.x - image_min.x) / step); px <= (int)((canvas_max.x - image_min.x) / step); px++)
            draw_list->AddLineV(image_min.x + px * step, canvas_min.y, canvas_max.y, data->GridColor, 1.0f);
        for (int py = (int)((canvas_min.y - image_min.y) / step); py <= (int)((canvas_max.y - image_min.y) / step); py++)
            draw_list->AddLineH(canvas_min.x, canvas_max.x, image_min.y + py * step, data->GridColor, 1.0f);
    }
    draw_list->PopClipRect();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsBasic()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsBasic()
{
    if (ImGui::TreeNode("基本"))
    {
        IMGUI_DEMO_MARKER("Widgets/Basic");
        ImGui::SeparatorText("全般");

        IMGUI_DEMO_MARKER("Widgets/Basic/Button");
        static int clicked = 0;
        if (ImGui::Button("ボタン"))
            clicked++;
        if (clicked & 1)
        {
            ImGui::SameLine();
            ImGui::Text("クリックしてくれてありがとう!");
        }

        IMGUI_DEMO_MARKER("Widgets/Basic/Checkbox");
        static bool check = true;
        ImGui::Checkbox("チェックボックス", &check);

        IMGUI_DEMO_MARKER("Widgets/Basic/RadioButton");
        static int e = 0;
        ImGui::RadioButton("ラジオ a", &e, 0); ImGui::SameLine();
        ImGui::RadioButton("ラジオ b", &e, 1); ImGui::SameLine();
        ImGui::RadioButton("ラジオ c", &e, 2);

        ImGui::AlignTextToFramePadding();
        ImGui::TextLinkOpenURL("ハイパーリンク", "https://github.com/ocornut/imgui/wiki/Error-Handling");

        // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
        IMGUI_DEMO_MARKER("Widgets/Basic/Buttons (Colored)");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
                ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
            ImGui::Button("クリック");
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }

        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
        // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
        // See 'Demo->Layout->Text Baseline Alignment' for details.
        ImGui::AlignTextToFramePadding();
        ImGui::Text("押し続けて連続入力:");
        ImGui::SameLine();

        // Arrow buttons with Repeater
        IMGUI_DEMO_MARKER("Widgets/Basic/Buttons (Repeating)");
        static int counter = 0;
        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; }
        ImGui::SameLine(0.0f, spacing);
        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++; }
        ImGui::PopItemFlag();
        ImGui::SameLine();
        ImGui::Text("%d", counter);

        ImGui::Button("ツールチップ");
        ImGui::SetItemTooltip("これはツールチップです");

        ImGui::LabelText("ラベル", "値");

        ImGui::SeparatorText("入力");

        {
            // If you want to use InputText() with std::string or any custom dynamic string type:
            // - For std::string: use the wrapper in misc/cpp/imgui_stdlib.h/.cpp
            // - Otherwise, see the 'Dear ImGui Demo->Widgets->Text Input->Resize Callback' for using ImGuiInputTextFlags_CallbackResize.
            IMGUI_DEMO_MARKER("Widgets/Basic/InputText");
            static char str0[128] = "こんにちは、世界!";
            ImGui::InputText("テキスト入力", str0, IM_COUNTOF(str0));
            ImGui::SameLine(); HelpMarker(
                "ユーザー向け:\n"
                "Shift を押しながら、またはマウスでテキストを選択できます。\n"
                "Ctrl+左/右 で単語単位の移動。\n"
                "Ctrl+A またはダブルクリックで全選択。\n"
                "Ctrl+X, Ctrl+C, Ctrl+V でクリップボード操作。\n"
                "Ctrl+Z で元に戻す、Ctrl+Y / Ctrl+Shift+Z でやり直し。\n"
                "Escape で変更を取り消し。\n\n"
                "プログラマー向け:\n"
                "InputText() を可変長の文字列型と連携させたい場合は ImGuiInputTextFlags_CallbackResize の仕組みを使えます。"
                "例は misc/cpp/imgui_stdlib.h を参照してください (imgui_demo.cpp では"
                "紹介していません)。");

            static char str1[128] = "";
            ImGui::InputTextWithHint("テキスト入力 (ヒント付き)", "ここに入力してください", str1, IM_COUNTOF(str1));

            IMGUI_DEMO_MARKER("Widgets/Basic/InputInt, InputFloat");
            static int i0 = 123;
            ImGui::InputInt("整数入力", &i0);

            static float f0 = 0.001f;
            ImGui::InputFloat("浮動小数入力", &f0, 0.01f, 1.0f, "%.3f");

            static double d0 = 999999.00000001;
            ImGui::InputDouble("倍精度入力", &d0, 0.01f, 1.0f, "%.8f");

            static float f1 = 1.e10f;
            ImGui::InputFloat("指数表記入力", &f1, 0.0f, 0.0f, "%e");
            ImGui::SameLine(); HelpMarker(
                "指数表記で値を入力できます。\n"
                "  例: \"1e+8\" は \"100000000\" になります。");

            static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            ImGui::InputFloat3("float3 入力", vec4a);
        }

        ImGui::SeparatorText("ドラッグ");

        {
            IMGUI_DEMO_MARKER("Widgets/Basic/DragInt, DragFloat");
            static int i1 = 50, i2 = 42, i3 = 128;
            ImGui::DragInt("整数ドラッグ", &i1, 1);
            ImGui::SameLine(); HelpMarker(
                "クリックしてドラッグすると値を変更できます。\n"
                "Shift/Alt を押している間は変化が速く/遅くなります。\n"
                "ダブルクリックまたは Ctrl+クリックで値を直接入力できます。");
            ImGui::DragInt("整数ドラッグ 0..100", &i2, 1, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragInt("整数ドラッグ 折り返し 100..200", &i3, 1, 100, 200, "%d", ImGuiSliderFlags_WrapAround);

            static float f1 = 1.00f, f2 = 0.0067f;
            ImGui::DragFloat("浮動小数ドラッグ", &f1, 0.005f);
            ImGui::DragFloat("小さな値のドラッグ", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
            //ImGui::DragFloat("drag wrap -1..1", &f3, 0.005f, -1.0f, 1.0f, NULL, ImGuiSliderFlags_WrapAround);
        }

        ImGui::SeparatorText("スライダー");

        {
            IMGUI_DEMO_MARKER("Widgets/Basic/SliderInt, SliderFloat");
            static int i1 = 0;
            ImGui::SliderInt("整数スライダー", &i1, -1, 3);
            ImGui::SameLine(); HelpMarker("Ctrl+クリックで値を直接入力できます。");

            static float f1 = 0.123f, f2 = 0.0f;
            ImGui::SliderFloat("浮動小数スライダー", &f1, 0.0f, 1.0f, "比率 = %.3f");
            ImGui::SliderFloat("浮動小数スライダー (対数)", &f2, -10.0f, 10.0f, "%.4f", ImGuiSliderFlags_Logarithmic);

            IMGUI_DEMO_MARKER("Widgets/Basic/SliderAngle");
            static float angle = 0.0f;
            ImGui::SliderAngle("角度スライダー", &angle);

            // Using the format string to display a name instead of an integer.
            // Here we completely omit '%d' from the format string, so it'll only display a name.
            // This technique can also be used with DragInt().
            IMGUI_DEMO_MARKER("Widgets/Basic/Slider (enum)");
            enum Element { Element_Fire, Element_Earth, Element_Air, Element_Water, Element_COUNT };
            static int elem = Element_Fire;
            const char* elems_names[Element_COUNT] = { "火", "地", "風", "水" };
            const char* elem_name = (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "不明";
            ImGui::SliderInt("スライダー (列挙型)", &elem, 0, Element_COUNT - 1, elem_name); // Use ImGuiSliderFlags_NoInput flag to disable Ctrl+Click here.
            ImGui::SameLine(); HelpMarker("書式文字列のパラメーターを使って、内部の整数値の代わりに名前を表示しています。");
        }

        ImGui::SeparatorText("選択・ピッカー");

        {
            IMGUI_DEMO_MARKER("Widgets/Basic/ColorEdit3, ColorEdit4");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            ImGui::ColorEdit3("色 1", col1);
            ImGui::SameLine(); HelpMarker(
                "色の四角をクリックするとカラーピッカーが開きます。\n"
                "クリックしたまま動かすとドラッグ&ドロップできます。\n"
                "色の四角を右クリックするとオプションが表示されます。\n"
                "各成分を Ctrl+クリックすると値を直接入力できます。\n");

            ImGui::ColorEdit4("色 2", col2);
        }

        {
            // Using the _simplified_ one-liner Combo() api here
            // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo() api.
            IMGUI_DEMO_MARKER("Widgets/Basic/Combo");
            const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
            static int item_current = 0;
            ImGui::Combo("コンボ", &item_current, items, IM_COUNTOF(items));
            ImGui::SameLine(); HelpMarker(
                "ここでは1行で書ける簡易版の Combo API を使っています。\n"
                "より柔軟で汎用的な BeginCombo/EndCombo API の使い方は、下の「コンボボックス」セクションを参照してください。");
        }

        {
            // Using the _simplified_ one-liner ListBox() api here
            // See "List boxes" section for examples of how to use the more flexible BeginListBox()/EndListBox() api.
            IMGUI_DEMO_MARKER("Widgets/Basic/ListBox");
            const char* items[] = { "リンゴ", "バナナ", "サクランボ", "キウイ", "マンゴー", "オレンジ", "パイナップル", "イチゴ", "スイカ" };
            static int item_current = 1;
            ImGui::ListBox("リストボックス", &item_current, items, IM_COUNTOF(items), 4);
            ImGui::SameLine(); HelpMarker(
                "ここでは1行で書ける簡易版の ListBox API を使っています。\n"
                "より柔軟で汎用的な BeginListBox/EndListBox API の使い方は、下の「リストボックス」セクションを参照してください。");
        }

        // Testing ImGuiOnceUponAFrame helper.
        //static ImGuiOnceUponAFrame once;
        //for (int i = 0; i < 5; i++)
        //    if (once)
        //        ImGui::Text("This will be displayed only once.");

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsBullets()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsBullets()
{
    if (ImGui::TreeNode("箇条書き"))
    {
        IMGUI_DEMO_MARKER("Widgets/Bullets");
        ImGui::BulletText("箇条書き 1");
        ImGui::BulletText("箇条書き 2\n複数行にわたります");
        if (ImGui::TreeNode("ツリーノード"))
        {
            ImGui::BulletText("別の箇条書き");
            ImGui::TreePop();
        }
        ImGui::Bullet(); ImGui::Text("箇条書き 3 (2回の呼び出し)");
        ImGui::Bullet(); ImGui::SmallButton("ボタン");
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsCollapsingHeaders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsCollapsingHeaders()
{
    if (ImGui::TreeNode("折りたたみヘッダー"))
    {
        IMGUI_DEMO_MARKER("Widgets/Collapsing Headers");
        static bool closable_group = true;
        ImGui::Checkbox("2つ目のヘッダーを表示", &closable_group);
        if (ImGui::CollapsingHeader("ヘッダー", ImGuiTreeNodeFlags_None))
        {
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            for (int i = 0; i < 5; i++)
                ImGui::Text("コンテンツ %d", i);
        }
        if (ImGui::CollapsingHeader("閉じるボタン付きヘッダー", &closable_group))
        {
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            for (int i = 0; i < 5; i++)
                ImGui::Text("さらにコンテンツ %d", i);
        }
        /*
        if (ImGui::CollapsingHeader("Header with a bullet", ImGuiTreeNodeFlags_Bullet))
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
        */
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsColorAndPickers()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsColorAndPickers()
{
    if (ImGui::TreeNode("色/ピッカーウィジェット"))
    {
        IMGUI_DEMO_MARKER("Widgets/Color");
        static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
        static ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;

        ImGui::SeparatorText("オプション");
        ImGui::CheckboxFlags("ImGuiColorEditFlags_NoAlpha", &base_flags, ImGuiColorEditFlags_NoAlpha);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_AlphaOpaque", &base_flags, ImGuiColorEditFlags_AlphaOpaque);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_AlphaNoBg", &base_flags, ImGuiColorEditFlags_AlphaNoBg);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_AlphaPreviewHalf", &base_flags, ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_NoOptions", &base_flags, ImGuiColorEditFlags_NoOptions); ImGui::SameLine(); HelpMarker("個々のカラーウィジェットを右クリックするとオプションが表示されます。");
        ImGui::CheckboxFlags("ImGuiColorEditFlags_NoDragDrop", &base_flags, ImGuiColorEditFlags_NoDragDrop);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_NoColorMarkers", &base_flags, ImGuiColorEditFlags_NoColorMarkers);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_HDR", &base_flags, ImGuiColorEditFlags_HDR); ImGui::SameLine(); HelpMarker("現在のところ、ドラッグ系ウィジェットの 0..1 の制限を外すだけの効果しかありません。");

        IMGUI_DEMO_MARKER("Widgets/Color/ColorEdit");
        ImGui::SeparatorText("インラインカラーエディター");
        ImGui::Text("カラーウィジェット:");
        ImGui::SameLine(); HelpMarker(
            "色の四角をクリックするとカラーピッカーが開きます。\n"
            "各成分を Ctrl+クリックすると値を直接入力できます。\n");
        ImGui::ColorEdit3("マイカラー##1", (float*)&color, base_flags);

        IMGUI_DEMO_MARKER("Widgets/Color/ColorEdit (HSV, with Alpha)");
        ImGui::Text("HSV + アルファのカラーウィジェット:");
        ImGui::ColorEdit4("マイカラー##2", (float*)&color, ImGuiColorEditFlags_DisplayHSV | base_flags);

        IMGUI_DEMO_MARKER("Widgets/Color/ColorEdit (float display)");
        ImGui::Text("Float 表示のカラーウィジェット:");
        ImGui::ColorEdit4("マイカラー##2f", (float*)&color, ImGuiColorEditFlags_Float | base_flags);

        IMGUI_DEMO_MARKER("Widgets/Color/ColorButton (with Picker)");
        ImGui::Text("ピッカー付きカラーボタン:");
        ImGui::SameLine(); HelpMarker(
            "ImGuiColorEditFlags_NoInputs フラグを使うと、スライダーやテキスト入力をすべて隠せます。\n"
            "ImGuiColorEditFlags_NoLabel フラグを使うと、渡したラベルはツールチップと"
            "ピッカーのポップアップにだけ使われます。");
        ImGui::ColorEdit4("マイカラー##3", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | base_flags);

        IMGUI_DEMO_MARKER("Widgets/Color/ColorButton (with custom Picker popup)");
        ImGui::Text("カスタムピッカーポップアップ付きカラーボタン:");

        // Generate a default palette. The palette will persist and can be edited.
        static bool saved_palette_init = true;
        static ImVec4 saved_palette[32] = {};
        if (saved_palette_init)
        {
            for (int n = 0; n < IM_COUNTOF(saved_palette); n++)
            {
                ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
                    saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
                saved_palette[n].w = 1.0f; // Alpha
            }
            saved_palette_init = false;
        }

        static ImVec4 backup_color;
        bool open_popup = ImGui::ColorButton("マイカラー##3b", color, base_flags);
        ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
        open_popup |= ImGui::Button("パレット");
        if (open_popup)
        {
            ImGui::OpenPopup("mypicker");
            backup_color = color;
        }
        if (ImGui::BeginPopup("mypicker"))
        {
            ImGui::Text("すばらしいパレット付きのカスタムカラーピッカー!");
            ImGui::Separator();
            ImGui::ColorPicker4("##picker", (float*)&color, base_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            ImGui::SameLine();

            ImGui::BeginGroup(); // Lock X position
            ImGui::Text("現在の色");
            ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
            ImGui::Text("前の色");
            if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
                color = backup_color;
            ImGui::Separator();
            ImGui::Text("パレット");
            for (int n = 0; n < IM_COUNTOF(saved_palette); n++)
            {
                ImGui::PushID(n);
                if ((n % 8) != 0)
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

                ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
                if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
                    color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

                // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
                // drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                    ImGui::EndDragDropTarget();
                }

                ImGui::PopID();
            }
            ImGui::EndGroup();
            ImGui::EndPopup();
        }

        IMGUI_DEMO_MARKER("Widgets/Color/ColorButton (simple)");
        ImGui::Text("カラーボタンのみ:");
        static bool no_border = false;
        ImGui::Checkbox("ImGuiColorEditFlags_NoBorder", &no_border);
        ImGui::ColorButton("マイカラー##3c", *(ImVec4*)&color, base_flags | (no_border ? ImGuiColorEditFlags_NoBorder : 0), ImVec2(80, 80));

        IMGUI_DEMO_MARKER("Widgets/Color/ColorPicker");
        ImGui::SeparatorText("カラーピッカー");

        static bool ref_color = false;
        static ImVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
        static int picker_mode = 0;
        static int display_mode = 0;
        static ImGuiColorEditFlags color_picker_flags = ImGuiColorEditFlags_AlphaBar;

        ImGui::PushID("Color picker");
        ImGui::CheckboxFlags("ImGuiColorEditFlags_NoAlpha", &color_picker_flags, ImGuiColorEditFlags_NoAlpha);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_AlphaBar", &color_picker_flags, ImGuiColorEditFlags_AlphaBar);
        ImGui::CheckboxFlags("ImGuiColorEditFlags_NoSidePreview", &color_picker_flags, ImGuiColorEditFlags_NoSidePreview);
        if (color_picker_flags & ImGuiColorEditFlags_NoSidePreview)
        {
            ImGui::SameLine();
            ImGui::Checkbox("参照色を使う", &ref_color);
            if (ref_color)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##RefColor", &ref_color_v.x, ImGuiColorEditFlags_NoInputs | base_flags);
            }
        }

        ImGui::Combo("ピッカーモード", &picker_mode, "自動/現在の設定\0ImGuiColorEditFlags_PickerHueBar\0ImGuiColorEditFlags_PickerHueWheel\0");
        ImGui::SameLine(); HelpMarker("明示的に指定しない場合、ピッカーを右クリックしてモードを変更できます。");

        ImGui::Combo("表示モード", &display_mode, "自動/現在の設定\0ImGuiColorEditFlags_NoInputs\0ImGuiColorEditFlags_DisplayRGB\0ImGuiColorEditFlags_DisplayHSV\0ImGuiColorEditFlags_DisplayHex\0");
        ImGui::SameLine(); HelpMarker(
            "表示モードを指定しない場合、ColorEdit は既定で RGB 入力を表示しますが、"
            "入力欄を右クリックすればユーザーが変更できます。\n\n表示モードを指定しない場合、ColorPicker は既定で"
            "RGB+HSV+Hex を表示します。\n\n既定値は SetColorEditOptions() で変更できます。");

        ImGuiColorEditFlags flags = base_flags | color_picker_flags;
        if (picker_mode == 1)  flags |= ImGuiColorEditFlags_PickerHueBar;
        if (picker_mode == 2)  flags |= ImGuiColorEditFlags_PickerHueWheel;
        if (display_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;       // Disable all RGB/HSV/Hex displays
        if (display_mode == 2) flags |= ImGuiColorEditFlags_DisplayRGB;     // Override display mode
        if (display_mode == 3) flags |= ImGuiColorEditFlags_DisplayHSV;
        if (display_mode == 4) flags |= ImGuiColorEditFlags_DisplayHex;
        ImGui::ColorPicker4("マイカラー##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

        ImGui::Text("コードで既定値を設定:");
        ImGui::SameLine(); HelpMarker(
            "SetColorEditOptions() は起動時の既定値を設定するためのものです。\n"
            "Push/Pop 版がないのは、必要ならウィジェットごとにオプションを強制でき、"
            "強制していないものはユーザーがオプションメニューで変更できるためです。\nゲッターがないのは、"
            "将来のバージョンと互換性のない値を保存させないようにするためです。");
        if (ImGui::Button("既定値: Uint8 + HSV + 色相バー"))
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_PickerHueBar);
        if (ImGui::Button("既定値: Float + HDR + 色相ホイール"))
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);

        // Always display a small version of both types of pickers
        // (that's in order to make it more visible in the demo to people who are skimming quickly through it)
        ImGui::Text("両方のタイプ:");
        float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.y) * 0.40f;
        ImGui::SetNextItemWidth(w);
        ImGui::ColorPicker3("##MyColor##5", (float*)&color, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(w);
        ImGui::ColorPicker3("##MyColor##6", (float*)&color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha);
        ImGui::PopID();

        // HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
        static ImVec4 color_hsv(0.23f, 1.0f, 1.0f, 1.0f); // Stored as HSV!
        ImGui::Spacing();
        ImGui::Text("HSV で保持する色");
        ImGui::SameLine(); HelpMarker(
            "既定では ColorEdit と ColorPicker には RGB で色を渡しますが、ImGuiColorEditFlags_InputHSV を使うと"
            "色を HSV のまま保持して渡せます。こうすると彩度や明度が 0 のときでも"
            "ピッカーで色相を操作できるという利点があります。");
        ImGui::Text("InputHSV を使ったカラーウィジェット:");
        ImGui::ColorEdit4("HSV を RGB として表示##1", (float*)&color_hsv, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
        ImGui::ColorEdit4("HSV を HSV として表示##1", (float*)&color_hsv, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
        ImGui::DragFloat4("生の HSV 値", (float*)&color_hsv, 0.01f, 0.0f, 1.0f);

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsComboBoxes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsComboBoxes()
{
    if (ImGui::TreeNode("コンボボックス"))
    {
        IMGUI_DEMO_MARKER("Widgets/Combo");
        // Combo Boxes are also called "Dropdown" in other systems
        // Expose flags as checkbox for the demo
        static ImGuiComboFlags flags = 0;
        ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", &flags, ImGuiComboFlags_PopupAlignLeft);
        ImGui::SameLine(); HelpMarker("ポップアップがコンボより大きい場合にだけ違いが出ます");
        if (ImGui::CheckboxFlags("ImGuiComboFlags_NoArrowButton", &flags, ImGuiComboFlags_NoArrowButton))
            flags &= ~ImGuiComboFlags_NoPreview;     // Clear incompatible flags
        if (ImGui::CheckboxFlags("ImGuiComboFlags_NoPreview", &flags, ImGuiComboFlags_NoPreview))
            flags &= ~(ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_WidthFitPreview); // Clear incompatible flags
        if (ImGui::CheckboxFlags("ImGuiComboFlags_WidthFitPreview", &flags, ImGuiComboFlags_WidthFitPreview))
            flags &= ~ImGuiComboFlags_NoPreview;

        // Override default popup height
        if (ImGui::CheckboxFlags("ImGuiComboFlags_HeightSmall", &flags, ImGuiComboFlags_HeightSmall))
            flags &= ~(ImGuiComboFlags_HeightMask_ & ~ImGuiComboFlags_HeightSmall);
        if (ImGui::CheckboxFlags("ImGuiComboFlags_HeightRegular", &flags, ImGuiComboFlags_HeightRegular))
            flags &= ~(ImGuiComboFlags_HeightMask_ & ~ImGuiComboFlags_HeightRegular);
        if (ImGui::CheckboxFlags("ImGuiComboFlags_HeightLargest", &flags, ImGuiComboFlags_HeightLargest))
            flags &= ~(ImGuiComboFlags_HeightMask_ & ~ImGuiComboFlags_HeightLargest);

        // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
        // stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static int item_selected_idx = 0; // Here we store our selection data as an index.

        // Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
        const char* combo_preview_value = items[item_selected_idx];
        if (ImGui::BeginCombo("コンボ 1", combo_preview_value, flags))
        {
            for (int n = 0; n < IM_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Show case embedding a filter using a simple trick: displaying the filter inside combo contents.
        // See https://github.com/ocornut/imgui/issues/718 for advanced/esoteric alternatives.
        if (ImGui::BeginCombo("コンボ 2 (フィルター付き)", combo_preview_value, flags))
        {
            static ImGuiTextFilter filter;
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
                filter.Clear();
            }
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
            filter.Draw("##Filter", -FLT_MIN);

            for (int n = 0; n < IM_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (filter.PassFilter(items[n]))
                    if (ImGui::Selectable(items[n], is_selected))
                        item_selected_idx = n;
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::SeparatorText("1行で書ける簡易版");
        HelpMarker("Combo() 関数は、すべての選択肢を1つの文字列にまとめたい場合以外はあまり便利ではありません。\n上のフラグはこのセクションには適用されません。");

        // Simplified one-liner Combo() API, using values packed in a single constant string
        // This is a convenience for when the selection set is small and known at compile-time.
        static int item_current_2 = 0;
        ImGui::Combo("コンボ 3 (1行版)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

        // Simplified one-liner Combo() using an array of const char*
        // This is not very useful (may obsolete): prefer using BeginCombo()/EndCombo() for full control.
        static int item_current_3 = -1; // If the selection isn't within 0..count, Combo won't display a preview
        ImGui::Combo("コンボ 4 (配列)", &item_current_3, items, IM_COUNTOF(items));

        // Simplified one-liner Combo() using an accessor function
        static int item_current_4 = 0;
        ImGui::Combo("コンボ 5 (関数)", &item_current_4, [](void* data, int n) { return ((const char**)data)[n]; }, items, IM_COUNTOF(items));

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDataTypes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDataTypes()
{
    if (ImGui::TreeNode("データ型"))
    {
        IMGUI_DEMO_MARKER("Widgets/Data Types");
        // DragScalar/InputScalar/SliderScalar functions allow various data types
        // - signed/unsigned
        // - 8/16/32/64-bits
        // - integer/float/double
        // To avoid polluting the public API with all possible combinations, we use the ImGuiDataType enum
        // to pass the type, and passing all arguments by pointer.
        // This is the reason the test code below creates local variables to hold "zero" "one" etc. for each type.
        // In practice, if you frequently use a given type that is not covered by the normal API entry points,
        // you can wrap it yourself inside a 1 line function which can take typed argument as value instead of void*,
        // and then pass their address to the generic function. For example:
        //   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format = "%lld")
        //   {
        //      return SliderScalar(label, ImGuiDataType_U64, value, &min, &max, format);
        //   }

        // Setup limits (as helper variables so we can take their address, as explained above)
        // Note: SliderScalar() functions have a maximum usable range of half the natural type maximum, hence the /2.
        #ifndef LLONG_MIN
        ImS64 LLONG_MIN = -9223372036854775807LL - 1;
        ImS64 LLONG_MAX = 9223372036854775807LL;
        ImU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
        #endif
        const char    s8_zero  = 0,   s8_one  = 1,   s8_fifty  = 50, s8_min  = -128,        s8_max = 127;
        const ImU8    u8_zero  = 0,   u8_one  = 1,   u8_fifty  = 50, u8_min  = 0,           u8_max = 255;
        const short   s16_zero = 0,   s16_one = 1,   s16_fifty = 50, s16_min = -32768,      s16_max = 32767;
        const ImU16   u16_zero = 0,   u16_one = 1,   u16_fifty = 50, u16_min = 0,           u16_max = 65535;
        const ImS32   s32_zero = 0,   s32_one = 1,   s32_fifty = 50, s32_min = INT_MIN/2,   s32_max = INT_MAX/2,    s32_hi_a = INT_MAX/2 - 100,    s32_hi_b = INT_MAX/2;
        const ImU32   u32_zero = 0,   u32_one = 1,   u32_fifty = 50, u32_min = 0,           u32_max = UINT_MAX/2,   u32_hi_a = UINT_MAX/2 - 100,   u32_hi_b = UINT_MAX/2;
        const ImS64   s64_zero = 0,   s64_one = 1,   s64_fifty = 50, s64_min = LLONG_MIN/2, s64_max = LLONG_MAX/2,  s64_hi_a = LLONG_MAX/2 - 100,  s64_hi_b = LLONG_MAX/2;
        const ImU64   u64_zero = 0,   u64_one = 1,   u64_fifty = 50, u64_min = 0,           u64_max = ULLONG_MAX/2, u64_hi_a = ULLONG_MAX/2 - 100, u64_hi_b = ULLONG_MAX/2;
        const float   f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
        const double  f64_zero = 0.,  f64_one = 1.,  f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

        // State
        static char   s8_v  = 127;
        static ImU8   u8_v  = 255;
        static short  s16_v = 32767;
        static ImU16  u16_v = 65535;
        static ImS32  s32_v = -1;
        static ImU32  u32_v = (ImU32)-1;
        static ImS64  s64_v = -1;
        static ImU64  u64_v = (ImU64)-1;
        static float  f32_v = 0.123f;
        static double f64_v = 90000.01234567890123456789;

        const float drag_speed = 0.2f;
        static bool drag_clamp = false;
        IMGUI_DEMO_MARKER("Widgets/Data Types/Drags");
        ImGui::SeparatorText("ドラッグ");
        ImGui::Checkbox("整数を 0..50 に制限する", &drag_clamp);
        ImGui::SameLine(); HelpMarker(
            "dear imgui のすべてのウィジェットと同様に、ユーザーの操作がない限り値を書き換えることはありません。\n"
            "Ctrl+クリックで値を入力すれば、制限の範囲を超えて指定できます。");
        ImGui::DragScalar("ドラッグ s8",        ImGuiDataType_S8,     &s8_v,  drag_speed, drag_clamp ? &s8_zero  : NULL, drag_clamp ? &s8_fifty  : NULL);
        ImGui::DragScalar("ドラッグ u8",        ImGuiDataType_U8,     &u8_v,  drag_speed, drag_clamp ? &u8_zero  : NULL, drag_clamp ? &u8_fifty  : NULL, "%u ms");
        ImGui::DragScalar("ドラッグ s16",       ImGuiDataType_S16,    &s16_v, drag_speed, drag_clamp ? &s16_zero : NULL, drag_clamp ? &s16_fifty : NULL);
        ImGui::DragScalar("ドラッグ u16",       ImGuiDataType_U16,    &u16_v, drag_speed, drag_clamp ? &u16_zero : NULL, drag_clamp ? &u16_fifty : NULL, "%u ms");
        ImGui::DragScalar("ドラッグ s32",       ImGuiDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
        ImGui::DragScalar("ドラッグ s32 (16進)",   ImGuiDataType_S32,    &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL, "0x%08X");
        ImGui::DragScalar("ドラッグ u32",       ImGuiDataType_U32,    &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
        ImGui::DragScalar("ドラッグ s64",       ImGuiDataType_S64,    &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
        ImGui::DragScalar("ドラッグ u64",       ImGuiDataType_U64,    &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
        ImGui::DragScalar("ドラッグ float",     ImGuiDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f");
        ImGui::DragScalar("ドラッグ float (対数)", ImGuiDataType_Float,  &f32_v, 0.005f,  &f32_zero, &f32_one, "%f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragScalar("ドラッグ double",    ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL,     "%.10f グラム");
        ImGui::DragScalar("ドラッグ double (対数)",ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", ImGuiSliderFlags_Logarithmic);

        IMGUI_DEMO_MARKER("Widgets/Data Types/Sliders");
        ImGui::SeparatorText("スライダー");
        ImGui::SliderScalar("スライダー s8 全域",       ImGuiDataType_S8,     &s8_v,  &s8_min,   &s8_max,   "%d");
        ImGui::SliderScalar("スライダー u8 全域",       ImGuiDataType_U8,     &u8_v,  &u8_min,   &u8_max,   "%u");
        ImGui::SliderScalar("スライダー s16 全域",      ImGuiDataType_S16,    &s16_v, &s16_min,  &s16_max,  "%d");
        ImGui::SliderScalar("スライダー u16 全域",      ImGuiDataType_U16,    &u16_v, &u16_min,  &u16_max,  "%u");
        ImGui::SliderScalar("スライダー s32 低い範囲",       ImGuiDataType_S32,    &s32_v, &s32_zero, &s32_fifty,"%d");
        ImGui::SliderScalar("スライダー s32 高い範囲",      ImGuiDataType_S32,    &s32_v, &s32_hi_a, &s32_hi_b, "%d");
        ImGui::SliderScalar("スライダー s32 全域",      ImGuiDataType_S32,    &s32_v, &s32_min,  &s32_max,  "%d");
        ImGui::SliderScalar("スライダー s32 (16進)",       ImGuiDataType_S32,    &s32_v, &s32_zero, &s32_fifty, "0x%04X");
        ImGui::SliderScalar("スライダー u32 低い範囲",       ImGuiDataType_U32,    &u32_v, &u32_zero, &u32_fifty,"%u");
        ImGui::SliderScalar("スライダー u32 高い範囲",      ImGuiDataType_U32,    &u32_v, &u32_hi_a, &u32_hi_b, "%u");
        ImGui::SliderScalar("スライダー u32 全域",      ImGuiDataType_U32,    &u32_v, &u32_min,  &u32_max,  "%u");
        ImGui::SliderScalar("スライダー s64 低い範囲",       ImGuiDataType_S64,    &s64_v, &s64_zero, &s64_fifty,"%" PRId64);
        ImGui::SliderScalar("スライダー s64 高い範囲",      ImGuiDataType_S64,    &s64_v, &s64_hi_a, &s64_hi_b, "%" PRId64);
        ImGui::SliderScalar("スライダー s64 全域",      ImGuiDataType_S64,    &s64_v, &s64_min,  &s64_max,  "%" PRId64);
        ImGui::SliderScalar("スライダー u64 低い範囲",       ImGuiDataType_U64,    &u64_v, &u64_zero, &u64_fifty,"%" PRIu64 " ms");
        ImGui::SliderScalar("スライダー u64 高い範囲",      ImGuiDataType_U64,    &u64_v, &u64_hi_a, &u64_hi_b, "%" PRIu64 " ms");
        ImGui::SliderScalar("スライダー u64 全域",      ImGuiDataType_U64,    &u64_v, &u64_min,  &u64_max,  "%" PRIu64 " ms");
        ImGui::SliderScalar("スライダー float 低い範囲",     ImGuiDataType_Float,  &f32_v, &f32_zero, &f32_one);
        ImGui::SliderScalar("スライダー float 低い範囲 (対数)", ImGuiDataType_Float,  &f32_v, &f32_zero, &f32_one,  "%.10f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderScalar("スライダー float 高い範囲",    ImGuiDataType_Float,  &f32_v, &f32_lo_a, &f32_hi_a, "%e");
        ImGui::SliderScalar("スライダー double 低い範囲",    ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f グラム");
        ImGui::SliderScalar("スライダー double 低い範囲 (対数)",ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one,  "%.10f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderScalar("スライダー double 高い範囲",   ImGuiDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e グラム");

        ImGui::SeparatorText("スライダー (逆順)");
        ImGui::SliderScalar("スライダー s8 逆順",    ImGuiDataType_S8,   &s8_v,  &s8_max,    &s8_min,   "%d");
        ImGui::SliderScalar("スライダー u8 逆順",    ImGuiDataType_U8,   &u8_v,  &u8_max,    &u8_min,   "%u");
        ImGui::SliderScalar("スライダー s32 逆順",   ImGuiDataType_S32,  &s32_v, &s32_fifty, &s32_zero, "%d");
        ImGui::SliderScalar("スライダー u32 逆順",   ImGuiDataType_U32,  &u32_v, &u32_fifty, &u32_zero, "%u");
        ImGui::SliderScalar("スライダー s64 逆順",   ImGuiDataType_S64,  &s64_v, &s64_fifty, &s64_zero, "%" PRId64);
        ImGui::SliderScalar("スライダー u64 逆順",   ImGuiDataType_U64,  &u64_v, &u64_fifty, &u64_zero, "%" PRIu64 " ms");

        IMGUI_DEMO_MARKER("Widgets/Data Types/Inputs");
        static bool inputs_step = true;
        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
        ImGui::SeparatorText("入力");
        ImGui::Checkbox("ステップボタンを表示", &inputs_step);
        ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", &flags, ImGuiInputTextFlags_ReadOnly);
        ImGui::CheckboxFlags("ImGuiInputTextFlags_ParseEmptyRefVal", &flags, ImGuiInputTextFlags_ParseEmptyRefVal);
        ImGui::CheckboxFlags("ImGuiInputTextFlags_DisplayEmptyRefVal", &flags, ImGuiInputTextFlags_DisplayEmptyRefVal);
        ImGui::InputScalar("入力 s8",      ImGuiDataType_S8,     &s8_v,  inputs_step ? &s8_one  : NULL, NULL, "%d", flags);
        ImGui::InputScalar("入力 u8",      ImGuiDataType_U8,     &u8_v,  inputs_step ? &u8_one  : NULL, NULL, "%u", flags);
        ImGui::InputScalar("入力 s16",     ImGuiDataType_S16,    &s16_v, inputs_step ? &s16_one : NULL, NULL, "%d", flags);
        ImGui::InputScalar("入力 u16",     ImGuiDataType_U16,    &u16_v, inputs_step ? &u16_one : NULL, NULL, "%u", flags);
        ImGui::InputScalar("入力 s32",     ImGuiDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d", flags);
        ImGui::InputScalar("入力 s32 (16進)", ImGuiDataType_S32,    &s32_v, inputs_step ? &s32_one : NULL, NULL, "%04X", flags);
        ImGui::InputScalar("入力 u32",     ImGuiDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u", flags);
        ImGui::InputScalar("入力 u32 (16進)", ImGuiDataType_U32,    &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X", flags);
        ImGui::InputScalar("入力 s64",     ImGuiDataType_S64,    &s64_v, inputs_step ? &s64_one : NULL, NULL, NULL, flags);
        ImGui::InputScalar("入力 u64",     ImGuiDataType_U64,    &u64_v, inputs_step ? &u64_one : NULL, NULL, NULL, flags);
        ImGui::InputScalar("入力 float",   ImGuiDataType_Float,  &f32_v, inputs_step ? &f32_one : NULL, NULL, NULL, flags);
        ImGui::InputScalar("入力 double",  ImGuiDataType_Double, &f64_v, inputs_step ? &f64_one : NULL, NULL, NULL, flags);

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDisableBlocks()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDisableBlocks(ImGuiDemoWindowData* demo_data)
{
    if (ImGui::TreeNode("無効化ブロック"))
    {
        IMGUI_DEMO_MARKER("Widgets/Disable Blocks");
        ImGui::Checkbox("上のセクション全体を無効化する", &demo_data->DisableSections);
        ImGui::SameLine(); HelpMarker("BeginDisabled()/EndDisabled() を他のセクションにまたがって使う例です。");
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDragAndDrop()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDragAndDrop()
{
    if (ImGui::TreeNode("ドラッグ&ドロップ"))
    {
        IMGUI_DEMO_MARKER("Widgets/Drag and drop");
        if (ImGui::TreeNode("標準ウィジェットでのドラッグ&ドロップ"))
        {
            IMGUI_DEMO_MARKER("Widgets/Drag and drop/Standard widgets");
            // ColorEdit widgets automatically act as drag source and drag target.
            // They are using standardized payload strings IMGUI_PAYLOAD_TYPE_COLOR_3F and IMGUI_PAYLOAD_TYPE_COLOR_4F
            // to allow your own widgets to use colors in their drag and drop interaction.
            // Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
            HelpMarker("色の四角からドラッグできます。");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            ImGui::ColorEdit3("色 1", col1);
            ImGui::ColorEdit4("色 2", col2);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("ドラッグ&ドロップで項目をコピー/入れ替え"))
        {
            IMGUI_DEMO_MARKER("Widgets/Drag and drop/Copy-swap items");
            enum Mode
            {
                Mode_Copy,
                Mode_Move,
                Mode_Swap
            };
            static int mode = 0;
            if (ImGui::RadioButton("コピー", mode == Mode_Copy)) { mode = Mode_Copy; } ImGui::SameLine();
            if (ImGui::RadioButton("移動", mode == Mode_Move)) { mode = Mode_Move; } ImGui::SameLine();
            if (ImGui::RadioButton("入れ替え", mode == Mode_Swap)) { mode = Mode_Swap; }
            static const char* names[9] =
            {
                "Bobby", "Beatrice", "Betty",
                "Brianna", "Barry", "Bernard",
                "Bibi", "Blaine", "Bryn"
            };
            for (int n = 0; n < IM_COUNTOF(names); n++)
            {
                ImGui::PushID(n);
                if ((n % 3) != 0)
                    ImGui::SameLine();
                ImGui::Button(names[n], ImVec2(60, 60));

                // Our buttons are both drag sources and drag targets here!
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    // Set payload to carry the index of our item (could be anything)
                    ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));

                    // Display preview (could be anything, e.g. when dragging an image we could decide to display
                    // the filename and a small preview of the image, etc.)
                    if (mode == Mode_Copy) { ImGui::Text("%s をコピー", names[n]); }
                    if (mode == Mode_Move) { ImGui::Text("%s を移動", names[n]); }
                    if (mode == Mode_Swap) { ImGui::Text("%s を入れ替え", names[n]); }
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(int));
                        int payload_n = *(const int*)payload->Data;
                        if (mode == Mode_Copy)
                        {
                            names[n] = names[payload_n];
                        }
                        if (mode == Mode_Move)
                        {
                            names[n] = names[payload_n];
                            names[payload_n] = "";
                        }
                        if (mode == Mode_Swap)
                        {
                            const char* tmp = names[n];
                            names[n] = names[payload_n];
                            names[payload_n] = tmp;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("ドラッグで項目を並べ替え (簡易版)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Drag and Drop/Drag to reorder items (simple)");
            // FIXME: there is temporary (usually single-frame) ID Conflict during reordering as a same item may be submitting twice.
            // This code was always slightly faulty but in a way which was not easily noticeable.
            // Until we fix this, enable ImGuiItemFlags_AllowDuplicateId to disable detecting the issue.
            ImGui::PushItemFlag(ImGuiItemFlags_AllowDuplicateId, true);

            // Simple reordering
            HelpMarker(
                "ここではドラッグ&ドロップ API をまったく使っていません!"
                "代わりに、項目が押されていてホバーされていない状態を調べ、それに応じて順序を入れ替えています。");
            static const char* item_names[] = { "項目 1", "項目 2", "項目 3", "項目 4", "項目 5" };
            for (int n = 0; n < IM_COUNTOF(item_names); n++)
            {
                const char* item = item_names[n];
                ImGui::Selectable(item);

                if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
                {
                    int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                    if (n_next >= 0 && n_next < IM_COUNTOF(item_names))
                    {
                        item_names[n] = item_names[n_next];
                        item_names[n_next] = item;
                        ImGui::ResetMouseDragDelta();
                    }
                }
            }

            ImGui::PopItemFlag();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("ドロップ先に表示するツールチップ"))
        {
            IMGUI_DEMO_MARKER("Widgets/Drag and Drop/Tooltip at target location");
            for (int n = 0; n < 2; n++)
            {
                // Drop targets
                ImGui::Button(n ? "ここにドロップ##1" : "ここにドロップ##0");
                if (ImGui::BeginDragDropTarget())
                {
                    ImGuiDragDropFlags drop_target_flags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoPreviewTooltip;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F, drop_target_flags))
                    {
                        IM_UNUSED(payload);
                        ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                        ImGui::SetTooltip("ここにはドロップできません!");
                    }
                    ImGui::EndDragDropTarget();
                }

                // Drop source
                static ImVec4 col4 = { 1.0f, 0.0f, 0.2f, 1.0f };
                if (n == 0)
                    ImGui::ColorButton("ドラッグしてね", col4);

            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsDragsAndSliders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsDragsAndSliders()
{
    if (ImGui::TreeNode("ドラッグ/スライダーのフラグ"))
    {
        IMGUI_DEMO_MARKER("Widgets/Drag and Slider Flags");
        // Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags are the same!
        static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
        ImGui::CheckboxFlags("ImGuiSliderFlags_AlwaysClamp", &flags, ImGuiSliderFlags_AlwaysClamp);
        ImGui::CheckboxFlags("ImGuiSliderFlags_ClampOnInput", &flags, ImGuiSliderFlags_ClampOnInput);
        ImGui::SameLine(); HelpMarker("Ctrl+クリックで手入力したときに min/max の範囲に値を制限します。既定では Ctrl+クリックで範囲外の値も入力できます。");
        ImGui::CheckboxFlags("ImGuiSliderFlags_ClampZeroRange", &flags, ImGuiSliderFlags_ClampZeroRange);
        ImGui::SameLine(); HelpMarker("min==max==0.0f のときでも制限します。そうでない場合、DragXXX 関数は制限しません。");
        ImGui::CheckboxFlags("ImGuiSliderFlags_Logarithmic", &flags, ImGuiSliderFlags_Logarithmic);
        ImGui::SameLine(); HelpMarker("対数スケールでの編集を有効にします (小さい値をより細かく調整できます)。");
        ImGui::CheckboxFlags("ImGuiSliderFlags_NoRoundToFormat", &flags, ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SameLine(); HelpMarker("内部の値を書式文字列の精度に合わせて丸める処理を無効にします (例: %.3f なら小数点以下3桁に丸められます)。");
        ImGui::CheckboxFlags("ImGuiSliderFlags_NoInput", &flags, ImGuiSliderFlags_NoInput);
        ImGui::SameLine(); HelpMarker("Ctrl+クリックや Enter キーでウィジェットに直接テキスト入力する機能を無効にします。");
        ImGui::CheckboxFlags("ImGuiSliderFlags_NoSpeedTweaks", &flags, ImGuiSliderFlags_NoSpeedTweaks);
        ImGui::SameLine(); HelpMarker("修飾キーによる変化速度の切り替えを無効にします。独自のロジックで変化速度を制御したい場合に便利です。");
        ImGui::CheckboxFlags("ImGuiSliderFlags_WrapAround", &flags, ImGuiSliderFlags_WrapAround);
        ImGui::SameLine(); HelpMarker("最大値から最小値へ、最小値から最大値へ折り返せるようにします (DragXXX() 関数のみ対応)");
        ImGui::CheckboxFlags("ImGuiSliderFlags_ColorMarkers", &flags, ImGuiSliderFlags_ColorMarkers);

        // Drags
        static float drag_f = 0.5f;
        static float drag_f4[4];
        static int drag_i = 50;
        ImGui::Text("内部の float 値: %f", drag_f);
        ImGui::DragFloat("DragFloat (0 -> 1)", &drag_f, 0.005f, 0.0f, 1.0f, "%.3f", flags);
        ImGui::DragFloat("DragFloat (0 -> +inf)", &drag_f, 0.005f, 0.0f, FLT_MAX, "%.3f", flags);
        ImGui::DragFloat("DragFloat (-inf -> 1)", &drag_f, 0.005f, -FLT_MAX, 1.0f, "%.3f", flags);
        ImGui::DragFloat("DragFloat (-inf -> +inf)", &drag_f, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
        //ImGui::DragFloat("DragFloat (0 -> 0)", &drag_f, 0.005f, 0.0f, 0.0f, "%.3f", flags);           // To test ClampZeroRange
        //ImGui::DragFloat("DragFloat (100 -> 100)", &drag_f, 0.005f, 100.0f, 100.0f, "%.3f", flags);
        ImGui::DragInt("DragInt (0 -> 100)", &drag_i, 0.5f, 0, 100, "%d", flags);
        ImGui::DragFloat4("DragFloat4 (0 -> 1)", drag_f4, 0.005f, 0.0f, 1.0f, "%.3f", flags); // Multi-component item, mostly here to document the effect of ImGuiSliderFlags_ColorMarkers.

        // Sliders
        static float slider_f = 0.5f;
        static float slider_f4[4];
        static int slider_i = 50;
        const ImGuiSliderFlags flags_for_sliders = (flags & ~ImGuiSliderFlags_WrapAround);
        ImGui::Text("内部の float 値: %f", slider_f);
        ImGui::SliderFloat("SliderFloat (0 -> 1)", &slider_f, 0.0f, 1.0f, "%.3f", flags_for_sliders);
        ImGui::SliderInt("SliderInt (0 -> 100)", &slider_i, 0, 100, "%d", flags_for_sliders);
        ImGui::SliderFloat4("SliderFloat4 (0 -> 1)", slider_f4, 0.0f, 1.0f, "%.3f", flags); // Multi-component item, mostly here to document the effect of ImGuiSliderFlags_ColorMarkers.

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsFonts()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsFonts()
{
    if (ImGui::TreeNode("フォント"))
    {
        IMGUI_DEMO_MARKER("Widgets/Fonts");
        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        ImGui::ShowFontAtlas(atlas);
        // FIXME-NEWATLAS: Provide a demo to add/create a procedural font?
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsImages()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsImages()
{
    if (ImGui::TreeNode("画像"))
    {
        IMGUI_DEMO_MARKER("Widgets/Images");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::TextWrapped(
            "ここではフォントテクスチャを表示しています (このデモで扱えるテクスチャはこれだけです)。"
            "'ImTextureID' 型を保存領域として使い、自前のテクスチャデータへのポインターや識別子を渡してください。"
            "テクスチャにマウスを乗せると拡大表示されます!");

        // Below we are displaying the font texture because it is the only texture we have access to inside the demo!
        // Read description about ImTextureID/ImTextureRef and FAQ for details about texture identifiers.
        // If you use one of the default imgui_impl_XXXX.cpp rendering backend, they all have comments at the top
        // of their respective source file to specify what they are using as texture identifier, for example:
        // - The imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer.
        // - The imgui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier, etc.
        // So with the DirectX11 backend, you call ImGui::Image() with a 'ID3D11ShaderResourceView*' cast to ImTextureID.
        // - If you decided that ImTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers
        //   to ImGui::Image(), and gather width/height through your own functions, etc.
        // - You can use ShowMetricsWindow() to inspect the draw data that are being passed to your renderer,
        //   it will help you debug issues if you are confused about it.
        // - Consider using the lower-level ImDrawList::AddImage() API, via ImGui::GetWindowDrawList()->AddImage().
        // - Read https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
        // - Read https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples

        // Grab the current texture identifier used by the font atlas.
        ImFontAtlas* atlas = io.Fonts;
        ImTextureRef my_tex_id = atlas->TexRef;
        float my_tex_w = (float)atlas->TexData->Width; // Regular user code should never have to care about TexData-> fields, but since we want to display the entire texture here, we pull Width/Height from it.
        float my_tex_h = (float)atlas->TexData->Height;
        ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);

        // Basic drawing
        ImGui::SeparatorText("Image()/ImageWithBg() 関数");
        ImVec2 uv_min = ImVec2(0.0f, 0.0f); // Top-left
        ImVec2 uv_max = ImVec2(1.0f, 1.0f); // Lower-right
        ImGui::PushStyleVar(ImGuiStyleVar_ImageBorderSize, IM_MAX(1.0f, ImGui::GetStyle().ImageBorderSize));
        ImGui::ImageWithBg(my_tex_id, ImVec2(my_tex_w, my_tex_h), uv_min, uv_max, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PopStyleVar();

        // Fancy widget
        ImGui::SeparatorText("操作できる画像ビューアー");
        static ExampleImageViewerData image_viewer;
        ImVec2 canvas_size(ImGui::GetContentRegionAvail().x, my_tex_h * 2.0f);
        ExampleImageViewer_DrawOptions(&image_viewer);
        ExampleImageViewer_DrawCanvas(&image_viewer, canvas_size, my_tex_id, (int)my_tex_w, (int)my_tex_h);

        IMGUI_DEMO_MARKER("Widgets/Images/Textured buttons");
        ImGui::SeparatorText("テクスチャ付きボタン");
        ImGui::TextWrapped("続いてテクスチャ付きのボタンです..");
        static int pressed_count = 0;
        for (int i = 0; i < 8; i++)
        {
            // UV coordinates are often (0.0f, 0.0f) and (1.0f, 1.0f) to display an entire textures.
            // Here are trying to display only a 32x32 pixels area of the texture, hence the UV computation.
            // Read about UV coordinates here: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
            ImGui::PushID(i);
            if (i > 0)
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(i - 1.0f, i - 1.0f));
            ImVec2 size = ImVec2(32.0f, 32.0f);                         // Size of the image we want to make visible
            ImVec2 uv0 = ImVec2(0.0f, 0.0f);                            // UV coordinates for lower-left
            ImVec2 uv1 = ImVec2(32.0f / my_tex_w, 32.0f / my_tex_h);    // UV coordinates for (32,32) in our texture
            ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
            ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
            if (ImGui::ImageButton("", my_tex_id, size, uv0, uv1, bg_col, tint_col))
                pressed_count += 1;
            if (i > 0)
                ImGui::PopStyleVar();
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::NewLine();
        ImGui::Text("%d 回押されました。", pressed_count);
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsListBoxes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsListBoxes()
{
    if (ImGui::TreeNode("リストボックス"))
    {
        IMGUI_DEMO_MARKER("Widgets/List Boxes");
        // BeginListBox() is essentially a thin wrapper to using BeginChild()/EndChild()
        // using the ImGuiChildFlags_FrameStyle flag for stylistic changes + displaying a label.
        // You may be tempted to simply use BeginChild() directly. However note that BeginChild() requires EndChild()
        // to always be called (inconsistent with BeginListBox()/EndListBox()).

        // Using the generic BeginListBox() API, you have full control over how to display the combo contents.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
        // stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static int item_selected_idx = 0; // Here we store our selected data as an index.

        static bool item_highlight = false;
        int item_highlighted_idx = -1; // Here we store our highlighted data as an index.
        ImGui::Checkbox("2つ目のリストボックスでホバー中の項目を強調表示", &item_highlight);

        if (ImGui::BeginListBox("リストボックス 1"))
        {
            for (int n = 0; n < IM_COUNTOF(items); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_selected_idx = n;

                if (item_highlight && ImGui::IsItemHovered())
                    item_highlighted_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::SameLine(); HelpMarker("ここでは2つのボックスで選択状態を共有しています。");

        // Custom size: use all width, 5 items tall
        ImGui::Text("全幅:");
        if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (int n = 0; n < IM_COUNTOF(items); n++)
            {
                bool is_selected = (item_selected_idx == n);
                ImGuiSelectableFlags flags = (item_highlighted_idx == n) ? ImGuiSelectableFlags_Highlight : 0;
                if (ImGui::Selectable(items[n], is_selected, flags))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsMultiComponents()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsMultiComponents()
{
    if (ImGui::TreeNode("複数成分のウィジェット"))
    {
        IMGUI_DEMO_MARKER("Widgets/Multi-component Widgets");
        static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
        static int vec4i[4] = { 1, 5, 100, 255 };

        static ImGuiSliderFlags flags = 0;
        ImGui::CheckboxFlags("ImGuiSliderFlags_ColorMarkers", &flags, ImGuiSliderFlags_ColorMarkers); // Only passing this to Drag/Sliders

        ImGui::SeparatorText("2成分");
        ImGui::InputFloat2("入力 float2", vec4f);
        ImGui::InputInt2("入力 int2", vec4i);
        ImGui::DragFloat2("ドラッグ float2", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        ImGui::DragInt2("ドラッグ int2", vec4i, 1, 0, 255, NULL, flags);
        ImGui::SliderFloat2("スライダー float2", vec4f, 0.0f, 1.0f, NULL, flags);
        ImGui::SliderInt2("スライダー int2", vec4i, 0, 255, NULL, flags);

        ImGui::SeparatorText("3成分");
        ImGui::InputFloat3("入力 float3", vec4f);
        ImGui::InputInt3("入力 int3", vec4i);
        ImGui::DragFloat3("ドラッグ float3", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        ImGui::DragInt3("ドラッグ int3", vec4i, 1, 0, 255, NULL, flags);
        ImGui::SliderFloat3("スライダー float3", vec4f, 0.0f, 1.0f, NULL, flags);
        ImGui::SliderInt3("スライダー int3", vec4i, 0, 255, NULL, flags);

        ImGui::SeparatorText("4成分");
        ImGui::InputFloat4("入力 float4", vec4f);
        ImGui::InputInt4("入力 int4", vec4i);
        ImGui::DragFloat4("ドラッグ float4", vec4f, 0.01f, 0.0f, 1.0f, NULL, flags);
        ImGui::DragInt4("ドラッグ int4", vec4i, 1, 0, 255, NULL, flags);
        ImGui::SliderFloat4("スライダー float4", vec4f, 0.0f, 1.0f, NULL, flags);
        ImGui::SliderInt4("スライダー int4", vec4i, 0, 255, NULL, flags);

        ImGui::SeparatorText("範囲");
        static float begin = 10, end = 90;
        static int begin_i = 100, end_i = 1000;
        ImGui::DragFloatRange2("範囲 float", &begin, &end, 0.25f, 0.0f, 100.0f, "最小: %.1f %%", "最大: %.1f %%", ImGuiSliderFlags_AlwaysClamp);
        ImGui::DragIntRange2("範囲 int", &begin_i, &end_i, 5, 0, 1000, "最小: %d 単位", "最大: %d 単位");
        ImGui::DragIntRange2("範囲 int (制限なし)", &begin_i, &end_i, 5, 0, 0, "最小: %d 単位", "最大: %d 単位");

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsPlotting()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsPlotting()
{
    // Plot/Graph widgets are not very good.
// Consider using a third-party library such as ImPlot: https://github.com/epezent/implot
// (see others https://github.com/ocornut/imgui/wiki/Useful-Extensions)
    if (ImGui::TreeNode("グラフ描画"))
    {
        IMGUI_DEMO_MARKER("Widgets/Plotting");
        ImGui::Text("もっと本格的なグラフが必要ですか? ImPlot の利用を検討してください:");
        ImGui::TextLinkOpenURL("https://github.com/epezent/implot");
        ImGui::Separator();

        static bool animate = true;
        ImGui::Checkbox("アニメーション", &animate);

        // Plot as lines and plot as histogram
        static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
        ImGui::PlotLines("フレーム時間", arr, IM_COUNTOF(arr));
        ImGui::PlotHistogram("ヒストグラム", arr, IM_COUNTOF(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80.0f));
        //ImGui::SameLine(); HelpMarker("Consider using ImPlot instead!");

        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.
        static float values[90] = {};
        static int values_offset = 0;
        static double refresh_time = 0.0;
        if (!animate || refresh_time == 0.0)
            refresh_time = ImGui::GetTime();
        while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % IM_COUNTOF(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 60.0f;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            float average = 0.0f;
            for (int n = 0; n < IM_COUNTOF(values); n++)
                average += values[n];
            average /= (float)IM_COUNTOF(values);
            char overlay[32];
            sprintf(overlay, "平均 %f", average);
            ImGui::PlotLines("折れ線", values, IM_COUNTOF(values), values_offset, overlay, -1.0f, 1.0f, ImVec2(0, 80.0f));
        }

        // Use functions to generate output
        // FIXME: This is actually VERY awkward because current plot API only pass in indices.
        // We probably want an API passing floats and user provide sample rate/count.
        struct Funcs
        {
            static float Sin(void*, int i) { return sinf(i * 0.1f); }
            static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
        };
        static int func_type = 0, display_count = 70;
        ImGui::SeparatorText("関数");
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        ImGui::Combo("関数の種類", &func_type, "Sin\0Saw\0");
        ImGui::SameLine();
        ImGui::SliderInt("サンプル数", &display_count, 1, 400);
        float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
        ImGui::PlotLines("折れ線##2", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
        ImGui::PlotHistogram("ヒストグラム##2", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsProgressBars()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsProgressBars()
{
    if (ImGui::TreeNode("プログレスバー"))
    {
        IMGUI_DEMO_MARKER("Widgets/Progress Bars");
        // Animate a simple progress bar
        static float progress_accum = 0.0f, progress_dir = 1.0f;
        progress_accum += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
        if (progress_accum >= +1.1f) { progress_accum = +1.1f; progress_dir *= -1.0f; }
        if (progress_accum <= -0.1f) { progress_accum = -0.1f; progress_dir *= -1.0f; }

        const float progress = IM_CLAMP(progress_accum, 0.0f, 1.0f);

        // Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
        // or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("プログレスバー");

        char buf[32];
        sprintf(buf, "%d/%d", (int)(progress * 1753), 1753);
        ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);

        // Pass an animated negative value, e.g. -1.0f * (float)ImGui::GetTime() is the recommended value.
        // Adjust the factor if you want to adjust the animation speed.
        ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "検索中..");
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("不確定");

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsQueryingStatuses()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsQueryingStatuses()
{
    if (ImGui::TreeNode("項目の状態を調べる (Edited/Active/Hovered など)"))
    {
        IMGUI_DEMO_MARKER("Widgets/Querying Item Status (Edited,Active,Hovered etc.)");
        // Select an item type
        const char* item_names[] =
        {
            "Text", "Button", "Button (w/ repeat)", "Checkbox", "SliderFloat", "InputText", "InputTextMultiline", "InputFloat",
            "InputFloat3", "ColorEdit4", "Selectable", "MenuItem", "TreeNode", "TreeNode (w/ double-click)", "Combo", "ListBox"
        };
        static int item_type = 4;
        static bool item_disabled = false;
        ImGui::Combo("項目の種類", &item_type, item_names, IM_COUNTOF(item_names), IM_COUNTOF(item_names));
        ImGui::SameLine();
        HelpMarker("さまざまな種類の項目が IsItemXXX 系の関数とどう連動するかを試せます。ほとんどの ImGui 関数の bool 戻り値は、基本的に ImGui::IsItemHovered() を呼ぶのと同じ意味になります。");
        ImGui::Checkbox("項目を無効化する", &item_disabled);

        // Submit selected items so we can query their status in the code following it.
        bool ret = false;
        static bool b = false;
        static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
        static char str[16] = {};
        if (item_disabled)
            ImGui::BeginDisabled(true);
        if (item_type == 0) { ImGui::Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
        if (item_type == 1) { ret = ImGui::Button("ITEM: Button"); }                                    // Testing button
        if (item_type == 2) { ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true); ret = ImGui::Button("ITEM: Button"); ImGui::PopItemFlag(); } // Testing button (with repeater)
        if (item_type == 3) { ret = ImGui::Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
        if (item_type == 4) { ret = ImGui::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
        if (item_type == 5) { ret = ImGui::InputText("ITEM: InputText", &str[0], IM_COUNTOF(str)); }  // Testing input text (which handles tabbing)
        if (item_type == 6) { ret = ImGui::InputTextMultiline("ITEM: InputTextMultiline", &str[0], IM_COUNTOF(str)); } // Testing input text (which uses a child window)
        if (item_type == 7) { ret = ImGui::InputFloat("ITEM: InputFloat", col4f, 1.0f); }               // Testing +/- buttons on scalar input
        if (item_type == 8) { ret = ImGui::InputFloat3("ITEM: InputFloat3", col4f); }                   // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 9) { ret = ImGui::ColorEdit4("ITEM: ColorEdit4", col4f); }                     // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 10) { ret = ImGui::Selectable("ITEM: Selectable"); }                            // Testing selectable item
        if (item_type == 11) { ret = ImGui::MenuItem("ITEM: MenuItem"); }                                // Testing menu item (they use ImGuiButtonFlags_PressedOnRelease button policy)
        if (item_type == 12) { ret = ImGui::TreeNode("ITEM: TreeNode"); if (ret) ImGui::TreePop(); }     // Testing tree node
        if (item_type == 13) { ret = ImGui::TreeNodeEx("ITEM: TreeNode w/ ImGuiTreeNodeFlags_OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen); } // Testing tree node with ImGuiButtonFlags_PressedOnDoubleClick button policy.
        if (item_type == 14) { const char* items[] = { "リンゴ", "バナナ", "サクランボ", "キウイ" }; static int current = 1; ret = ImGui::Combo("ITEM: Combo", &current, items, IM_COUNTOF(items)); }
        if (item_type == 15) { const char* items[] = { "リンゴ", "バナナ", "サクランボ", "キウイ" }; static int current = 1; ret = ImGui::ListBox("ITEM: ListBox", &current, items, IM_COUNTOF(items), IM_COUNTOF(items)); }

        bool hovered_delay_none = ImGui::IsItemHovered();
        bool hovered_delay_stationary = ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary);
        bool hovered_delay_short = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort);
        bool hovered_delay_normal = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal);
        bool hovered_delay_tooltip = ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip); // = Normal + Stationary

        // Display the values of IsItemHovered() and other common item state functions.
        // Note that the ImGuiHoveredFlags_XXX flags can be combined.
        // Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
        // we query every state in a single call to avoid storing them and to simplify the code.
        ImGui::BulletText(
            "Return value = %d\n"
            "IsItemFocused() = %d\n"
            "IsItemHovered() = %d\n"
            "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
            "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
            "IsItemHovered(_AllowWhenOverlappedByItem) = %d\n"
            "IsItemHovered(_AllowWhenOverlappedByWindow) = %d\n"
            "IsItemHovered(_AllowWhenDisabled) = %d\n"
            "IsItemHovered(_RectOnly) = %d\n"
            "IsItemActive() = %d\n"
            "IsItemEdited() = %d\n"
            "IsItemActivated() = %d\n"
            "IsItemDeactivated() = %d\n"
            "IsItemDeactivatedAfterEdit() = %d\n"
            "IsItemVisible() = %d\n"
            "IsItemClicked() = %d\n"
            "IsItemToggledOpen() = %d\n"
            "GetItemRectMin() = (%.1f, %.1f)\n"
            "GetItemRectMax() = (%.1f, %.1f)\n"
            "GetItemRectSize() = (%.1f, %.1f)",
            ret,
            ImGui::IsItemFocused(),
            ImGui::IsItemHovered(),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlappedByItem),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlappedByWindow),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled),
            ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly),
            ImGui::IsItemActive(),
            ImGui::IsItemEdited(),
            ImGui::IsItemActivated(),
            ImGui::IsItemDeactivated(),
            ImGui::IsItemDeactivatedAfterEdit(),
            ImGui::IsItemVisible(),
            ImGui::IsItemClicked(),
            ImGui::IsItemToggledOpen(),
            ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y,
            ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y,
            ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y
        );
        ImGui::BulletText(
            "ホバー遅延/静止判定のテスト:\n"
            "IsItemHovered() = %d\n"
            "IsItemHovered(_Stationary) = %d\n"
            "IsItemHovered(_DelayShort) = %d\n"
            "IsItemHovered(_DelayNormal) = %d\n"
            "IsItemHovered(_Tooltip) = %d",
            hovered_delay_none, hovered_delay_stationary, hovered_delay_short, hovered_delay_normal, hovered_delay_tooltip);

        if (item_disabled)
            ImGui::EndDisabled();

        char buf[1] = "";
        ImGui::InputText("未使用", buf, IM_COUNTOF(buf), ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        HelpMarker("このウィジェットは、上のウィジェットから Tab で抜けて Deactivated() などの状態を確認するためだけに置いてあります。");

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("ウィンドウの状態を調べる (Focused/Hovered など)"))
    {
        IMGUI_DEMO_MARKER("Widgets/Querying Window Status (Focused,Hovered etc.)");
        static bool embed_all_inside_a_child_window = false;
        ImGui::Checkbox("_RootWindow フラグを試すため、すべてを子ウィンドウの中に入れる", &embed_all_inside_a_child_window);
        if (embed_all_inside_a_child_window)
            ImGui::BeginChild("outer_child", ImVec2(0, ImGui::GetFontSize() * 20.0f), ImGuiChildFlags_Borders);

        // Testing IsWindowFocused() function with its various flags.
        ImGui::BulletText(
            "IsWindowFocused() = %d\n"
            "IsWindowFocused(_ChildWindows) = %d\n"
            "IsWindowFocused(_ChildWindows|_NoPopupHierarchy) = %d\n"
            "IsWindowFocused(_ChildWindows|_DockHierarchy) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowFocused(_RootWindow) = %d\n"
            "IsWindowFocused(_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowFocused(_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowFocused(_AnyWindow) = %d\n",
            ImGui::IsWindowFocused(),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_NoPopupHierarchy),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_NoPopupHierarchy),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_DockHierarchy),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_NoPopupHierarchy),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_DockHierarchy),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow));

        // Testing IsWindowHovered() function with its various flags.
        ImGui::BulletText(
            "IsWindowHovered() = %d\n"
            "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
            "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
            "IsWindowHovered(_ChildWindows) = %d\n"
            "IsWindowHovered(_ChildWindows|_NoPopupHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_DockHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowHovered(_RootWindow) = %d\n"
            "IsWindowHovered(_RootWindow|_NoPopupHierarchy) = %d\n"
            "IsWindowHovered(_RootWindow|_DockHierarchy) = %d\n"
            "IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
            "IsWindowHovered(_AnyWindow) = %d\n"
            "IsWindowHovered(_Stationary) = %d\n",
            ImGui::IsWindowHovered(),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_NoPopupHierarchy),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_DockHierarchy),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_NoPopupHierarchy),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_DockHierarchy),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_NoPopupHierarchy),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_DockHierarchy),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_Stationary));

        ImGui::BeginChild("child", ImVec2(0, 50), ImGuiChildFlags_Borders);
        ImGui::Text("_ChildWindows フラグを試すための、もう1つの子ウィンドウです。");
        ImGui::EndChild();
        if (embed_all_inside_a_child_window)
            ImGui::EndChild();

        // Calling IsItemHovered() after begin returns the hovered status of the title bar.
        // This is useful in particular if you want to create a context menu associated to the title bar of a window.
        // This will also work when docked into a Tab (the Tab replace the Title Bar and guarantee the same properties).
        static bool test_window = false;
        ImGui::Checkbox("タイトルバーを調べるため Begin() の直後に Hovered/Active をテストする", &test_window);
        if (test_window)
        {
            // FIXME-DOCK: This window cannot be docked within the ImGui Demo window, this will cause a feedback loop and get them stuck.
            // Could we fix this through an ImGuiWindowClass feature? Or an API call to tag our parent as "don't skip items"?
            ImGui::Begin("タイトルバーの Hovered/Active テスト", &test_window);
            if (ImGui::BeginPopupContextItem()) // <-- This is using IsItemHovered()
            {
                if (ImGui::MenuItem("閉じる")) { test_window = false; }
                ImGui::EndPopup();
            }
            ImGui::Text(
                "IsItemHovered() (Begin 直後) = %d (== タイトルバーがホバーされているか)\n"
                "IsItemActive() (Begin 直後) = %d (== ウィンドウがクリック/移動されているか)\n",
                ImGui::IsItemHovered(), ImGui::IsItemActive());
            ImGui::End();
        }

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsSelectables()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsSelectables()
{
    //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("選択可能項目 (Selectable)"))
    {
        IMGUI_DEMO_MARKER("Widgets/Selectables");
        // Selectable() has 2 overloads:
        // - The one taking "bool selected" as a read-only selection information.
        //   When Selectable() has been clicked it returns true and you can alter selection state accordingly.
        // - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
        // The earlier is more flexible, as in real application your selection may be stored in many different ways
        // and not necessarily inside a bool value (e.g. in flags within objects, as an external list, etc).
        IMGUI_DEMO_MARKER("Widgets/Selectables/Basic");
        if (ImGui::TreeNode("基本"))
        {
            static bool selection[5] = { false, true, false, false };
            ImGui::Selectable("1. 選択できます", &selection[0]);
            ImGui::Selectable("2. 選択できます", &selection[1]);
            ImGui::Selectable("3. 選択できます", &selection[2]);
            if (ImGui::Selectable("4. ダブルクリックできます", selection[3], ImGuiSelectableFlags_AllowDoubleClick))
                if (ImGui::IsMouseDoubleClicked(0))
                    selection[3] = !selection[3];
            ImGui::TreePop();
        }

        IMGUI_DEMO_MARKER("Widgets/Selectables/Rendering more items on the same line");
        if (ImGui::TreeNode("同じ行に複数の項目を並べる"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selectables/Multiple items on the same line");
            // - Using SetNextItemAllowOverlap()
            // - Using the Selectable() override that takes "bool* p_selected" parameter, the bool value is toggled automatically.
            {
                static bool selected[3] = {};
                ImGui::SetNextItemAllowOverlap(); ImGui::Selectable("main.c", &selected[0]); ImGui::SameLine(); ImGui::SmallButton("リンク 1");
                ImGui::SetNextItemAllowOverlap(); ImGui::Selectable("hello.cpp", &selected[1]); ImGui::SameLine(); ImGui::SmallButton("リンク 2");
                ImGui::SetNextItemAllowOverlap(); ImGui::Selectable("hello.h", &selected[2]); ImGui::SameLine(); ImGui::SmallButton("リンク 3");
            }

            // (2)
            // - Using ImGuiSelectableFlags_AllowOverlap is a shortcut for calling SetNextItemAllowOverlap()
            // - No visible label, display contents inside the selectable bounds.
            // - We don't maintain actual selection in this example to keep things simple.
            ImGui::Spacing();
            {
                static bool checked[5] = {};
                static int selected_n = 0;
                const float color_marker_w = ImGui::CalcTextSize("x").x;
                for (int n = 0; n < 5; n++)
                {
                    ImGui::PushID(n);
                    ImGui::AlignTextToFramePadding();
                    if (ImGui::Selectable("##selectable", selected_n == n, ImGuiSelectableFlags_AllowOverlap))
                        selected_n = n;
                    ImGui::SameLine(0, 0);
                    ImGui::Checkbox("##check", &checked[n]);
                    ImGui::SameLine();
                    ImVec4 color((n & 1) ? 1.0f : 0.2f, (n & 2) ? 1.0f : 0.2f, 0.2f, 1.0f);
                    ImGui::ColorButton("##color", color, ImGuiColorEditFlags_NoTooltip, ImVec2(color_marker_w, 0));
                    ImGui::SameLine();
                    ImGui::Text("ラベル");
                    ImGui::PopID();
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("テーブルの中で"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selectables/In Tables");
            static bool selected[10] = {};

            if (ImGui::BeginTable("split1", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                for (int i = 0; i < 10; i++)
                {
                    char label[32];
                    sprintf(label, "項目 %d", i);
                    ImGui::TableNextColumn();
                    ImGui::Selectable(label, &selected[i]); // FIXME-TABLE: Selection overlap
                }
                ImGui::EndTable();
            }
            ImGui::Spacing();
            if (ImGui::BeginTable("split2", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                for (int i = 0; i < 10; i++)
                {
                    char label[32];
                    sprintf(label, "項目 %d", i);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Selectable(label, &selected[i], ImGuiSelectableFlags_SpanAllColumns);
                    ImGui::TableNextColumn();
                    ImGui::Text("別の内容");
                    ImGui::TableNextColumn();
                    ImGui::Text("123456");
                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("グリッド"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selectables/Grid");
            static char selected[4][4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

            // Add in a bit of silly fun...
            const float time = (float)ImGui::GetTime();
            const bool winning_state = memchr(selected, 0, sizeof(selected)) == NULL; // If all cells are selected...
            if (winning_state)
                ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f + 0.5f * cosf(time * 2.0f), 0.5f + 0.5f * sinf(time * 3.0f)));

            const float size = ImGui::CalcTextSize("セーラー").x;
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                {
                    if (x > 0)
                        ImGui::SameLine();
                    ImGui::PushID(y * 4 + x);
                    if (ImGui::Selectable("セーラー", selected[y][x] != 0, 0, ImVec2(size, size)))
                    {
                        // Toggle clicked cell + toggle neighbors
                        selected[y][x] ^= 1;
                        if (x > 0) { selected[y][x - 1] ^= 1; }
                        if (x < 3) { selected[y][x + 1] ^= 1; }
                        if (y > 0) { selected[y - 1][x] ^= 1; }
                        if (y < 3) { selected[y + 1][x] ^= 1; }
                    }
                    ImGui::PopID();
                }

            if (winning_state)
                ImGui::PopStyleVar();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("配置"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selectables/Alignment");
            HelpMarker(
                "既定では Selectable は style.SelectableTextAlign を使いますが、PushStyleVar() を使って項目ごとに"
                "上書きできます。既定は左揃えのままにしておくのがよいでしょう。そうしないと同じ行に複数の項目を"
                "並べるのが難しくなります。");

            static bool selected[3 * 3] = { true, false, true, false, true, false, true, false, true };
            const float size = ImGui::CalcTextSize("(1.0,1.0)").x;
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    ImVec2 alignment = ImVec2((float)x / 2.0f, (float)y / 2.0f);
                    char name[32];
                    sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
                    if (x > 0) ImGui::SameLine();
                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                    ImGui::Selectable(name, &selected[3 * y + x], ImGuiSelectableFlags_None, ImVec2(size, size));
                    ImGui::PopStyleVar();
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsSelectionAndMultiSelect()
//-----------------------------------------------------------------------------
// Multi-selection demos
// Also read: https://github.com/ocornut/imgui/wiki/Multi-Select
//-----------------------------------------------------------------------------

static const char* ExampleNames[] =
{
    "アーティチョーク", "ルッコラ", "アスパラガス", "アボカド", "タケノコ", "モヤシ", "豆", "ビーツ", "チコリ", "ピーマン",
    "ゴーヤ", "チンゲンサイ", "ブロッコリー", "芽キャベツ", "ゴボウ", "キャベツ", "ヒョウタン", "ケッパー", "ニンジン", "キャッサバ",
    "カリフラワー", "セロリ", "セロリアック", "ステムレタス", "ハヤトウリ", "カイラン", "トウモロコシ", "キュウリ"
};

// Extra functions to add deletion support to ImGuiSelectionBasicStorage
struct ExampleSelectionWithDeletion : ImGuiSelectionBasicStorage
{
    // Find which item should be Focused after deletion.
    // Call _before_ item submission. Return an index in the before-deletion item list, your item loop should call SetKeyboardFocusHere() on it.
    // The subsequent ApplyDeletionPostLoop() code will use it to apply Selection.
    // - We cannot provide this logic in core Dear ImGui because we don't have access to selection data.
    // - We don't actually manipulate the ImVector<> here, only in ApplyDeletionPostLoop(), but using similar API for consistency and flexibility.
    // - Important: Deletion only works if the underlying ImGuiID for your items are stable: aka not depend on their index, but on e.g. item id/ptr.
    // FIXME-MULTISELECT: Doesn't take account of the possibility focus target will be moved during deletion. Need refocus or scroll offset.
    int ApplyDeletionPreLoop(ImGuiMultiSelectIO* ms_io, int items_count)
    {
        if (Size == 0)
            return -1;

        // If focused item is not selected...
        const int focused_idx = (int)ms_io->NavIdItem;  // Index of currently focused item
        if (ms_io->NavIdSelected == false)  // This is merely a shortcut, == Contains(adapter->IndexToStorage(items, focused_idx))
        {
            ms_io->RangeSrcReset = true;    // Request to recover RangeSrc from NavId next frame. Would be ok to reset even when NavIdSelected==true, but it would take an extra frame to recover RangeSrc when deleting a selected item.
            return focused_idx;             // Request to focus same item after deletion.
        }

        // If focused item is selected: land on first unselected item after focused item.
        for (int idx = focused_idx + 1; idx < items_count; idx++)
            if (!Contains(GetStorageIdFromIndex(idx)))
                return idx;

        // If focused item is selected: otherwise return last unselected item before focused item.
        for (int idx = IM_MIN(focused_idx, items_count) - 1; idx >= 0; idx--)
            if (!Contains(GetStorageIdFromIndex(idx)))
                return idx;

        return -1;
    }

    // Rewrite item list (delete items) + update selection.
    // - Call after EndMultiSelect()
    // - We cannot provide this logic in core Dear ImGui because we don't have access to your items, nor to selection data.
    template<typename ITEM_TYPE>
    void ApplyDeletionPostLoop(ImGuiMultiSelectIO* ms_io, ImVector<ITEM_TYPE>& items, int item_curr_idx_to_select)
    {
        // Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
        // If NavId was not part of selection, we will stay on same item.
        ImVector<ITEM_TYPE> new_items;
        new_items.reserve(items.Size - Size);
        int item_next_idx_to_select = -1;
        for (int idx = 0; idx < items.Size; idx++)
        {
            if (!Contains(GetStorageIdFromIndex(idx)))
                new_items.push_back(items[idx]);
            if (item_curr_idx_to_select == idx)
                item_next_idx_to_select = new_items.Size - 1;
        }
        items.swap(new_items);

        // Update selection
        Clear();
        if (item_next_idx_to_select != -1 && ms_io->NavIdSelected)
            SetItemSelected(GetStorageIdFromIndex(item_next_idx_to_select), true);
    }
};

// Example: Implement dual list box storage and interface
struct ExampleDualListBox
{
    ImVector<ImGuiID>           Items[2];               // ID is index into ExampleName[]
    ImGuiSelectionBasicStorage  Selections[2];          // Store ExampleItemId into selection
    bool                        OptKeepSorted = true;

    void MoveAll(int src, int dst)
    {
        IM_ASSERT((src == 0 && dst == 1) || (src == 1 && dst == 0));
        for (ImGuiID item_id : Items[src])
            Items[dst].push_back(item_id);
        Items[src].clear();
        SortItems(dst);
        Selections[src].Swap(Selections[dst]);
        Selections[src].Clear();
    }
    void MoveSelected(int src, int dst)
    {
        for (int src_n = 0; src_n < Items[src].Size; src_n++)
        {
            ImGuiID item_id = Items[src][src_n];
            if (!Selections[src].Contains(item_id))
                continue;
            Items[src].erase(&Items[src][src_n]); // FIXME-OPT: Could be implemented more optimally (rebuild src items and swap)
            Items[dst].push_back(item_id);
            src_n--;
        }
        if (OptKeepSorted)
            SortItems(dst);
        Selections[src].Swap(Selections[dst]);
        Selections[src].Clear();
    }
    void ApplySelectionRequests(ImGuiMultiSelectIO* ms_io, int side)
    {
        // In this example we store item id in selection (instead of item index)
        Selections[side].UserData = Items[side].Data;
        Selections[side].AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx) { ImGuiID* items = (ImGuiID*)self->UserData; return items[idx]; };
        Selections[side].ApplyRequests(ms_io);
    }
    static int IMGUI_CDECL CompareItemsByValue(const void* lhs, const void* rhs)
    {
        const int* a = (const int*)lhs;
        const int* b = (const int*)rhs;
        return *a - *b;
    }
    void SortItems(int n)
    {
        qsort(Items[n].Data, (size_t)Items[n].Size, sizeof(Items[n][0]), CompareItemsByValue);
    }
    void Show()
    {
        //if (ImGui::Checkbox("Sorted", &OptKeepSorted) && OptKeepSorted) { SortItems(0); SortItems(1); }
        if (ImGui::BeginTable("split", 3, ImGuiTableFlags_None))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Left side
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);      // Buttons
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Right side
            ImGui::TableNextRow();

            int request_move_selected = -1;
            int request_move_all = -1;
            float child_height_0 = 0.0f;
            for (int side = 0; side < 2; side++)
            {
                // FIXME-MULTISELECT: Dual List Box: Add context menus
                // FIXME-NAV: Using ImGuiWindowFlags_NavFlattened exhibit many issues.
                ImVector<ImGuiID>& items = Items[side];
                ImGuiSelectionBasicStorage& selection = Selections[side];

                ImGui::TableSetColumnIndex((side == 0) ? 0 : 2);
                ImGui::Text("%s (%d)", (side == 0) ? "選べる野菜" : "カゴの中", items.Size);

                // Submit scrolling range to avoid glitches on moving/deletion
                const float items_height = ImGui::GetTextLineHeightWithSpacing();
                ImGui::SetNextWindowContentSize(ImVec2(0.0f, items.Size * items_height));

                bool child_visible;
                if (side == 0)
                {
                    // Left child is resizable
                    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing() * 4), ImVec2(FLT_MAX, FLT_MAX));
                    child_visible = ImGui::BeginChild("0", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY);
                    child_height_0 = ImGui::GetWindowSize().y;
                }
                else
                {
                    // Right child use same height as left one
                    child_visible = ImGui::BeginChild("1", ImVec2(-FLT_MIN, child_height_0), ImGuiChildFlags_FrameStyle);
                }
                if (child_visible)
                {
                    ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_BoxSelect1d;
                    ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, items.Size);
                    ApplySelectionRequests(ms_io, side);

                    for (int item_n = 0; item_n < items.Size; item_n++)
                    {
                        ImGuiID item_id = items[item_n];
                        bool item_is_selected = selection.Contains(item_id);
                        ImGui::SetNextItemSelectionUserData(item_n);
                        ImGui::Selectable(ExampleNames[item_id], item_is_selected, ImGuiSelectableFlags_AllowDoubleClick);
                        if (ImGui::IsItemFocused())
                        {
                            // FIXME-MULTISELECT: Dual List Box: Transfer focus
                            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
                                request_move_selected = side;
                            if (ImGui::IsMouseDoubleClicked(0)) // FIXME-MULTISELECT: Double-click on multi-selection?
                                request_move_selected = side;
                        }
                    }

                    ms_io = ImGui::EndMultiSelect();
                    ApplySelectionRequests(ms_io, side);
                }
                ImGui::EndChild();
            }

            // Buttons columns
            ImGui::TableSetColumnIndex(1);
            ImGui::NewLine();
            //ImVec2 button_sz = { ImGui::CalcTextSize(">>").x + ImGui::GetStyle().FramePadding.x * 2.0f, ImGui::GetFrameHeight() + padding.y * 2.0f };
            ImVec2 button_sz = { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };

            // (Using BeginDisabled()/EndDisabled() works but feels distracting given how it is currently visualized)
            if (ImGui::Button(">>", button_sz))
                request_move_all = 0;
            if (ImGui::Button(">", button_sz))
                request_move_selected = 0;
            if (ImGui::Button("<", button_sz))
                request_move_selected = 1;
            if (ImGui::Button("<<", button_sz))
                request_move_all = 1;

            // Process requests
            if (request_move_all != -1)
                MoveAll(request_move_all, request_move_all ^ 1);
            if (request_move_selected != -1)
                MoveSelected(request_move_selected, request_move_selected ^ 1);

            // FIXME-MULTISELECT: Support action from outside
            /*
            if (OptKeepSorted == false)
            {
                ImGui::NewLine();
                if (ImGui::ArrowButton("MoveUp", ImGuiDir_Up)) {}
                if (ImGui::ArrowButton("MoveDown", ImGuiDir_Down)) {}
            }
            */

            ImGui::EndTable();
        }
    }
};

static void DemoWindowWidgetsSelectionAndMultiSelect(ImGuiDemoWindowData* demo_data)
{
    if (ImGui::TreeNode("選択状態と複数選択"))
    {
        IMGUI_DEMO_MARKER("Widgets/Selection State & Multi-Select");
        HelpMarker("選択は Selectable() や TreeNode() などのウィジェットで作れます。選択状態はアプリケーション側のコード/データが保持します。");

        ImGui::BulletText("Wiki ページ:");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("imgui/wiki/Multi-Select", "https://github.com/ocornut/imgui/wiki/Multi-Select");

        // Without any fancy API: manage single-selection yourself.
        if (ImGui::TreeNode("単一選択"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Single-Select");
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "オブジェクト %d", n);
                if (ImGui::Selectable(buf, selected == n))
                    selected = n;
            }
            ImGui::TreePop();
        }

        // Demonstrate implementation a most-basic form of multi-selection manually
        // This doesn't support the Shift modifier which requires BeginMultiSelect()!
        if (ImGui::TreeNode("複数選択 (BeginMultiSelect を使わない簡易版)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (manual/simplified, without BeginMultiSelect)");
            HelpMarker("Ctrl を押しながらクリックすると複数の項目を選択できます。");
            static bool selection[5] = { false, false, false, false, false };
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "オブジェクト %d", n);
                if (ImGui::Selectable(buf, selection[n]))
                {
                    if (!ImGui::GetIO().KeyCtrl) // Clear selection when Ctrl is not held
                        memset(selection, 0, sizeof(selection));
                    selection[n] ^= 1; // Toggle current item
                }
            }
            ImGui::TreePop();
        }

        // Demonstrate handling proper multi-selection using the BeginMultiSelect/EndMultiSelect API.
        // Shift+Click w/ Ctrl and other standard features are supported.
        // We use the ImGuiSelectionBasicStorage helper which you may freely reimplement.
        if (ImGui::TreeNode("複数選択"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select");
            ImGui::Text("対応している機能:");
            ImGui::BulletText("キーボード操作 (矢印キー、PageUp/PageDown、Home/End、スペース)。");
            ImGui::BulletText("Ctrl キーで選択を保ったまま切り替え。");
            ImGui::BulletText("Shift キーで範囲選択。");
            ImGui::BulletText("Ctrl+A ですべて選択。");
            ImGui::BulletText("Escape で選択を解除。");
            ImGui::BulletText("クリックしてドラッグすると矩形選択。");
            ImGui::Text("ヒント: 「デモ->ツール->デバッグログ->Selection」で選択リクエストの発生をその場で確認できます。");

            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            const int ITEMS_COUNT = 50;
            static ImGuiSelectionBasicStorage selection;
            ImGui::Text("選択中: %d/%d", selection.Size, ITEMS_COUNT);

            // The BeginChild() has no purpose for selection logic, other that offering a scrolling region.
            if (ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
            {
                ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                for (int n = 0; n < ITEMS_COUNT; n++)
                {
                    char label[64];
                    sprintf(label, "オブジェクト %05d: %s", n, ExampleNames[n % IM_COUNTOF(ExampleNames)]);
                    bool item_is_selected = selection.Contains((ImGuiID)n);
                    ImGui::SetNextItemSelectionUserData(n);
                    ImGui::Selectable(label, item_is_selected);
                }

                ms_io = ImGui::EndMultiSelect();
                selection.ApplyRequests(ms_io);
            }
            ImGui::EndChild();
            ImGui::TreePop();
        }

        // Demonstrate using the clipper with BeginMultiSelect()/EndMultiSelect()
        if (ImGui::TreeNode("複数選択 (クリッパー使用)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (with clipper)");
            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            static ImGuiSelectionBasicStorage selection;

            ImGui::Text("追加された機能:");
            ImGui::BulletText("ImGuiListClipper を使用。");

            const int ITEMS_COUNT = 10000;
            ImGui::Text("選択中: %d/%d", selection.Size, ITEMS_COUNT);
            if (ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
            {
                ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                ImGuiListClipper clipper;
                clipper.Begin(ITEMS_COUNT);
                if (ms_io->RangeSrcItem != -1)
                    clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                while (clipper.Step())
                {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
                    {
                        char label[64];
                        sprintf(label, "オブジェクト %05d: %s", n, ExampleNames[n % IM_COUNTOF(ExampleNames)]);
                        bool item_is_selected = selection.Contains((ImGuiID)n);
                        ImGui::SetNextItemSelectionUserData(n);
                        ImGui::Selectable(label, item_is_selected);
                    }
                }

                ms_io = ImGui::EndMultiSelect();
                selection.ApplyRequests(ms_io);
            }
            ImGui::EndChild();
            ImGui::TreePop();
        }

        // Demonstrate dynamic item list + deletion support using the BeginMultiSelect/EndMultiSelect API.
        // In order to support Deletion without any glitches you need to:
        // - (1) If items are submitted in their own scrolling area, submit contents size SetNextWindowContentSize() ahead of time to prevent one-frame readjustment of scrolling.
        // - (2) Items needs to have persistent ID Stack identifier = ID needs to not depends on their index. PushID(index) = KO. PushID(item_id) = OK. This is in order to focus items reliably after a selection.
        // - (3) BeginXXXX process
        // - (4) Focus process
        // - (5) EndXXXX process
        if (ImGui::TreeNode("複数選択 (削除あり)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (with deletion)");
            // Storing items data separately from selection data.
            // (you may decide to store selection data inside your item (aka intrusive storage) if you don't need multiple views over same items)
            // Use a custom selection.Adapter: store item identifier in Selection (instead of index)
            static ImVector<ImGuiID> items;
            static ExampleSelectionWithDeletion selection;
            selection.UserData = (void*)&items;
            selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx) { ImVector<ImGuiID>* p_items = (ImVector<ImGuiID>*)self->UserData; return (*p_items)[idx]; }; // Index -> ID

            ImGui::Text("追加された機能:");
            ImGui::BulletText("Delete キーに対応した可変長リスト。");
            ImGui::Text("選択数: %d/%d", selection.Size, items.Size);

            // Initialize default list with 50 items + button to add/remove items.
            static ImGuiID items_next_id = 0;
            if (items_next_id == 0)
                for (ImGuiID n = 0; n < 50; n++)
                    items.push_back(items_next_id++);
            if (ImGui::SmallButton("20 項目を追加"))     { for (int n = 0; n < 20; n++) { items.push_back(items_next_id++); } }
            ImGui::SameLine();
            if (ImGui::SmallButton("20 項目を削除"))  { for (int n = IM_MIN(20, items.Size); n > 0; n--) { selection.SetItemSelected(items.back(), false); items.pop_back(); } }

            // (1) Extra to support deletion: Submit scrolling range to avoid glitches on deletion
            const float items_height = ImGui::GetTextLineHeightWithSpacing();
            ImGui::SetNextWindowContentSize(ImVec2(0.0f, items.Size * items_height));

            if (ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
            {
                ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, items.Size);
                selection.ApplyRequests(ms_io);

                const bool want_delete = ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (selection.Size > 0);
                const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;

                for (int n = 0; n < items.Size; n++)
                {
                    const ImGuiID item_id = items[n];
                    char label[64];
                    sprintf(label, "オブジェクト %05u: %s", item_id, ExampleNames[item_id % IM_COUNTOF(ExampleNames)]);

                    bool item_is_selected = selection.Contains(item_id);
                    ImGui::SetNextItemSelectionUserData(n);
                    ImGui::Selectable(label, item_is_selected);
                    if (item_curr_idx_to_focus == n)
                        ImGui::SetKeyboardFocusHere(-1);
                }

                // Apply multi-select requests
                ms_io = ImGui::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                if (want_delete)
                    selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);
            }
            ImGui::EndChild();
            ImGui::TreePop();
        }

        // Implement a Dual List Box (#6648)
        if (ImGui::TreeNode("複数選択 (2つのリストボックス)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (dual list box)");
            // Init default state
            static ExampleDualListBox dlb;
            if (dlb.Items[0].Size == 0 && dlb.Items[1].Size == 0)
                for (int item_id = 0; item_id < IM_COUNTOF(ExampleNames); item_id++)
                    dlb.Items[0].push_back((ImGuiID)item_id);

            // Show
            dlb.Show();

            ImGui::TreePop();
        }

        // Demonstrate using the clipper with BeginMultiSelect()/EndMultiSelect()
        if (ImGui::TreeNode("複数選択 (テーブル内)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (in a table)");
            static ImGuiSelectionBasicStorage selection;

            const int ITEMS_COUNT = 10000;
            ImGui::Text("選択中: %d/%d", selection.Size, ITEMS_COUNT);
            if (ImGui::BeginTable("##Basket", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter, ImVec2(0.0f, ImGui::GetFontSize() * 20)))
            {
                ImGui::TableSetupColumn("オブジェクト");
                ImGui::TableSetupColumn("操作");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, ITEMS_COUNT);
                selection.ApplyRequests(ms_io);

                ImGuiListClipper clipper;
                clipper.Begin(ITEMS_COUNT);
                if (ms_io->RangeSrcItem != -1)
                    clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                while (clipper.Step())
                {
                    for (int n = clipper.DisplayStart; n < clipper.DisplayEnd; n++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(n);
                        char label[64];
                        sprintf(label, "オブジェクト %05d: %s", n, ExampleNames[n % IM_COUNTOF(ExampleNames)]);
                        bool item_is_selected = selection.Contains((ImGuiID)n);
                        ImGui::SetNextItemSelectionUserData(n);
                        ImGui::Selectable(label, item_is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
                        ImGui::TableNextColumn();
                        ImGui::SmallButton("実行");
                        ImGui::PopID();
                    }
                }

                ms_io = ImGui::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("複数選択 (チェックボックス)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (checkboxes)");
            ImGui::Text("チェックボックスのリスト (Selectable ではない場合):");
            ImGui::BulletText("_NoAutoSelect と _NoAutoClear フラグを使用。");
            ImGui::BulletText("Shift+クリックで複数のチェックを切り替え。");
            ImGui::BulletText("Shift+キーボードで現在の値を他のチェックボックスにコピー。");

            // If you have an array of checkboxes, you may want to use NoAutoSelect + NoAutoClear and the ImGuiSelectionExternalStorage helper.
            static bool items[20] = {};
            static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_NoAutoSelect | ImGuiMultiSelectFlags_NoAutoClear | ImGuiMultiSelectFlags_ClearOnEscape;
            ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoAutoSelect", &flags, ImGuiMultiSelectFlags_NoAutoSelect);
            ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoAutoClear", &flags, ImGuiMultiSelectFlags_NoAutoClear);
            ImGui::CheckboxFlags("ImGuiMultiSelectFlags_BoxSelect2d", &flags, ImGuiMultiSelectFlags_BoxSelect2d); // Cannot use ImGuiMultiSelectFlags_BoxSelect1d as checkboxes are varying width.

            if (ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
            {
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, -1, IM_COUNTOF(items));
                ImGuiSelectionExternalStorage storage_wrapper;
                storage_wrapper.UserData = (void*)items;
                storage_wrapper.AdapterSetItemSelected = [](ImGuiSelectionExternalStorage* self, int n, bool selected) { bool* array = (bool*)self->UserData; array[n] = selected; };
                storage_wrapper.ApplyRequests(ms_io);
                for (int n = 0; n < 20; n++)
                {
                    char label[32];
                    sprintf(label, "項目 %d", n);
                    ImGui::SetNextItemSelectionUserData(n);
                    ImGui::Checkbox(label, &items[n]);
                }
                ms_io = ImGui::EndMultiSelect();
                storage_wrapper.ApplyRequests(ms_io);
            }
            ImGui::EndChild();

            ImGui::TreePop();
        }

        // Demonstrate individual selection scopes in same window
        if (ImGui::TreeNode("複数選択 (複数のスコープ)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (multiple scopes)");
            // Use default select: Pass index to SetNextItemSelectionUserData(), store index in Selection
            const int SCOPES_COUNT = 3;
            const int ITEMS_COUNT = 8; // Per scope
            static ImGuiSelectionBasicStorage selections_data[SCOPES_COUNT];

            // Use ImGuiMultiSelectFlags_ScopeRect to not affect other selections in same window.
            static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ScopeRect | ImGuiMultiSelectFlags_ClearOnEscape;// | ImGuiMultiSelectFlags_ClearOnClickVoid;
            if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ScopeWindow", &flags, ImGuiMultiSelectFlags_ScopeWindow) && (flags & ImGuiMultiSelectFlags_ScopeWindow))
                flags &= ~ImGuiMultiSelectFlags_ScopeRect;
            if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ScopeRect", &flags, ImGuiMultiSelectFlags_ScopeRect) && (flags & ImGuiMultiSelectFlags_ScopeRect))
                flags &= ~ImGuiMultiSelectFlags_ScopeWindow;
            ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ClearOnClickVoid", &flags, ImGuiMultiSelectFlags_ClearOnClickVoid);
            ImGui::CheckboxFlags("ImGuiMultiSelectFlags_BoxSelect1d", &flags, ImGuiMultiSelectFlags_BoxSelect1d);

            for (int selection_scope_n = 0; selection_scope_n < SCOPES_COUNT; selection_scope_n++)
            {
                ImGui::PushID(selection_scope_n);
                ImGuiSelectionBasicStorage* selection = &selections_data[selection_scope_n];
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection->Size, ITEMS_COUNT);
                selection->ApplyRequests(ms_io);

                ImGui::SeparatorText("選択スコープ");
                ImGui::Text("選択数: %d/%d", selection->Size, ITEMS_COUNT);

                for (int n = 0; n < ITEMS_COUNT; n++)
                {
                    char label[64];
                    sprintf(label, "オブジェクト %05d: %s", n, ExampleNames[n % IM_COUNTOF(ExampleNames)]);
                    bool item_is_selected = selection->Contains((ImGuiID)n);
                    ImGui::SetNextItemSelectionUserData(n);
                    ImGui::Selectable(label, item_is_selected);
                }

                // Apply multi-select requests
                ms_io = ImGui::EndMultiSelect();
                selection->ApplyRequests(ms_io);
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        // See ShowExampleAppAssetsBrowser()
        if (ImGui::TreeNode("複数選択 (タイル表示のアセットブラウザー)"))
        {
            ImGui::Checkbox("アセットブラウザー", &demo_data->ShowAppAssetsBrowser);
            ImGui::Text("(メニューの「サンプル->アセットブラウザー」からも開けます)");
            ImGui::TreePop();
        }

        // Demonstrate supporting multiple-selection in a tree.
        // - We don't use linear indices for selection user data, but our ExampleTreeNode* pointer directly!
        //   This showcase how SetNextItemSelectionUserData() never assume indices!
        // - The difficulty here is to "interpolate" from RangeSrcItem to RangeDstItem in the SetAll/SetRange request.
        //   We want this interpolation to match what the user sees: in visible order, skipping closed nodes.
        //   This is implemented by our TreeGetNextNodeInVisibleOrder() user-space helper.
        // - Important: In a real codebase aiming to implement full-featured selectable tree with custom filtering, you
        //   are more likely to build an array mapping sequential indices to visible tree nodes, since your
        //   filtering/search + clipping process will benefit from it. Having this will make this interpolation much easier.
        // - Consider this a prototype: we are working toward simplifying some of it.
        if (ImGui::TreeNode("複数選択 (ツリー)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (trees)");
            HelpMarker(
                "これはかなり高度で実験的な内容です。複数選択をこれから学ぶ場合は、"
                "ツリーでの使い方から始めないでください!\n\n"
                "将来のバージョンでは、この辺りをもっと簡単で整理されたものにする予定です。");

            struct ExampleTreeFuncs
            {
                static void DrawNode(ExampleTreeNode* node, ImGuiSelectionBasicStorage* selection)
                {
                    ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                    tree_node_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Enable pressing left to jump to parent
                    if (node->Childs.Size == 0)
                        tree_node_flags |= ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Leaf;
                    if (selection->Contains((ImGuiID)node->UID))
                        tree_node_flags |= ImGuiTreeNodeFlags_Selected;

                    // Using SetNextItemStorageID() to specify storage id, so we can easily peek into
                    // the storage holding open/close stage, using our TreeNodeGetOpen/TreeNodeSetOpen() functions.
                    ImGui::SetNextItemSelectionUserData((ImGuiSelectionUserData)(intptr_t)node);
                    ImGui::SetNextItemStorageID((ImGuiID)node->UID);
                    if (ImGui::TreeNodeEx(node->Name, tree_node_flags))
                    {
                        for (ExampleTreeNode* child : node->Childs)
                            DrawNode(child, selection);
                        ImGui::TreePop();
                    }
                    else if (ImGui::IsItemToggledOpen())
                    {
                        TreeCloseAndUnselectChildNodes(node, selection);
                    }
                }

                // When closing a node: 1) close and unselect all child nodes, 2) select parent if any child was selected.
                // FIXME: This is currently handled by user logic but I'm hoping to eventually provide tree node
                // features to do this automatically, e.g. a ImGuiTreeNodeFlags_AutoCloseChildNodes etc.
                static int TreeCloseAndUnselectChildNodes(ExampleTreeNode* node, ImGuiSelectionBasicStorage* selection, int depth = 0)
                {
                    // Recursive close (the test for depth == 0 is because we call this on a node that was just closed!)
                    int unselected_count = selection->Contains((ImGuiID)node->UID) ? 1 : 0;
                    if (depth == 0 || ImGui::TreeNodeGetOpen((ImGuiID)node->UID))
                    {
                        for (ExampleTreeNode* child : node->Childs)
                            unselected_count += TreeCloseAndUnselectChildNodes(child, selection, depth + 1);
                        ImGui::TreeNodeSetOpen((ImGuiID)node->UID, false);
                    }

                    // Select root node if any of its child was selected, otherwise unselect
                    selection->SetItemSelected((ImGuiID)node->UID, (depth == 0 && unselected_count > 0));
                    return unselected_count;
                }

                // Apply multi-selection requests
                static void ApplySelectionRequests(ImGuiMultiSelectIO* ms_io, ExampleTreeNode* tree, ImGuiSelectionBasicStorage* selection)
                {
                    for (ImGuiSelectionRequest& req : ms_io->Requests)
                    {
                        if (req.Type == ImGuiSelectionRequestType_SetAll)
                        {
                            if (req.Selected)
                                TreeSetAllInOpenNodes(tree, selection, req.Selected);
                            else
                                selection->Clear();
                        }
                        else if (req.Type == ImGuiSelectionRequestType_SetRange)
                        {
                            ExampleTreeNode* first_node = (ExampleTreeNode*)(intptr_t)req.RangeFirstItem;
                            ExampleTreeNode* last_node = (ExampleTreeNode*)(intptr_t)req.RangeLastItem;
                            for (ExampleTreeNode* node = first_node; node != NULL; node = TreeGetNextNodeInVisibleOrder(node, last_node))
                                selection->SetItemSelected((ImGuiID)node->UID, req.Selected);
                        }
                    }
                }

                static void TreeSetAllInOpenNodes(ExampleTreeNode* node, ImGuiSelectionBasicStorage* selection, bool selected)
                {
                    if (node->Parent != NULL) // Root node isn't visible nor selectable in our scheme
                        selection->SetItemSelected((ImGuiID)node->UID, selected);
                    if (node->Parent == NULL || ImGui::TreeNodeGetOpen((ImGuiID)node->UID))
                        for (ExampleTreeNode* child : node->Childs)
                            TreeSetAllInOpenNodes(child, selection, selected);
                }

                // Interpolate in *user-visible order* AND only *over opened nodes*.
                // If you have a sequential mapping tables (e.g. generated after a filter/search pass) this would be simpler.
                // Here the tricks are that:
                // - we store/maintain ExampleTreeNode::IndexInParent which allows implementing a linear iterator easily, without searches, without recursion.
                //   this could be replaced by a search in parent, aka 'int index_in_parent = curr_node->Parent->Childs.find_index(curr_node)'
                //   which would only be called when crossing from child to a parent, aka not too much.
                // - we call SetNextItemStorageID() before our TreeNode() calls with an ID which doesn't relate to UI stack,
                //   making it easier to call TreeNodeGetOpen()/TreeNodeSetOpen() from any location.
                static ExampleTreeNode* TreeGetNextNodeInVisibleOrder(ExampleTreeNode* curr_node, ExampleTreeNode* last_node)
                {
                    // Reached last node
                    if (curr_node == last_node)
                        return NULL;

                    // Recurse into childs. Query storage to tell if the node is open.
                    if (curr_node->Childs.Size > 0 && ImGui::TreeNodeGetOpen((ImGuiID)curr_node->UID))
                        return curr_node->Childs[0];

                    // Next sibling, then into our own parent
                    while (curr_node->Parent != NULL)
                    {
                        if (curr_node->IndexInParent + 1 < curr_node->Parent->Childs.Size)
                            return curr_node->Parent->Childs[curr_node->IndexInParent + 1];
                        curr_node = curr_node->Parent;
                    }
                    return NULL;
                }

            }; // ExampleTreeFuncs

            static ImGuiSelectionBasicStorage selection;
            if (demo_data->DemoTree == NULL)
                demo_data->DemoTree = ExampleTree_CreateDemoTree(); // Create tree once
            ImGui::Text("選択数: %d", selection.Size);

            if (ImGui::BeginChild("##Tree", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
            {
                ExampleTreeNode* tree = demo_data->DemoTree;
                ImGuiMultiSelectFlags ms_flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect2d;
                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(ms_flags, selection.Size, -1);
                ExampleTreeFuncs::ApplySelectionRequests(ms_io, tree, &selection);
                for (ExampleTreeNode* node : tree->Childs)
                    ExampleTreeFuncs::DrawNode(node, &selection);
                ms_io = ImGui::EndMultiSelect();
                ExampleTreeFuncs::ApplySelectionRequests(ms_io, tree, &selection);
            }
            ImGui::EndChild();

            ImGui::TreePop();
        }

        // Advanced demonstration of BeginMultiSelect()
        // - Showcase clipping.
        // - Showcase deletion.
        // - Showcase basic drag and drop.
        // - Showcase TreeNode variant (note that tree node don't expand in the demo: supporting expanding tree nodes + clipping a separate thing).
        // - Showcase using inside a table.
        //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("複数選択 (応用)"))
        {
            IMGUI_DEMO_MARKER("Widgets/Selection State/Multi-Select (advanced)");
            // Options
            enum WidgetType { WidgetType_Selectable, WidgetType_TreeNode };
            static bool use_clipper = true;
            static bool use_deletion = true;
            static bool use_drag_drop = true;
            static bool show_in_table = false;
            static bool show_color_button = true;
            static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
            static WidgetType widget_type = WidgetType_Selectable;

            if (ImGui::TreeNode("オプション"))
            {
                if (ImGui::RadioButton("Selectable を使う", widget_type == WidgetType_Selectable)) { widget_type = WidgetType_Selectable; }
                ImGui::SameLine();
                if (ImGui::RadioButton("ツリーノードを使う", widget_type == WidgetType_TreeNode)) { widget_type = WidgetType_TreeNode; }
                ImGui::SameLine();
                HelpMarker("TreeNode() も技術的には対応していますが… 正しく使うのはより複雑です (ツリーへの線形/ランダムアクセス手段が必要で、フィルターやクリッパーをすでに実装しているような高度なツリー構成に向いています)。今後、簡略化とデモの充実を進めていきます。\n\n現時点でのツリーのデモは、ルートノードしかない空のツリーなのであまり意味がありません。");
                ImGui::Checkbox("クリッパーを有効にする", &use_clipper);
                ImGui::Checkbox("削除を有効にする", &use_deletion);
                ImGui::Checkbox("ドラッグ&ドロップを有効にする", &use_drag_drop);
                ImGui::Checkbox("テーブルで表示する", &show_in_table);
                ImGui::Checkbox("カラーボタンを表示する", &show_color_button);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_SingleSelect", &flags, ImGuiMultiSelectFlags_SingleSelect);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoSelectAll", &flags, ImGuiMultiSelectFlags_NoSelectAll);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoRangeSelect", &flags, ImGuiMultiSelectFlags_NoRangeSelect);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoAutoSelect", &flags, ImGuiMultiSelectFlags_NoAutoSelect);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoAutoClear", &flags, ImGuiMultiSelectFlags_NoAutoClear);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoAutoClearOnReselect", &flags, ImGuiMultiSelectFlags_NoAutoClearOnReselect);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_NoSelectOnRightClick", &flags, ImGuiMultiSelectFlags_NoSelectOnRightClick);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_BoxSelect1d", &flags, ImGuiMultiSelectFlags_BoxSelect1d);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_BoxSelect2d", &flags, ImGuiMultiSelectFlags_BoxSelect2d);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_BoxSelectNoScroll", &flags, ImGuiMultiSelectFlags_BoxSelectNoScroll);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ClearOnEscape", &flags, ImGuiMultiSelectFlags_ClearOnEscape);
                ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ClearOnClickVoid", &flags, ImGuiMultiSelectFlags_ClearOnClickVoid);
                if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ScopeWindow", &flags, ImGuiMultiSelectFlags_ScopeWindow) && (flags & ImGuiMultiSelectFlags_ScopeWindow))
                    flags &= ~ImGuiMultiSelectFlags_ScopeRect;
                if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_ScopeRect", &flags, ImGuiMultiSelectFlags_ScopeRect) && (flags & ImGuiMultiSelectFlags_ScopeRect))
                    flags &= ~ImGuiMultiSelectFlags_ScopeWindow;
                if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_SelectOnAuto", &flags, ImGuiMultiSelectFlags_SelectOnAuto))
                    flags &= ~(ImGuiMultiSelectFlags_SelectOnMask_ ^ ImGuiMultiSelectFlags_SelectOnAuto);
                ImGui::SameLine(); HelpMarker("未選択の項目をクリックしたときはマウス押下時に、選択済みの項目をクリックしたときはマウスを離したときに選択を適用します。(既定)");
                if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_SelectOnClickAlways", &flags, ImGuiMultiSelectFlags_SelectOnClickAlways))
                    flags &= ~(ImGuiMultiSelectFlags_SelectOnMask_ ^ ImGuiMultiSelectFlags_SelectOnClickAlways);
                ImGui::SameLine(); HelpMarker("複数選択に対するドラッグ&ドロップはできなくなりますが、既存の選択範囲内をクリックしても BoxSelect などで常に選択し直せるようになります。(Excel のような挙動)");
                if (ImGui::CheckboxFlags("ImGuiMultiSelectFlags_SelectOnClickRelease", &flags, ImGuiMultiSelectFlags_SelectOnClickRelease))
                    flags &= ~(ImGuiMultiSelectFlags_SelectOnMask_ ^ ImGuiMultiSelectFlags_SelectOnClickRelease);
                ImGui::SameLine(); HelpMarker("未選択の項目を、選択状態を変えずにドラッグできるようにします。");
                ImGui::TreePop();
            }

            // Initialize default list with 1000 items.
            // Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
            static ImVector<int> items;
            static int items_next_id = 0;
            if (items_next_id == 0) { for (int n = 0; n < 1000; n++) { items.push_back(items_next_id++); } }
            static ExampleSelectionWithDeletion selection;
            static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu

            ImGui::Text("選択数: %d/%d", selection.Size, items.Size);

            const float items_height = (widget_type == WidgetType_TreeNode) ? ImGui::GetTextLineHeight() : ImGui::GetTextLineHeightWithSpacing();
            ImGui::SetNextWindowContentSize(ImVec2(0.0f, items.Size * items_height));
            if (ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
            {
                ImVec2 color_button_sz(ImGui::GetFontSize(), ImGui::GetFontSize());
                if (widget_type == WidgetType_TreeNode)
                    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.0f);

                ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, items.Size);
                selection.ApplyRequests(ms_io);

                const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
                const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;
                request_deletion_from_menu = false;

                if (show_in_table)
                {
                    if (widget_type == WidgetType_TreeNode)
                        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
                    ImGui::BeginTable("##Split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 0.70f);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 0.30f);
                    //ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.0f);
                }

                ImGuiListClipper clipper;
                if (use_clipper)
                {
                    clipper.Begin(items.Size);
                    if (item_curr_idx_to_focus != -1)
                        clipper.IncludeItemByIndex(item_curr_idx_to_focus); // Ensure focused item is not clipped.
                    if (ms_io->RangeSrcItem != -1)
                        clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.
                }

                while (!use_clipper || clipper.Step())
                {
                    const int item_begin = use_clipper ? clipper.DisplayStart : 0;
                    const int item_end = use_clipper ? clipper.DisplayEnd : items.Size;
                    for (int n = item_begin; n < item_end; n++)
                    {
                        if (show_in_table)
                            ImGui::TableNextColumn();

                        const int item_id = items[n];
                        const char* item_category = ExampleNames[item_id % IM_COUNTOF(ExampleNames)];
                        char label[64];
                        sprintf(label, "オブジェクト %05d: %s", item_id, item_category);

                        // IMPORTANT: for deletion refocus to work we need object ID to be stable,
                        // aka not depend on their index in the list. Here we use our persistent item_id
                        // instead of index to build a unique ID that will persist.
                        // (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
                        ImGui::PushID(item_id);

                        // Emit a color button, to test that Shift+LeftArrow landing on an item that is not part
                        // of the selection scope doesn't erroneously alter our selection.
                        if (show_color_button)
                        {
                            ImU32 dummy_col = (ImU32)((unsigned int)n * 0xC250B74B) | IM_COL32_A_MASK;
                            ImGui::ColorButton("##", ImColor(dummy_col), ImGuiColorEditFlags_NoTooltip, color_button_sz);
                            ImGui::SameLine();
                        }

                        // Submit item
                        bool item_is_selected = selection.Contains((ImGuiID)n);
                        bool item_is_open = false;
                        ImGui::SetNextItemSelectionUserData(n);
                        if (widget_type == WidgetType_Selectable)
                        {
                            ImGui::Selectable(label, item_is_selected, ImGuiSelectableFlags_None);
                        }
                        else if (widget_type == WidgetType_TreeNode)
                        {
                            ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                            if (item_is_selected)
                                tree_node_flags |= ImGuiTreeNodeFlags_Selected;
                            item_is_open = ImGui::TreeNodeEx(label, tree_node_flags);
                        }

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == n)
                            ImGui::SetKeyboardFocusHere(-1);

                        // Drag and Drop
                        if (use_drag_drop && ImGui::BeginDragDropSource())
                        {
                            // Create payload with full selection OR single unselected item.
                            // (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
                            if (ImGui::GetDragDropPayload() == NULL)
                            {
                                ImVector<int> payload_items;
                                void* it = NULL;
                                ImGuiID id = 0;
                                if (!item_is_selected)
                                    payload_items.push_back(item_id);
                                else
                                    while (selection.GetNextSelectedItem(&it, &id))
                                        payload_items.push_back((int)id);
                                ImGui::SetDragDropPayload("MULTISELECT_DEMO_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
                            }

                            // Display payload content in tooltip
                            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                            const int* payload_items = (int*)payload->Data;
                            const int payload_count = (int)payload->DataSize / (int)sizeof(int);
                            if (payload_count == 1)
                                ImGui::Text("オブジェクト %05d: %s", payload_items[0], ExampleNames[payload_items[0] % IM_COUNTOF(ExampleNames)]);
                            else
                                ImGui::Text("%d 個のオブジェクトをドラッグ中", payload_count);

                            ImGui::EndDragDropSource();
                        }

                        if (widget_type == WidgetType_TreeNode && item_is_open)
                            ImGui::TreePop();

                        // Right-click: context menu
                        if (ImGui::BeginPopupContextItem())
                        {
                            ImGui::BeginDisabled(!use_deletion || selection.Size == 0);
                            sprintf(label, "%d 件を削除###DeleteSelected", selection.Size);
                            if (ImGui::Selectable(label))
                                request_deletion_from_menu = true;
                            ImGui::EndDisabled();
                            ImGui::Selectable("閉じる");
                            ImGui::EndPopup();
                        }

                        // Demo content within a table
                        if (show_in_table)
                        {
                            ImGui::TableNextColumn();
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                            ImGui::InputText("##NoLabel", (char*)(void*)item_category, strlen(item_category), ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopStyleVar();
                        }

                        ImGui::PopID();
                    }
                    if (!use_clipper)
                        break;
                }

                if (show_in_table)
                {
                    ImGui::EndTable();
                    if (widget_type == WidgetType_TreeNode)
                        ImGui::PopStyleVar();
                }

                // Apply multi-select requests
                ms_io = ImGui::EndMultiSelect();
                selection.ApplyRequests(ms_io);
                if (want_delete)
                    selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);

                if (widget_type == WidgetType_TreeNode)
                    ImGui::PopStyleVar();
            }
            ImGui::EndChild();
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTabs()
//-----------------------------------------------------------------------------

static void EditTabBarFittingPolicyFlags(ImGuiTabBarFlags* p_flags)
{
    if ((*p_flags & ImGuiTabBarFlags_FittingPolicyMask_) == 0)
        *p_flags |= ImGuiTabBarFlags_FittingPolicyDefault_;
    if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyMixed", p_flags, ImGuiTabBarFlags_FittingPolicyMixed))
        *p_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyMixed);
    if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyShrink", p_flags, ImGuiTabBarFlags_FittingPolicyShrink))
        *p_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyShrink);
    if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyScroll", p_flags, ImGuiTabBarFlags_FittingPolicyScroll))
        *p_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyScroll);
}

static void DemoWindowWidgetsTabs()
{
    if (ImGui::TreeNode("タブ"))
    {
        IMGUI_DEMO_MARKER("Widgets/Tabs");
        if (ImGui::TreeNode("基本"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tabs/Basic");
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("アボカド"))
                {
                    ImGui::Text("これはアボカドのタブです!\nなんとかかんとか、なんとかかんとか");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("ブロッコリー"))
                {
                    ImGui::Text("これはブロッコリーのタブです!\nなんとかかんとか、なんとかかんとか");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("キュウリ"))
                {
                    ImGui::Text("これはキュウリのタブです!\nなんとかかんとか、なんとかかんとか");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::Separator();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("応用と閉じるボタン"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tabs/Advanced & Close Button");
            // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
            static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
            ImGui::CheckboxFlags("ImGuiTabBarFlags_Reorderable", &tab_bar_flags, ImGuiTabBarFlags_Reorderable);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_AutoSelectNewTabs", &tab_bar_flags, ImGuiTabBarFlags_AutoSelectNewTabs);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_TabListPopupButton", &tab_bar_flags, ImGuiTabBarFlags_TabListPopupButton);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_NoCloseWithMiddleMouseButton", &tab_bar_flags, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_DrawSelectedOverline", &tab_bar_flags, ImGuiTabBarFlags_DrawSelectedOverline);
            EditTabBarFittingPolicyFlags(&tab_bar_flags);

            // Tab Bar
            ImGui::AlignTextToFramePadding();
            ImGui::Text("開いているタブ:");
            const char* names[4] = { "アーティチョーク", "ビーツ", "セロリ", "大根" };
            static bool opened[4] = { true, true, true, true }; // Persistent user state
            for (int n = 0; n < IM_COUNTOF(opened); n++)
            {
                ImGui::SameLine();
                ImGui::Checkbox(names[n], &opened[n]);
            }

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
            // the underlying bool will be set to false when the tab is closed.
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                for (int n = 0; n < IM_COUNTOF(opened); n++)
                    if (opened[n] && ImGui::BeginTabItem(names[n], &opened[n], ImGuiTabItemFlags_None))
                    {
                        ImGui::Text("これは %s のタブです!", names[n]);
                        if (n & 1)
                            ImGui::Text("これは奇数番目のタブです。");
                        ImGui::EndTabItem();
                    }
                ImGui::EndTabBar();
            }
            ImGui::Separator();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("TabItemButton と Leading/Trailing フラグ"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tabs/TabItemButton & Leading-Trailing flags");
            static ImVector<int> active_tabs;
            static int next_tab_id = 0;
            if (next_tab_id == 0) // Initialize with some default tabs
                for (int i = 0; i < 3; i++)
                    active_tabs.push_back(next_tab_id++);

            // TabItemButton() and Leading/Trailing flags are distinct features which we will demo together.
            // (It is possible to submit regular tabs with Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing flags...
            // but they tend to make more sense together)
            static bool show_leading_button = true;
            static bool show_trailing_button = true;
            ImGui::Checkbox("先頭に TabItemButton() を表示", &show_leading_button);
            ImGui::Checkbox("末尾に TabItemButton() を表示", &show_trailing_button);

            // Expose some other flags which are useful to showcase how they interact with Leading/Trailing tabs
            static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyShrink;
            EditTabBarFittingPolicyFlags(&tab_bar_flags);

            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                // Demo a Leading TabItemButton(): click the "?" button to open a menu
                if (show_leading_button)
                    if (ImGui::TabItemButton("?", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip))
                        ImGui::OpenPopup("MyHelpMenu");
                if (ImGui::BeginPopup("MyHelpMenu"))
                {
                    ImGui::Selectable("こんにちは!");
                    ImGui::EndPopup();
                }

                // Demo Trailing Tabs: click the "+" button to add a new tab.
                // (In your app you may want to use a font icon instead of the "+")
                // We submit it before the regular tabs, but thanks to the ImGuiTabItemFlags_Trailing flag it will always appear at the end.
                if (show_trailing_button)
                    if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
                        active_tabs.push_back(next_tab_id++); // Add new tab

                // Submit our regular tabs
                for (int n = 0; n < active_tabs.Size; )
                {
                    bool open = true;
                    char name[16];
                    snprintf(name, IM_COUNTOF(name), "%04d", active_tabs[n]);
                    if (ImGui::BeginTabItem(name, &open, ImGuiTabItemFlags_None))
                    {
                        ImGui::Text("これは %s のタブです!", name);
                        ImGui::EndTabItem();
                    }

                    if (!open)
                        active_tabs.erase(active_tabs.Data + n);
                    else
                        n++;
                }

                ImGui::EndTabBar();
            }
            ImGui::Separator();
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsText()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsText()
{
    if (ImGui::TreeNode("テキスト"))
    {
        IMGUI_DEMO_MARKER("Widgets/Text");
        if (ImGui::TreeNode("色付きテキスト"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text/Colored Text");
            // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "ピンク");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "黄色");
            ImGui::TextDisabled("無効状態");
            ImGui::SameLine(); HelpMarker("TextDisabled の色は ImGuiStyle に保存されています。");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("フォントサイズ"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text/Font Size");
            ImGuiStyle& style = ImGui::GetStyle();
            const float global_scale = style.FontScaleMain * style.FontScaleDpi;
            ImGui::Text("style.FontScaleMain = %0.2f", style.FontScaleMain);
            ImGui::Text("style.FontScaleDpi = %0.2f", style.FontScaleDpi);
            ImGui::Text("global_scale = ~%0.2f", global_scale); // This is not technically accurate as internal scales may apply, but conceptually let's pretend it is.
            ImGui::Text("FontSize = %0.2f", ImGui::GetFontSize());

            ImGui::SeparatorText("");
            static float custom_size = 16.0f;
            ImGui::SliderFloat("custom_size", &custom_size, 10.0f, 100.0f, "%.0f");
            ImGui::Text("ImGui::PushFont(nullptr, custom_size);");
            ImGui::PushFont(NULL, custom_size);
            ImGui::Text("FontSize = %.2f (== %.2f * global_scale)", ImGui::GetFontSize(), custom_size);
            ImGui::PopFont();

            ImGui::SeparatorText("");
            static float custom_scale = 1.0f;
            ImGui::SliderFloat("custom_scale", &custom_scale, 0.5f, 4.0f, "%.2f");
            ImGui::Text("ImGui::PushFont(nullptr, style.FontSizeBase * custom_scale);");
            ImGui::PushFont(NULL, style.FontSizeBase * custom_scale);
            ImGui::Text("FontSize = %.2f (== style.FontSizeBase * %.2f * global_scale)", ImGui::GetFontSize(), custom_scale);
            ImGui::PopFont();

            ImGui::SeparatorText("");
            for (float scaling = 0.5f; scaling <= 4.0f; scaling += 0.5f)
            {
                ImGui::PushFont(NULL, style.FontSizeBase * scaling);
                ImGui::Text("FontSize = %.2f (== style.FontSizeBase * %.2f * global_scale)", ImGui::GetFontSize(), scaling);
                ImGui::PopFont();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("テキストの折り返し"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text/Word Wrapping");
            // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
            ImGui::TextWrapped(
                "このテキストはウィンドウの端で自動的に折り返されるはずです。現在の折り返し処理は、"
                "英語やその他いくつかの言語に適した単純な規則に従っています。");
            ImGui::Spacing();

            static float wrap_width = 200.0f;
            ImGui::SliderFloat("折り返し幅", &wrap_width, -20, 600, "%.0f");

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for (int n = 0; n < 2; n++)
            {
                ImGui::Text("テスト段落 %d:", n);
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImVec2 marker_min = ImVec2(pos.x + wrap_width, pos.y);
                ImVec2 marker_max = ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight());
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
                if (n == 0)
                    ImGui::Text("のんびり屋の犬はいい犬です。この段落は %.0f ピクセル以内に収まるはずです。1文字の単語も試します。素早い茶色のキツネがのんびり屋の犬を飛び越えます。", wrap_width);
                else
                    ImGui::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");

                // Draw actual text bounding box, following by marker of our expected limit (should not overlap!)
                draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
                draw_list->AddRectFilled(marker_min, marker_max, IM_COL32(255, 0, 255, 255));
                ImGui::PopTextWrapPos();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("UTF-8 テキスト"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text/UTF-8 Text");
            // UTF-8 test with Japanese characters
            // (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See docs/FONTS.md for details.)
            // - From C++11 you can use the u8"my text" syntax to encode literal strings as UTF-8
            // - For earlier compiler, you may be able to encode your sources as UTF-8 (e.g. in Visual Studio, you
            //   can save your source files as 'UTF-8 without signature').
            // - FOR THIS DEMO FILE ONLY, BECAUSE WE WANT TO SUPPORT OLD COMPILERS, WE ARE *NOT* INCLUDING RAW UTF-8
            //   CHARACTERS IN THIS SOURCE FILE. Instead we are encoding a few strings with hexadecimal constants.
            //   Don't do this in your application! Please use u8"text in any language" in your application!
            // Note that characters values are preserved even by InputText() if the font cannot be displayed,
            // so you can safely copy & paste garbled characters into another application.
            ImGui::TextWrapped(
                "CJK のテキストは、フォントが対応する CJK 文字範囲付きで読み込まれている場合にのみ表示されます。"
                "追加の文字範囲を読み込むには io.Fonts->AddFontFromFileTTF() を自分で呼んでください。"
                "詳しくは docs/FONTS.md を参照してください。");
            ImGui::Text("ひらがな: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)");
            ImGui::Text("漢字: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
            static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
            //static char buf[32] = u8"NIHONGO"; // <- this is how you would write it with C++11, using real kanjis
            ImGui::InputText("UTF-8 入力", buf, IM_COUNTOF(buf));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTextFilter()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTextFilter()
{
    if (ImGui::TreeNode("テキストフィルター"))
    {
        IMGUI_DEMO_MARKER("Widgets/Text Filter");
        // Helper class to easy setup a text filter.
        // You may want to implement a more feature-full filtering scheme in your own application.
        HelpMarker("厳密にはウィジェットではありませんが、ImGuiTextFilter は文字列に対する簡単な絞り込みを行うためのヘルパーです。");
        static ImGuiTextFilter filter;
        ImGui::Text("フィルターの使い方:\n"
            "  \"\"         すべての行を表示\n"
            "  \"xxx\"      \"xxx\" を含む行を表示\n"
            "  \"xxx,yyy\"  \"xxx\" または \"yyy\" を含む行を表示\n"
            "  \"-xxx\"     \"xxx\" を含む行を隠す");
        filter.Draw();
        const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
        for (int i = 0; i < IM_COUNTOF(lines); i++)
            if (filter.PassFilter(lines[i]))
                ImGui::BulletText("%s", lines[i]);
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTextInput()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTextInput()
{
    // To wire InputText() with std::string or any other custom string type,
    // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
    if (ImGui::TreeNode("テキスト入力"))
    {
        IMGUI_DEMO_MARKER("Widgets/Text Input");
        if (ImGui::TreeNode("複数行テキスト入力"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Multi-line Text Input");
            // WE ARE USING A FIXED-SIZE BUFFER FOR SIMPLICITY HERE.
            // If you want to use InputText() with std::string or any custom dynamic string type:
            // - For std::string: use the wrapper in misc/cpp/imgui_stdlib.h/.cpp
            // - Otherwise, see the 'Dear ImGui Demo->Widgets->Text Input->Resize Callback' for using ImGuiInputTextFlags_CallbackResize.
            static char text[1024 * 16] =
                "/*\n"
                " Pentium F00F バグ (問題となる命令の16進エンコード\n"
                " F0 0F C7 C8 を縮めた呼び名) は、\n"
                " 正式には「ロック付き CMPXCHG8B 命令の不正オペランド\n"
                " バグ」と呼ばれ、P5 マイクロアーキテクチャの\n"
                " Intel Pentium、Pentium MMX、Pentium OverDrive\n"
                " プロセッサーの大半に存在する設計上の欠陥です。\n"
                "*/\n\n"
                "label:\n"
                "\tlock cmpxchg8b eax\n";

            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            HelpMarker("InputTextMultiline() を可変長の文字列型と連携させたい場合は ImGuiInputTextFlags_CallbackResize の仕組みを使えます。例は misc/cpp/imgui_stdlib.h を参照してください。(ここで <string> をインクルードしたくないため、imgui_demo.cpp では紹介していません)");
            ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", &flags, ImGuiInputTextFlags_ReadOnly);
            ImGui::CheckboxFlags("ImGuiInputTextFlags_WordWrap", &flags, ImGuiInputTextFlags_WordWrap);
            ImGui::SameLine(); HelpMarker("この機能は現在ベータ版です。imgui.h のコメントを読んでください");
            ImGui::CheckboxFlags("ImGuiInputTextFlags_AllowTabInput", &flags, ImGuiInputTextFlags_AllowTabInput);
            ImGui::SameLine(); HelpMarker("_AllowTabInput が設定されている場合、Tab でウィジェットを通過しても自動的にアクティブにはなりません。後続のウィジェットへ順に移動できるようにするためです。");
            ImGui::CheckboxFlags("ImGuiInputTextFlags_CtrlEnterForNewLine", &flags, ImGuiInputTextFlags_CtrlEnterForNewLine);
            ImGui::InputTextMultiline("##source", text, IM_COUNTOF(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("文字種を制限したテキスト入力"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Filtered Text Input");
            struct TextFilters
            {
                // Modify character input by altering 'data->Eventchar' (ImGuiInputTextFlags_CallbackCharFilter callback)
                static int FilterCasingSwap(ImGuiInputTextCallbackData* data)
                {
                    if (data->EventChar >= 'a' && data->EventChar <= 'z') { data->EventChar -= 'a' - 'A'; } // Lowercase becomes uppercase
                    else if (data->EventChar >= 'A' && data->EventChar <= 'Z') { data->EventChar += 'a' - 'A'; } // Uppercase becomes lowercase
                    return 0;
                }

                // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i', otherwise return 1 (filter out)
                static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
                {
                    if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar))
                        return 0;
                    return 1;
                }
            };

            static char buf1[32] = ""; ImGui::InputText("既定", buf1, IM_COUNTOF(buf1));
            static char buf2[32] = ""; ImGui::InputText("10進数", buf2, IM_COUNTOF(buf2), ImGuiInputTextFlags_CharsDecimal);
            static char buf3[32] = ""; ImGui::InputText("16進数", buf3, IM_COUNTOF(buf3), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
            static char buf4[32] = ""; ImGui::InputText("大文字のみ", buf4, IM_COUNTOF(buf4), ImGuiInputTextFlags_CharsUppercase);
            static char buf5[32] = ""; ImGui::InputText("空白なし", buf5, IM_COUNTOF(buf5), ImGuiInputTextFlags_CharsNoBlank);
            static char buf6[32] = ""; ImGui::InputText("大文字小文字を入れ替え", buf6, IM_COUNTOF(buf6), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterCasingSwap); // Use CharFilter callback to replace characters.
            static char buf7[32] = ""; ImGui::InputText("\"imgui\"", buf7, IM_COUNTOF(buf7), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters); // Use CharFilter callback to disable some characters.
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("パスワード入力"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Password input");
            static char password[64] = "password123";
            ImGui::InputText("パスワード", password, IM_COUNTOF(password), ImGuiInputTextFlags_Password);
            ImGui::SameLine(); HelpMarker("すべての文字を '*' で表示します。\nクリップボードへの切り取りとコピーを無効にします。\nログ出力を無効にします。\n");
            ImGui::InputTextWithHint("パスワード (ヒント付き)", "<パスワード>", password, IM_COUNTOF(password), ImGuiInputTextFlags_Password);
            ImGui::InputText("パスワード (平文表示)", password, IM_COUNTOF(password));
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("補完・履歴・編集のコールバック"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Completion, History, Edit Callbacks");
            struct Funcs
            {
                static int MyCallback(ImGuiInputTextCallbackData* data)
                {
                    if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
                    {
                        data->InsertChars(data->CursorPos, "..");
                    }
                    else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
                    {
                        if (data->EventKey == ImGuiKey_UpArrow)
                        {
                            data->DeleteChars(0, data->BufTextLen);
                            data->InsertChars(0, "上キーが押されました!");
                            data->SelectAll();
                        }
                        else if (data->EventKey == ImGuiKey_DownArrow)
                        {
                            data->DeleteChars(0, data->BufTextLen);
                            data->InsertChars(0, "下キーが押されました!");
                            data->SelectAll();
                        }
                    }
                    else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
                    {
                        // Toggle casing of first character
                        char c = data->Buf[0];
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) data->Buf[0] ^= 32;
                        data->BufDirty = true;

                        // Increment a counter
                        int* p_int = (int*)data->UserData;
                        *p_int = *p_int + 1;
                    }
                    return 0;
                }
            };
            static char buf1[64];
            ImGui::InputText("補完", buf1, IM_COUNTOF(buf1), ImGuiInputTextFlags_CallbackCompletion, Funcs::MyCallback);
            ImGui::SameLine(); HelpMarker(
                "ここでは Tab を押すたびに \"..\" を追加しています。"
                "このコールバックのより実用的な例は「サンプル>コンソール」を参照してください。");

            static char buf2[64];
            ImGui::InputText("履歴", buf2, IM_COUNTOF(buf2), ImGuiInputTextFlags_CallbackHistory, Funcs::MyCallback);
            ImGui::SameLine(); HelpMarker(
                "ここでは上下キーを押すたびにテキストを置き換えて選択しています。"
                "このコールバックのより実用的な例は「サンプル>コンソール」を参照してください。");

            static char buf3[64];
            static int edit_count = 0;
            ImGui::InputText("編集", buf3, IM_COUNTOF(buf3), ImGuiInputTextFlags_CallbackEdit, Funcs::MyCallback, (void*)&edit_count);
            ImGui::SameLine(); HelpMarker(
                "ここでは編集のたびに先頭文字の大文字小文字を切り替え、編集回数を数えています。");
            ImGui::SameLine(); ImGui::Text("(%d)", edit_count);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("リサイズコールバック"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Resize Callback");
            // To wire InputText() with std::string or any other custom string type,
            // you can use the ImGuiInputTextFlags_CallbackResize flag + create a custom ImGui::InputText() wrapper
            // using your preferred type. See misc/cpp/imgui_stdlib.h for an implementation of this using std::string.
            HelpMarker(
                "ImGuiInputTextFlags_CallbackResize を使って、独自の文字列型を InputText() と連携させています。\n\n"
                "std::string 向けの実装例は misc/cpp/imgui_stdlib.h を参照してください。");
            struct Funcs
            {
                static int MyResizeCallback(ImGuiInputTextCallbackData* data)
                {
                    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
                    {
                        ImVector<char>* my_str = (ImVector<char>*)data->UserData;
                        IM_ASSERT(my_str->begin() == data->Buf);
                        my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
                        data->Buf = my_str->begin();
                    }
                    return 0;
                }

                // Note: Because ImGui:: is a namespace you would typically add your own function into the namespace.
                // For example, you code may declare a function 'ImGui::InputText(const char* label, MyString* my_str)'
                static bool MyInputTextMultiline(const char* label, ImVector<char>* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
                {
                    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
                    return ImGui::InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
                }
            };

            // For this demo we are using ImVector as a string container.
            // Note that because we need to store a terminating zero character, our size/capacity are 1 more
            // than usually reported by a typical string class.
            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
            ImGui::CheckboxFlags("ImGuiInputTextFlags_WordWrap", &flags, ImGuiInputTextFlags_WordWrap);

            static ImVector<char> my_str;
            if (my_str.empty())
                my_str.push_back(0);
            Funcs::MyInputTextMultiline("##MyStr", &my_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
            ImGui::Text("データ: %p\nサイズ: %d\n容量: %d", (void*)my_str.begin(), my_str.size(), my_str.capacity());
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("省略表示と文字揃え"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Eliding, Alignment");
            static char buf1[128] = "/path/to/some/folder/with/long/filename.cpp";
            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_ElideLeft;
            ImGui::CheckboxFlags("ImGuiInputTextFlags_ElideLeft", &flags, ImGuiInputTextFlags_ElideLeft);
            ImGui::InputText("パス", buf1, IM_COUNTOF(buf1), flags);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("その他"))
        {
            IMGUI_DEMO_MARKER("Widgets/Text Input/Miscellaneous");
            static char buf1[16];
            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_EscapeClearsAll;
            ImGui::CheckboxFlags("ImGuiInputTextFlags_EscapeClearsAll", &flags, ImGuiInputTextFlags_EscapeClearsAll);
            ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", &flags, ImGuiInputTextFlags_ReadOnly);
            ImGui::CheckboxFlags("ImGuiInputTextFlags_NoUndoRedo", &flags, ImGuiInputTextFlags_NoUndoRedo);
            ImGui::InputText("こんにちは", buf1, IM_COUNTOF(buf1), flags);
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTooltips()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTooltips()
{
    if (ImGui::TreeNode("ツールチップ"))
    {
        IMGUI_DEMO_MARKER("Widgets/Tooltips");
        // Tooltips are windows following the mouse. They do not take focus away.
        ImGui::SeparatorText("全般");

        // Typical use cases:
        // - Short-form (text only):      SetItemTooltip("Hello");
        // - Short-form (any contents):   if (BeginItemTooltip()) { Text("Hello"); EndTooltip(); }

        // - Full-form (text only):       if (IsItemHovered(...)) { SetTooltip("Hello"); }
        // - Full-form (any contents):    if (IsItemHovered(...) && BeginTooltip()) { Text("Hello"); EndTooltip(); }

        HelpMarker(
            "ツールチップは通常、IsItemHovered() と SetTooltip() を組み合わせて作ります。\n\n"
            "標準的なフラグでこの2つをまとめて行う SetItemTooltip() というヘルパー関数も用意しています。");

        ImVec2 sz = ImVec2(-FLT_MIN, 0.0f);

        ImGui::Button("基本", sz);
        ImGui::SetItemTooltip("これはツールチップです");

        ImGui::Button("凝ったもの", sz);
        if (ImGui::BeginItemTooltip())
        {
            ImGui::Text("これは凝ったツールチップです");
            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            ImGui::PlotLines("曲線", arr, IM_COUNTOF(arr));
            ImGui::Text("Sin(time) = %f", sinf((float)ImGui::GetTime()));
            ImGui::EndTooltip();
        }

        ImGui::SeparatorText("常に表示");

        // Showcase NOT relying on a IsItemHovered() to emit a tooltip.
        // Here the tooltip is always emitted when 'always_on == true'.
        static int always_on = 0;
        ImGui::RadioButton("オフ", &always_on, 0);
        ImGui::SameLine();
        ImGui::RadioButton("常に表示 (簡易)", &always_on, 1);
        ImGui::SameLine();
        ImGui::RadioButton("常に表示 (応用)", &always_on, 2);
        if (always_on == 1)
            ImGui::SetTooltip("ずっとついていきます。");
        else if (always_on == 2 && ImGui::BeginTooltip())
        {
            ImGui::ProgressBar(sinf((float)ImGui::GetTime()) * 0.5f + 0.5f, ImVec2(ImGui::GetFontSize() * 25, 0.0f));
            ImGui::EndTooltip();
        }

        ImGui::SeparatorText("カスタム");

        HelpMarker(
            "IsItemHovered() に ImGuiHoveredFlags_ForTooltip を渡すのが、アプリ全体でツールチップの"
            "表示条件を統一する推奨のやり方です。ただし、特定のツールチップだけ独自のフラグを"
            "使うこともできます。");

        // The following examples are passed for documentation purpose but may not be useful to most users.
        // Passing ImGuiHoveredFlags_ForTooltip to IsItemHovered() will pull ImGuiHoveredFlags flags values from
        // 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav' depending on whether mouse or keyboard/gamepad is being used.
        // With default settings, ImGuiHoveredFlags_ForTooltip is equivalent to ImGuiHoveredFlags_DelayShort + ImGuiHoveredFlags_Stationary.
        ImGui::Button("手動", sz);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            ImGui::SetTooltip("これは手動で表示したツールチップです。");

        ImGui::Button("DelayNone", sz);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
            ImGui::SetTooltip("これは遅延なしのツールチップです。");

        ImGui::Button("DelayShort", sz);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
            ImGui::SetTooltip("これは短い遅延 (%0.2f 秒) 付きのツールチップです。", ImGui::GetStyle().HoverDelayShort);

        ImGui::Button("DelayLong", sz);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
            ImGui::SetTooltip("これは長い遅延 (%0.2f 秒) 付きのツールチップです。", ImGui::GetStyle().HoverDelayNormal);

        ImGui::Button("Stationary", sz);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary))
            ImGui::SetTooltip("これはマウスが静止してから表示されるツールチップです。");

        // Using ImGuiHoveredFlags_ForTooltip will pull flags from 'style.HoverFlagsForTooltipMouse' or 'style.HoverFlagsForTooltipNav',
        // which default value include the ImGuiHoveredFlags_AllowWhenDisabled flag.
        ImGui::BeginDisabled();
        ImGui::Button("無効な項目", sz);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            ImGui::SetTooltip("これは無効な項目のツールチップです。");
        ImGui::EndDisabled();

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsTreeNodes()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsTreeNodes()
{
    if (ImGui::TreeNode("ツリーノード"))
    {
        IMGUI_DEMO_MARKER("Widgets/Tree Nodes");
        // See see "Examples -> Property Editor" (ShowExampleAppPropertyEditor() function) for a fancier, data-driven tree.
        if (ImGui::TreeNode("基本的なツリー"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tree Nodes/Basic Trees");
            for (int i = 0; i < 5; i++)
            {
                // Use SetNextItemOpen() so set the default state of a node to be open. We could
                // also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (i == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                // Here we use PushID() to generate a unique base ID, and then the "" used as TreeNode id won't conflict.
                // An alternative to using 'PushID() + TreeNode("", ...)' to generate a unique ID is to use 'TreeNode((void*)(intptr_t)i, ...)',
                // aka generate a dummy pointer-sized value to be hashed. The demo below uses that technique. Both are fine.
                ImGui::PushID(i);
                if (ImGui::TreeNode("", "子 %d", i))
                {
                    ImGui::Text("なんとかかんとか");
                    ImGui::SameLine();
                    if (ImGui::SmallButton("ボタン")) {}
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("階層の線"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tree Nodes/Hierarchy Lines");
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_DefaultOpen;
            HelpMarker("DrawLinesXXX の既定のオプションは style.TreeLinesFlags に保存されています");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_DrawLinesNone", &base_flags, ImGuiTreeNodeFlags_DrawLinesNone);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_DrawLinesFull", &base_flags, ImGuiTreeNodeFlags_DrawLinesFull);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_DrawLinesToNodes", &base_flags, ImGuiTreeNodeFlags_DrawLinesToNodes);

            if (ImGui::TreeNodeEx("親", base_flags))
            {
                if (ImGui::TreeNodeEx("子 1", base_flags))
                {
                    ImGui::Button("子 1 のボタン");
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("子 2", base_flags))
                {
                    ImGui::Button("子 2 のボタン");
                    ImGui::TreePop();
                }
                ImGui::Text("残りの内容");
                ImGui::Text("残りの内容");
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("大きなツリーのクリッピング"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tree Nodes/Clipping Large Trees");
            ImGui::TextWrapped(
                "- ImGuiListClipper をツリーに使うのは、配列やグリッドに使うほど簡単ではありません。\n"
                "- そのやり方の例は「デモ->サンプル->プロパティエディター」を参照してください。\n"
                "- 議論は #3823 を参照してください");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("選択できるノード"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tree Nodes/Selectable Nodes");
            HelpMarker(
                "自前で実装した選択可能なノードです。\n"
                "クリックで選択、Ctrl+クリックで選択の切り替え、矢印のクリックかダブルクリックで開閉します。\n\n"
                "より高度な複数選択機能には複数選択 API も使えます (「デモ->ウィジェット->選択状態と複数選択」を参照)。");

            // Hold in 'selection_mask' a simple representation of what may be user-side selection state.
            // - You may retain selection state inside or outside your objects in whatever format you see fit.
            //   You may use ImGuiSelectionBasicStorage which is conceptually close to a set<> of identifiers.
            // - We record which node was clicked and then apply selection at the end of the loop.
            // - This is a manual and simplified reimplementation of multi-selection, which the full
            //   BeginMultiSelect() API implements better, but which is not trivial to wire for trees.
            static int selection_mask = 0x00;
            int node_clicked_idx = -1;
            for (int node_n = 0; node_n < 6; node_n++)
            {
                // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
                // To alter selection we use if 'IsItemClicked() && !IsItemToggledOpen()', so clicking on an arrow doesn't alter selection.
                // In a BeginMultiSelect()/EndMultiSelect() we could use IsItemToggledSelection() but here we reimplement and use our own logic.
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (selection_mask & (1 << node_n))
                    flags |= ImGuiTreeNodeFlags_Selected;

                bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)node_n, flags, "選択できるノード %d", node_n);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    node_clicked_idx = node_n;
                if (is_open)
                {
                    ImGui::BulletText("<ここにノードの内容>");
                    ImGui::TreePop();
                }
            }
            if (node_clicked_idx != -1)
            {
                // Update selection state (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
                if (ImGui::GetIO().KeyCtrl)
                    selection_mask ^= (1 << node_clicked_idx);          // Ctrl+Click to toggle
                else //if (!(selection_mask & (1 << node_clicked_idx))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
                    selection_mask = (1 << node_clicked_idx);           // Click to single-select
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("応用"))
        {
            IMGUI_DEMO_MARKER("Widgets/Tree Nodes/Advanced");
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            static bool align_label_with_current_x_position = false;
            static bool use_drag_and_drop = false;
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnArrow", &base_flags, ImGuiTreeNodeFlags_OpenOnArrow);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnDoubleClick", &base_flags, ImGuiTreeNodeFlags_OpenOnDoubleClick);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanAvailWidth", &base_flags, ImGuiTreeNodeFlags_SpanAvailWidth); ImGui::SameLine(); HelpMarker("ノードの後ろに他の項目を並べられるようにする代わりに、当たり判定を利用可能な幅いっぱいに広げます。");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanFullWidth", &base_flags, ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanLabelWidth", &base_flags, ImGuiTreeNodeFlags_SpanLabelWidth); ImGui::SameLine(); HelpMarker("当たり判定をテキストラベルと少しの余白だけに縮めます。");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanAllColumns", &base_flags, ImGuiTreeNodeFlags_SpanAllColumns); ImGui::SameLine(); HelpMarker("テーブル内でのみ使用します。");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_AllowOverlap", &base_flags, ImGuiTreeNodeFlags_AllowOverlap);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_Framed", &base_flags, ImGuiTreeNodeFlags_Framed); ImGui::SameLine(); HelpMarker("背景付きの枠を描きます (CollapsingHeader などで使用)");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_FramePadding", &base_flags, ImGuiTreeNodeFlags_FramePadding);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_NavLeftJumpsToParent", &base_flags, ImGuiTreeNodeFlags_NavLeftJumpsToParent);

            HelpMarker("DrawLinesXXX の既定のオプションは style.TreeLinesFlags に保存されています");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_DrawLinesNone", &base_flags, ImGuiTreeNodeFlags_DrawLinesNone);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_DrawLinesFull", &base_flags, ImGuiTreeNodeFlags_DrawLinesFull);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_DrawLinesToNodes", &base_flags, ImGuiTreeNodeFlags_DrawLinesToNodes);

            ImGui::Checkbox("ラベルを現在の X 位置に揃える", &align_label_with_current_x_position);
            ImGui::Checkbox("ツリーノードをドラッグ&ドロップ元にする", &use_drag_and_drop);
            if (align_label_with_current_x_position)
                ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

            for (int node_n = 0; node_n < 6; node_n++)
            {
                ImGuiTreeNodeFlags node_flags = base_flags;
                if (node_n < 3)
                {
                    // Items 0..2 are Tree Node
                    bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)node_n, node_flags, "選択できるノード %d", node_n);
                    if (use_drag_and_drop && ImGui::BeginDragDropSource())
                    {
                        ImGui::SetDragDropPayload("MY_TREENODE_PAYLOAD_TYPE", NULL, 0);
                        ImGui::Text("これはドラッグ&ドロップ元です");
                        ImGui::EndDragDropSource();
                    }
                    if (node_n == 2 && (base_flags & ImGuiTreeNodeFlags_SpanLabelWidth))
                    {
                        // Item 2 has an additional inline button to help demonstrate SpanLabelWidth.
                        ImGui::SameLine();
                        if (ImGui::SmallButton("ボタン")) {}
                    }
                    if (is_open)
                    {
                        ImGui::BulletText("なんとかかんとか\nなんとかかんとか");
                        ImGui::SameLine();
                        ImGui::SmallButton("ボタン");
                        ImGui::TreePop();
                    }
                }
                else
                {
                    // Items 3..5 are Tree Leaves
                    // The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
                    // use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
                    ImGui::TreeNodeEx((void*)(intptr_t)node_n, node_flags, "選択できる葉ノード %d", node_n);
                    if (use_drag_and_drop && ImGui::BeginDragDropSource())
                    {
                        ImGui::SetDragDropPayload("MY_TREENODE_PAYLOAD_TYPE", NULL, 0);
                        ImGui::Text("これはドラッグ&ドロップ元です");
                        ImGui::EndDragDropSource();
                    }
                }
            }
            if (align_label_with_current_x_position)
                ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgetsVerticalSliders()
//-----------------------------------------------------------------------------

static void DemoWindowWidgetsVerticalSliders()
{
    if (ImGui::TreeNode("縦方向のスライダー"))
    {
        IMGUI_DEMO_MARKER("Widgets/Vertical Sliders");
        const float spacing = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));

        static int int_value = 0;
        ImGui::VSliderInt("##int", ImVec2(18, 160), &int_value, 0, 5);
        ImGui::SameLine();

        static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
        ImGui::PushID("set1");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(i / 7.0f, 0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(i / 7.0f, 0.9f, 0.9f));
            ImGui::VSliderFloat("##v", ImVec2(18, 160), &values[i], 0.0f, 1.0f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                ImGui::SetTooltip("%.3f", values[i]);
            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID("set2");
        static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
        const int rows = 3;
        const ImVec2 small_slider_size(18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
        for (int nx = 0; nx < 4; nx++)
        {
            if (nx > 0) ImGui::SameLine();
            ImGui::BeginGroup();
            for (int ny = 0; ny < rows; ny++)
            {
                ImGui::PushID(nx * rows + ny);
                ImGui::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
                if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                    ImGui::SetTooltip("%.3f", values2[nx]);
                ImGui::PopID();
            }
            ImGui::EndGroup();
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID("set3");
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
            ImGui::VSliderFloat("##v", ImVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\n秒");
            ImGui::PopStyleVar();
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::PopStyleVar();
        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowWidgets()
//-----------------------------------------------------------------------------

static void DemoWindowWidgets(ImGuiDemoWindowData* demo_data)
{
    //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader("ウィジェット"))
        return;
    // IMGUI_DEMO_MARKER("Widgets");

    const bool disable_all = demo_data->DisableSections; // The Checkbox for that is inside the "Disabled" section at the bottom
    if (disable_all)
        ImGui::BeginDisabled();

    DemoWindowWidgetsBasic();
    DemoWindowWidgetsBullets();
    DemoWindowWidgetsCollapsingHeaders();
    DemoWindowWidgetsComboBoxes();
    DemoWindowWidgetsColorAndPickers();
    DemoWindowWidgetsDataTypes();

    if (disable_all)
        ImGui::EndDisabled();
    DemoWindowWidgetsDisableBlocks(demo_data);
    if (disable_all)
        ImGui::BeginDisabled();

    DemoWindowWidgetsDragAndDrop();
    DemoWindowWidgetsDragsAndSliders();
    DemoWindowWidgetsFonts();
    DemoWindowWidgetsImages();
    DemoWindowWidgetsListBoxes();
    DemoWindowWidgetsMultiComponents();
    DemoWindowWidgetsPlotting();
    DemoWindowWidgetsProgressBars();
    DemoWindowWidgetsQueryingStatuses();
    DemoWindowWidgetsSelectables();
    DemoWindowWidgetsSelectionAndMultiSelect(demo_data);
    DemoWindowWidgetsTabs();
    DemoWindowWidgetsText();
    DemoWindowWidgetsTextFilter();
    DemoWindowWidgetsTextInput();
    DemoWindowWidgetsTooltips();
    DemoWindowWidgetsTreeNodes();
    DemoWindowWidgetsVerticalSliders();

    if (disable_all)
        ImGui::EndDisabled();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowLayout()
//-----------------------------------------------------------------------------

static void DemoWindowLayout()
{
    if (!ImGui::CollapsingHeader("レイアウトとスクロール"))
        return;

    if (ImGui::TreeNode("子ウィンドウ"))
    {
        IMGUI_DEMO_MARKER("Layout/Child windows");
        ImGui::SeparatorText("子ウィンドウ");

        HelpMarker("子ウィンドウを使うと、親ウィンドウの中に独立したスクロール/クリッピング領域を作れます。");
        static bool disable_mouse_wheel = false;
        static bool disable_menu = false;
        ImGui::Checkbox("マウスホイールを無効にする", &disable_mouse_wheel);
        ImGui::Checkbox("メニューを無効にする", &disable_menu);

        // Child 1: no border, enable horizontal scrollbar
        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
            if (disable_mouse_wheel)
                window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
            ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), ImGuiChildFlags_None, window_flags);
            for (int i = 0; i < 100; i++)
                ImGui::Text("%04d: スクロールできる領域", i);
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Child 2: rounded border
        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
            if (disable_mouse_wheel)
                window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
            if (!disable_menu)
                window_flags |= ImGuiWindowFlags_MenuBar;
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("ChildR", ImVec2(0, 260), ImGuiChildFlags_Borders, window_flags);
            if (!disable_menu && ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("メニュー"))
                {
                    ShowExampleMenuFile();
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            if (ImGui::BeginTable("split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
            {
                for (int i = 0; i < 100; i++)
                {
                    char buf[32];
                    sprintf(buf, "%03d", i);
                    ImGui::TableNextColumn();
                    ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        // Child 3: manual-resize
        ImGui::SeparatorText("手動リサイズ");
        {
            HelpMarker("下の境界をドラッグするとサイズを変えられます。下の境界をダブルクリックすると内容の高さに自動で合わせます。");
            //if (ImGui::Button("Set Height to 200"))
            //    ImGui::SetNextWindowSize(ImVec2(-FLT_MIN, 200.0f));

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
            if (ImGui::BeginChild("ResizableChild", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
                for (int n = 0; n < 10; n++)
                    ImGui::Text("行 %04d", n);
            ImGui::PopStyleColor();
            ImGui::EndChild();
        }

        // Child 4: auto-resizing height with a limit
        ImGui::SeparatorText("制約付きの自動リサイズ");
        {
            static int draw_lines = 3;
            static int max_height_in_lines = 10;
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::DragInt("行数", &draw_lines, 0.2f);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::DragInt("最大の高さ (行数)", &max_height_in_lines, 0.2f);

            ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, ImGui::GetTextLineHeightWithSpacing() * max_height_in_lines));
            if (ImGui::BeginChild("ConstrainedChild", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
                for (int n = 0; n < draw_lines; n++)
                    ImGui::Text("行 %04d", n);
            ImGui::EndChild();
        }

        ImGui::SeparatorText("その他/応用");

        // Demonstrate a few extra things
        // - Changing ImGuiCol_ChildBg (which is transparent black in default styles)
        // - Using SetCursorPos() to position child window (the child window is an item from the POV of parent window)
        //   You can also call SetNextWindowPos() to position the child window. The parent window will effectively
        //   layout from this position.
        // - Using ImGui::GetItemRectMin/Max() to query the "item" state (because the child window is an item from
        //   the POV of the parent window). See 'Demo->Querying Status (Edited/Active/Hovered etc.)' for details.
        {
            static int offset_x = 0;
            static bool override_bg_color = true;
            static ImGuiChildFlags child_flags = ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::DragInt("X オフセット", &offset_x, 1.0f, -1000, 1000);
            ImGui::Checkbox("ChildBg の色を上書きする", &override_bg_color);
            ImGui::CheckboxFlags("ImGuiChildFlags_Borders", &child_flags, ImGuiChildFlags_Borders);
            ImGui::CheckboxFlags("ImGuiChildFlags_AlwaysUseWindowPadding", &child_flags, ImGuiChildFlags_AlwaysUseWindowPadding);
            ImGui::CheckboxFlags("ImGuiChildFlags_ResizeX", &child_flags, ImGuiChildFlags_ResizeX);
            ImGui::CheckboxFlags("ImGuiChildFlags_ResizeY", &child_flags, ImGuiChildFlags_ResizeY);
            ImGui::CheckboxFlags("ImGuiChildFlags_FrameStyle", &child_flags, ImGuiChildFlags_FrameStyle);
            ImGui::SameLine(); HelpMarker("子ウィンドウを枠付き項目のように表示します: ChildBg, ChildRounding, ChildBorderSize, WindowPadding ではなく FrameBg, FrameRounding, FrameBorderSize, FramePadding を使います。");
            if (child_flags & ImGuiChildFlags_FrameStyle)
                override_bg_color = false;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (float)offset_x);
            if (override_bg_color)
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
            ImGui::BeginChild("Red", ImVec2(200, 100), child_flags, ImGuiWindowFlags_None);
            if (override_bg_color)
                ImGui::PopStyleColor();

            for (int n = 0; n < 50; n++)
                ImGui::Text("テスト %d", n);
            ImGui::EndChild();
            bool child_is_hovered = ImGui::IsItemHovered();
            ImVec2 child_rect_min = ImGui::GetItemRectMin();
            ImVec2 child_rect_max = ImGui::GetItemRectMax();
            ImGui::Text("ホバー中: %d", child_is_hovered);
            ImGui::Text("子ウィンドウの矩形: (%.0f,%.0f) (%.0f,%.0f)", child_rect_min.x, child_rect_min.y, child_rect_max.x, child_rect_max.y);
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("ウィジェットの幅"))
    {
        IMGUI_DEMO_MARKER("Layout/Widgets Width");
        static float f = 0.0f;
        static bool show_indented_items = true;
        ImGui::Checkbox("インデントした項目も表示する", &show_indented_items);

        // Use SetNextItemWidth() to set the width of a single upcoming item.
        // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
        // In real code use you'll probably want to choose width values that are proportional to your font size
        // e.g. Using '20.0f * GetFontSize()' as width instead of '200.0f', etc.

        ImGui::Text("SetNextItemWidth/PushItemWidth(100)");
        ImGui::SameLine(); HelpMarker("固定幅。");
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("float##1b", &f);
        if (show_indented_items)
        {
            ImGui::Indent();
            ImGui::DragFloat("float (インデント)##1b", &f);
            ImGui::Unindent();
        }
        ImGui::PopItemWidth();

        ImGui::Text("SetNextItemWidth/PushItemWidth(-100)");
        ImGui::SameLine(); HelpMarker("右端から 100 引いた位置に揃える");
        ImGui::PushItemWidth(-100);
        ImGui::DragFloat("float##2a", &f);
        if (show_indented_items)
        {
            ImGui::Indent();
            ImGui::DragFloat("float (インデント)##2b", &f);
            ImGui::Unindent();
        }
        ImGui::PopItemWidth();

        ImGui::Text("SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)");
        ImGui::SameLine(); HelpMarker("利用できる幅の半分。\n(~ 右端 - カーソル位置)\n(カラム内でも機能します)");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        ImGui::DragFloat("float##3a", &f);
        if (show_indented_items)
        {
            ImGui::Indent();
            ImGui::DragFloat("float (インデント)##3b", &f);
            ImGui::Unindent();
        }
        ImGui::PopItemWidth();

        ImGui::Text("SetNextItemWidth/PushItemWidth(-GetContentRegionAvail().x * 0.5f)");
        ImGui::SameLine(); HelpMarker("右端から半分引いた位置に揃える");
        ImGui::PushItemWidth(-ImGui::GetContentRegionAvail().x * 0.5f);
        ImGui::DragFloat("float##4a", &f);
        if (show_indented_items)
        {
            ImGui::Indent();
            ImGui::DragFloat("float (インデント)##4b", &f);
            ImGui::Unindent();
        }
        ImGui::PopItemWidth();

        ImGui::Text("SetNextItemWidth/PushItemWidth(-Min(GetContentRegionAvail().x * 0.40f, GetFontSize() * 12))");
        ImGui::PushItemWidth(-IM_MIN(ImGui::GetFontSize() * 12, ImGui::GetContentRegionAvail().x * 0.40f));
        ImGui::DragFloat("float##5a", &f);
        if (show_indented_items)
        {
            ImGui::Indent();
            ImGui::DragFloat("float (インデント)##5b", &f);
            ImGui::Unindent();
        }
        ImGui::PopItemWidth();

        // Demonstrate using PushItemWidth to surround three items.
        // Calling SetNextItemWidth() before each of them would have the same effect.
        ImGui::Text("SetNextItemWidth/PushItemWidth(-FLT_MIN)");
        ImGui::SameLine(); HelpMarker("右端に揃える");
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::DragFloat("##float6a", &f);
        if (show_indented_items)
        {
            ImGui::Indent();
            ImGui::DragFloat("float (インデント)##6b", &f);
            ImGui::Unindent();
        }
        ImGui::PopItemWidth();

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("基本的な横並びレイアウト"))
    {
        IMGUI_DEMO_MARKER("Layout/Basic Horizontal Layout");
        ImGui::TextWrapped("(ImGui::SameLine() を使うと、直前の項目の右に続けて項目を並べられます)");

        // Text
        IMGUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine");
        ImGui::Text("2つの項目: こんにちは"); ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "セーラーさん");

        // Adjust spacing
        ImGui::Text("間隔を広く: こんにちは"); ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "セーラーさん");

        // Button
        ImGui::AlignTextToFramePadding();
        ImGui::Text("通常のボタン"); ImGui::SameLine();
        ImGui::Button("バナナ"); ImGui::SameLine();
        ImGui::Button("リンゴ"); ImGui::SameLine();
        ImGui::Button("ヤグルマギク");

        // Button
        ImGui::Text("小さいボタン"); ImGui::SameLine();
        ImGui::SmallButton("こんなふうに"); ImGui::SameLine();
        ImGui::Text("テキストの中に収まります。");

        // Aligned to arbitrary position. Easy/cheap column.
        IMGUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (with offset)");
        ImGui::Text("位置揃え");
        ImGui::SameLine(150); ImGui::Text("x=150");
        ImGui::SameLine(300); ImGui::Text("x=300");
        ImGui::Text("位置揃え");
        ImGui::SameLine(150); ImGui::SmallButton("x=150");
        ImGui::SameLine(300); ImGui::SmallButton("x=300");

        // Checkbox
        IMGUI_DEMO_MARKER("Layout/Basic Horizontal Layout/SameLine (more)");
        static bool c1 = false, c2 = false, c3 = false, c4 = false;
        ImGui::Checkbox("私の", &c1); ImGui::SameLine();
        ImGui::Checkbox("仕立て屋は", &c2); ImGui::SameLine();
        ImGui::Checkbox("とても", &c3); ImGui::SameLine();
        ImGui::Checkbox("裕福です", &c4);

        // Various
        static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
        ImGui::PushItemWidth(ImGui::CalcTextSize("AAAAAAA").x);
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
        static int item = -1;
        ImGui::Combo("コンボ", &item, items, IM_COUNTOF(items)); ImGui::SameLine();
        ImGui::SliderFloat("X", &f0, 0.0f, 5.0f); ImGui::SameLine();
        ImGui::SliderFloat("Y", &f1, 0.0f, 5.0f); ImGui::SameLine();
        ImGui::SliderFloat("Z", &f2, 0.0f, 5.0f);

        ImGui::Text("リスト:");
        static int selection[4] = { 0, 1, 2, 3 };
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::ListBox("", &selection[i], items, IM_COUNTOF(items));
            ImGui::PopID();
            //ImGui::SetItemTooltip("ListBox %d hovered", i);
        }
        ImGui::PopItemWidth();

        // Dummy
        IMGUI_DEMO_MARKER("Layout/Basic Horizontal Layout/Dummy");
        ImVec2 button_sz(40, 40);
        ImGui::Button("A", button_sz); ImGui::SameLine();
        ImGui::Dummy(button_sz); ImGui::SameLine();
        ImGui::Button("B", button_sz);

        // Manually wrapping
        // (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
        IMGUI_DEMO_MARKER("Layout/Basic Horizontal Layout/Manual wrapping");
        ImGui::Text("手動での折り返し:");
        ImGuiStyle& style = ImGui::GetStyle();
        int buttons_count = 20;
        float window_visible_x2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
        for (int n = 0; n < buttons_count; n++)
        {
            ImGui::PushID(n);
            ImGui::Button("箱", button_sz);
            float last_button_x2 = ImGui::GetItemRectMax().x;
            float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
            if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
                ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("グループ"))
    {
        IMGUI_DEMO_MARKER("Layout/Groups");
        HelpMarker(
            "BeginGroup() は基本的に、新しい行の水平位置を固定します。"
            "EndGroup() はグループ全体をひとまとめにするので、IsItemHovered()/IsItemActive() や SameLine() などの"
            "「項目」向けの関数をグループ全体に対して使えるようになります。");
        ImGui::BeginGroup();
        {
            ImGui::BeginGroup();
            ImGui::Button("AAA");
            ImGui::SameLine();
            ImGui::Button("BBB");
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Button("CCC");
            ImGui::Button("DDD");
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::Button("EEE");
            ImGui::EndGroup();
            ImGui::SetItemTooltip("1つ目のグループがホバーされました");
        }
        // Capture the group size and create widgets using the same size
        ImVec2 size = ImGui::GetItemRectSize();
        const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
        ImGui::PlotHistogram("##values", values, IM_COUNTOF(values), 0, NULL, 0.0f, 1.0f, size);

        ImGui::Button("アクション", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        ImGui::SameLine();
        ImGui::Button("リアクション", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::Button("流行り言葉を\n活用する", size);
        ImGui::SameLine();

        if (ImGui::BeginListBox("リスト", size))
        {
            ImGui::Selectable("選択済み", true);
            ImGui::Selectable("未選択", false);
            ImGui::EndListBox();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("テキストのベースライン揃え"))
    {
        IMGUI_DEMO_MARKER("Layout/Text Baseline Alignment");
        {
            ImGui::BulletText("テキストのベースライン:");
            ImGui::SameLine(); HelpMarker(
                "テキストをウィジェットと揃えるために適用される垂直方向の位置合わせを試しています。"
                "テキストや「小さな」ウィジェットだけの行は、枠付きウィジェットのある行より高さが小さくなります。");
            ImGui::Indent();

            ImGui::Text("NG なんとかかんとか"); ImGui::SameLine();
            ImGui::Button("枠付きの項目"); ImGui::SameLine();
            HelpMarker("ボタンのベースラインがテキストとずれて見えます..");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            // (because we don't know what's coming after the Text() statement, we need to move the text baseline
            // down by FramePadding.y ahead of time)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("OK なんとかかんとか"); ImGui::SameLine();
            ImGui::Button("枠付きの項目##2"); ImGui::SameLine();
            HelpMarker("AlignTextToFramePadding() を呼んで、テキストのベースラインを +FramePadding.y 分だけ揃えています");

            // SmallButton() uses the same vertical padding as Text
            ImGui::Button("TEST##1"); ImGui::SameLine();
            ImGui::Text("TEST"); ImGui::SameLine();
            ImGui::SmallButton("TEST##2");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            ImGui::AlignTextToFramePadding();
            ImGui::Text("枠付き項目に揃えたテキスト"); ImGui::SameLine();
            ImGui::Button("項目##1"); ImGui::SameLine();
            ImGui::Text("項目"); ImGui::SameLine();
            ImGui::SmallButton("項目##2"); ImGui::SameLine();
            ImGui::Button("項目##3");

            ImGui::Unindent();
        }

        ImGui::Spacing();

        {
            ImGui::BulletText("複数行のテキスト:");
            ImGui::Indent();
            ImGui::Text("いち\nに\nさん"); ImGui::SameLine();
            ImGui::Text("こんにちは\n世界"); ImGui::SameLine();
            ImGui::Text("バナナ");

            ImGui::Text("バナナ"); ImGui::SameLine();
            ImGui::Text("こんにちは\n世界"); ImGui::SameLine();
            ImGui::Text("いち\nに\nさん");

            ImGui::Button("HOP##1"); ImGui::SameLine();
            ImGui::Text("バナナ"); ImGui::SameLine();
            ImGui::Text("こんにちは\n世界"); ImGui::SameLine();
            ImGui::Text("バナナ");

            ImGui::Button("HOP##2"); ImGui::SameLine();
            ImGui::Text("こんにちは\n世界"); ImGui::SameLine();
            ImGui::Text("バナナ");
            ImGui::Unindent();
        }

        ImGui::Spacing();

        {
            ImGui::BulletText("その他の項目:");
            ImGui::Indent();

            // SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button.
            ImGui::Button("80x80", ImVec2(80, 80));
            ImGui::SameLine();
            ImGui::Button("50x50", ImVec2(50, 50));
            ImGui::SameLine();
            ImGui::Button("Button()");
            ImGui::SameLine();
            ImGui::SmallButton("SmallButton()");

            // Tree
            // (here the node appears after a button and has odd intent, so we use ImGuiTreeNodeFlags_DrawLinesNone to disable hierarchy outline)
            const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::Button("ボタン##1"); // Will make line higher
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::TreeNodeEx("ノード##1", ImGuiTreeNodeFlags_DrawLinesNone))
            {
                // Placeholder tree data
                for (int i = 0; i < 6; i++)
                    ImGui::BulletText("項目 %d..", i);
                ImGui::TreePop();
            }

            const float padding = (float)(int)(ImGui::GetFontSize() * 1.20f); // Large padding
            ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, padding);
            ImGui::Button("ボタン##2");
            ImGui::PopStyleVar();
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::TreeNodeEx("ノード##2", ImGuiTreeNodeFlags_DrawLinesNone))
                ImGui::TreePop();

            // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget.
            // Otherwise you can use SmallButton() (smaller fit).
            ImGui::AlignTextToFramePadding();

            // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add
            // other contents "inside" the node.
            bool node_open = ImGui::TreeNode("ノード##3");
            ImGui::SameLine(0.0f, spacing); ImGui::Button("ボタン##3");
            if (node_open)
            {
                // Placeholder tree data
                for (int i = 0; i < 6; i++)
                    ImGui::BulletText("項目 %d..", i);
                ImGui::TreePop();
            }

            // Bullet
            ImGui::Button("ボタン##4");
            ImGui::SameLine(0.0f, spacing);
            ImGui::BulletText("箇条書きテキスト");

            ImGui::AlignTextToFramePadding();
            ImGui::BulletText("ノード");
            ImGui::SameLine(0.0f, spacing); ImGui::Button("ボタン##5");
            ImGui::Unindent();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("スクロール"))
    {
        IMGUI_DEMO_MARKER("Layout/Scrolling/Vertical");
        // Vertical scroll functions
        HelpMarker("SetScrollHereY() や SetScrollFromPosY() を使うと、指定した垂直位置までスクロールできます。");

        static int track_item = 50;
        static bool enable_track = true;
        static bool enable_extra_decorations = false;
        static float scroll_to_off_px = 0.0f;
        static float scroll_to_pos_px = 200.0f;

        ImGui::Checkbox("装飾", &enable_extra_decorations);

        ImGui::PushItemWidth(ImGui::GetFontSize() * 10);
        enable_track |= ImGui::DragInt("##item", &track_item, 0.25f, 0, 99, "項目 = %d");
        ImGui::SameLine();
        ImGui::Checkbox("追従", &enable_track);

        bool scroll_to_off = ImGui::DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");
        ImGui::SameLine();
        scroll_to_off |= ImGui::Button("オフセットへスクロール");

        bool scroll_to_pos = ImGui::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
        ImGui::SameLine();
        scroll_to_pos |= ImGui::Button("指定位置へスクロール");
        ImGui::PopItemWidth();

        if (scroll_to_off || scroll_to_pos)
            enable_track = false;

        ImGuiStyle& style = ImGui::GetStyle();
        float child_w = (ImGui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
        if (child_w < 1.0f)
            child_w = 1.0f;
        ImGui::PushID("##VerticalScrolling");
        for (int i = 0; i < 5; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::BeginGroup();
            const char* names[] = { "上端", "25%", "中央", "75%", "下端" };
            ImGui::TextUnformatted(names[i]);

            const ImGuiWindowFlags child_flags = enable_extra_decorations ? ImGuiWindowFlags_MenuBar : 0;
            const ImGuiID child_id = ImGui::GetID((void*)(intptr_t)i);
            const bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, 200.0f), ImGuiChildFlags_Borders, child_flags);
            if (ImGui::BeginMenuBar())
            {
                ImGui::TextUnformatted("abc");
                ImGui::EndMenuBar();
            }
            if (scroll_to_off)
                ImGui::SetScrollY(scroll_to_off_px);
            if (scroll_to_pos)
                ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + scroll_to_pos_px, i * 0.25f);
            if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (enable_track && item == track_item)
                    {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "項目 %d", item);
                        ImGui::SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                    }
                    else
                    {
                        ImGui::Text("項目 %d", item);
                    }
                }
            }
            float scroll_y = ImGui::GetScrollY();
            float scroll_max_y = ImGui::GetScrollMaxY();
            ImGui::EndChild();
            ImGui::Text("%.0f/%.0f", scroll_y, scroll_max_y);
            ImGui::EndGroup();
        }
        ImGui::PopID();

        // Horizontal scroll functions
        IMGUI_DEMO_MARKER("Layout/Scrolling/Horizontal");
        ImGui::Spacing();
        HelpMarker(
            "SetScrollHereX() や SetScrollFromPosX() を使うと、指定した水平位置までスクロールできます。\n\n"
            "ほとんどのウィンドウのクリッピング矩形は左右の WindowPadding の半分を隠すため、"
            "SetScrollFromPosX(+1) では通常テキストが切れてしまいますが、"
            "対応する SetScrollFromPosY(+1) ではそうなりません。");
        ImGui::PushID("##HorizontalScrolling");
        for (int i = 0; i < 5; i++)
        {
            float child_height = ImGui::GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
            ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar | (enable_extra_decorations ? ImGuiWindowFlags_AlwaysVerticalScrollbar : 0);
            ImGuiID child_id = ImGui::GetID((void*)(intptr_t)i);
            bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(-100, child_height), ImGuiChildFlags_Borders, child_flags);
            if (scroll_to_off)
                ImGui::SetScrollX(scroll_to_off_px);
            if (scroll_to_pos)
                ImGui::SetScrollFromPosX(ImGui::GetCursorStartPos().x + scroll_to_pos_px, i * 0.25f);
            if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (item > 0)
                        ImGui::SameLine();
                    if (enable_track && item == track_item)
                    {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "項目 %d", item);
                        ImGui::SetScrollHereX(i * 0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
                    }
                    else
                    {
                        ImGui::Text("項目 %d", item);
                    }
                }
            }
            float scroll_x = ImGui::GetScrollX();
            float scroll_max_x = ImGui::GetScrollMaxX();
            ImGui::EndChild();
            ImGui::SameLine();
            const char* names[] = { "左端", "25%", "中央", "75%", "右端" };
            ImGui::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
            ImGui::Spacing();
        }
        ImGui::PopID();

        // Miscellaneous Horizontal Scrolling Demo
        IMGUI_DEMO_MARKER("Layout/Scrolling/Horizontal (more)");
        HelpMarker(
            "ウィンドウの横スクロールは ImGuiWindowFlags_HorizontalScrollbar フラグで有効になります。\n\n"
            "Begin() の前に SetNextWindowContentWidth() を呼んで、コンテンツ幅を明示的に指定したい場合もあるでしょう。");
        static int lines = 7;
        ImGui::SliderInt("行数", &lines, 1, 15);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
        ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30);
        ImGui::BeginChild("scrolling", scrolling_child_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        for (int line = 0; line < lines; line++)
        {
            // Display random stuff. For the sake of this trivial demo we are using basic Button() + SameLine()
            // If you want to create your own time line for a real application you may be better off manipulating
            // the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets
            // yourself. You may also want to use the lower-level ImDrawList API.
            int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
            for (int n = 0; n < num_buttons; n++)
            {
                if (n > 0) ImGui::SameLine();
                ImGui::PushID(n + line * 1000);
                char num_buf[16];
                sprintf(num_buf, "%d", n);
                const char* label = (!(n % 15)) ? "FizzBuzz" : (!(n % 3)) ? "Fizz" : (!(n % 5)) ? "Buzz" : num_buf;
                float hue = n * 0.05f;
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
                ImGui::Button(label, ImVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
        }
        float scroll_x = ImGui::GetScrollX();
        float scroll_max_x = ImGui::GetScrollMaxX();
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        float scroll_x_delta = 0.0f;
        ImGui::SmallButton("<<");
        if (ImGui::IsItemActive())
            scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f;
        ImGui::SameLine();
        ImGui::Text("コードからスクロール"); ImGui::SameLine();
        ImGui::SmallButton(">>");
        if (ImGui::IsItemActive())
            scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f;
        ImGui::SameLine();
        ImGui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
        if (scroll_x_delta != 0.0f)
        {
            // Demonstrate a trick: you can use Begin to set yourself in the context of another window
            // (here we are already out of your child window)
            ImGui::BeginChild("scrolling");
            ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
            ImGui::EndChild();
        }
        ImGui::Spacing();

        static bool show_horizontal_contents_size_demo_window = false;
        ImGui::Checkbox("横方向のコンテンツサイズのデモウィンドウを表示", &show_horizontal_contents_size_demo_window);

        if (show_horizontal_contents_size_demo_window)
        {
            static bool show_h_scrollbar = true;
            static bool show_button = true;
            static bool show_tree_nodes = true;
            static bool show_text_wrapped = false;
            static bool show_columns = true;
            static bool show_tab_bar = true;
            static bool show_child = false;
            static bool explicit_content_size = false;
            static float contents_size_x = 300.0f;
            if (explicit_content_size)
                ImGui::SetNextWindowContentSize(ImVec2(contents_size_x, 0.0f));
            ImGui::Begin("横方向のコンテンツサイズのデモウィンドウ", &show_horizontal_contents_size_demo_window, show_h_scrollbar ? ImGuiWindowFlags_HorizontalScrollbar : 0);
            IMGUI_DEMO_MARKER("Layout/Scrolling/Horizontal contents size demo window");
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 0));
            HelpMarker(
                "横スクロールを有効にしたときに、各ウィジェットがどう反応し、作業矩形の拡大にどう影響するかを試せます。\n\n"
                "矩形を可視化するには「メトリクス->ツール->ウィンドウの矩形を表示」を使ってください。");
            ImGui::Checkbox("横スクロールバー", &show_h_scrollbar);
            ImGui::Checkbox("ボタン", &show_button);            // Will grow contents size (unless explicitly overwritten)
            ImGui::Checkbox("ツリーノード", &show_tree_nodes);    // Will grow contents size and display highlight over full width
            ImGui::Checkbox("折り返しテキスト", &show_text_wrapped);// Will grow and use contents size
            ImGui::Checkbox("カラム", &show_columns);          // Will use contents size
            ImGui::Checkbox("タブバー", &show_tab_bar);          // Will use contents size
            ImGui::Checkbox("子ウィンドウ", &show_child);              // Will grow and use contents size
            ImGui::Checkbox("コンテンツサイズを明示指定", &explicit_content_size);
            ImGui::Text("スクロール %.1f/%.1f %.1f/%.1f", ImGui::GetScrollX(), ImGui::GetScrollMaxX(), ImGui::GetScrollY(), ImGui::GetScrollMaxY());
            if (explicit_content_size)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::CalcTextSize("123456").x);
                ImGui::DragFloat("##csx", &contents_size_x);
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 10, p.y + 10), IM_COL32_WHITE);
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + contents_size_x - 10, p.y), ImVec2(p.x + contents_size_x, p.y + 10), IM_COL32_WHITE);
                ImGui::Dummy(ImVec2(0, 10));
            }
            ImGui::PopStyleVar(2);
            ImGui::Separator();
            if (show_button)
            {
                ImGui::Button("これは幅 300 のボタンです", ImVec2(300, 0));
            }
            if (show_tree_nodes)
            {
                bool open = true;
                if (ImGui::TreeNode("これはツリーノードです"))
                {
                    if (ImGui::TreeNode("さらにもう1つのツリーノード..."))
                    {
                        ImGui::Text("ツリーの内容");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::CollapsingHeader("CollapsingHeader", &open);
            }
            if (show_text_wrapped)
            {
                ImGui::TextWrapped("このテキストは作業矩形の端で自動的に折り返されるはずです。");
            }
            if (show_columns)
            {
                ImGui::Text("テーブル:");
                if (ImGui::BeginTable("table", 4, ImGuiTableFlags_Borders))
                {
                    for (int n = 0; n < 4; n++)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("幅 %.2f", ImGui::GetContentRegionAvail().x);
                    }
                    ImGui::EndTable();
                }
                ImGui::Text("カラム:");
                ImGui::Columns(4);
                for (int n = 0; n < 4; n++)
                {
                    ImGui::Text("幅 %.2f", ImGui::GetColumnWidth());
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
            }
            if (show_tab_bar && ImGui::BeginTabBar("Hello"))
            {
                if (ImGui::BeginTabItem("OneOneOne")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("TwoTwoTwo")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("ThreeThreeThree")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("FourFourFour")) { ImGui::EndTabItem(); }
                ImGui::EndTabBar();
            }
            if (show_child)
            {
                ImGui::BeginChild("child", ImVec2(0, 0), ImGuiChildFlags_Borders);
                ImGui::EndChild();
            }
            ImGui::End();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("テキストのクリッピング"))
    {
        IMGUI_DEMO_MARKER("Layout/Text Clipping");
        static ImVec2 size(100.0f, 100.0f);
        static ImVec2 offset(30.0f, 30.0f);
        ImGui::DragFloat2("サイズ", (float*)&size, 0.5f, 1.0f, 200.0f, "%.0f");
        ImGui::TextWrapped("(クリックしてドラッグするとスクロールします)");

        HelpMarker(
            "(左) ImGui::PushClipRect() を使う場合:\n"
            "ImGui の当たり判定と ImDrawList の描画の両方に影響します。\n"
            "(クリッピング矩形を操作にも反映させたい場合はこちら)\n\n"
            "(中央) ImDrawList::PushClipRect() を使う場合:\n"
            "ImDrawList の描画にのみ影響します。\n"
            "(ImDrawList の呼び出しだけを使っているときの近道として)\n\n"
            "(右) ImDrawList::AddText() に細かい ClipRect を渡す場合:\n"
            "その ImDrawList::AddText() の描画だけに影響します。\n"
            "クリッピング矩形を変更せずに描画呼び出しを減らすため、内部でよく使われる方法です。");

        for (int n = 0; n < 3; n++)
        {
            if (n > 0)
                ImGui::SameLine();

            ImGui::PushID(n);
            ImGui::InvisibleButton("##canvas", size);
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                offset.x += ImGui::GetIO().MouseDelta.x;
                offset.y += ImGui::GetIO().MouseDelta.y;
            }
            ImGui::PopID();
            if (!ImGui::IsItemVisible()) // Skip rendering as ImDrawList elements are not clipped.
                continue;

            const ImVec2 p0 = ImGui::GetItemRectMin();
            const ImVec2 p1 = ImGui::GetItemRectMax();
            const char* text_str = "1行目 こんにちは\n2行目 切り取ってみて!";
            const ImVec2 text_pos = ImVec2(p0.x + offset.x, p0.y + offset.y);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            switch (n)
            {
            case 0:
                ImGui::PushClipRect(p0, p1, true);
                draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
                draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
                ImGui::PopClipRect();
                break;
            case 1:
                draw_list->PushClipRect(p0, p1, true);
                draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
                draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);
                draw_list->PopClipRect();
                break;
            case 2:
                ImVec4 clip_rect(p0.x, p0.y, p1.x, p1.y); // AddText() takes a ImVec4* here so let's convert.
                draw_list->AddRectFilled(p0, p1, IM_COL32(90, 90, 120, 255));
                draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), text_pos, IM_COL32_WHITE, text_str, NULL, 0.0f, &clip_rect);
                break;
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("重なりモード"))
    {
        IMGUI_DEMO_MARKER("Layout/Overlap Mode");
        static bool enable_allow_overlap = true;

        HelpMarker(
            "当たり判定は既定で項目の送信順に行われ、一般には「奥から手前」の順と受け取られます。\n\n"
            "SetNextItemAllowOverlap() を使うと、その項目が別の項目に重ねられる可能性があることを伝えられます。"
            "こうするとホバー判定の仕組みが変わり、AllowOverlap モードの項目はホバー状態を受け付けるのに1フレーム余分にかかります。");
        ImGui::Checkbox("AllowOverlap を有効にする", &enable_allow_overlap);

        ImVec2 button1_pos = ImGui::GetCursorScreenPos();
        ImVec2 button2_pos = ImVec2(button1_pos.x + 50.0f, button1_pos.y + 50.0f);
        if (enable_allow_overlap)
            ImGui::SetNextItemAllowOverlap();
        ImGui::Button("ボタン 1", ImVec2(80, 80));
        ImGui::SetCursorScreenPos(button2_pos);
        ImGui::Button("ボタン 2", ImVec2(80, 80));

        // This is typically used with width-spanning items.
        // (note that Selectable() has a dedicated flag ImGuiSelectableFlags_AllowOverlap, which is a shortcut
        // for using SetNextItemAllowOverlap(). For demo purpose we use SetNextItemAllowOverlap() here.)
        if (enable_allow_overlap)
            ImGui::SetNextItemAllowOverlap();
        ImGui::Selectable("選択できる項目", false);
        ImGui::SameLine();
        ImGui::SmallButton("++");

        ImGui::TreePop();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowPopups()
//-----------------------------------------------------------------------------

static void DemoWindowPopups()
{
    if (!ImGui::CollapsingHeader("ポップアップとモーダルウィンドウ"))
        return;

    // The properties of popups windows are:
    // - They block normal mouse hovering detection outside them. (*)
    // - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    // - Their visibility state (~bool) is held internally by Dear ImGui instead of being held by the programmer as
    //   we are used to with regular Begin() calls. User can manipulate the visibility state by calling OpenPopup().
    // (*) One can use IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even
    //     when normally blocked by a popup.
    // Those three properties are connected. The library needs to hold their visibility state BECAUSE it can close
    // popups at any time.

    // Typical use for regular windows:
    //   bool my_tool_is_active = false; if (ImGui::Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
    // Typical use for popups:
    //   if (ImGui::Button("Open")) ImGui::OpenPopup("MyPopup"); if (ImGui::BeginPopup("MyPopup")) { [...] EndPopup(); }

    // With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
    // This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

    if (ImGui::TreeNode("ポップアップ"))
    {
        IMGUI_DEMO_MARKER("Popups/Popups");
        ImGui::TextWrapped(
            "ポップアップが表示されている間は、その背後のウィンドウを操作できなくなります。"
            "ポップアップの外側をクリックすると閉じます。");

        static int selected_fish = -1;
        const char* names[] = { "タイ", "ハドック", "サバ", "スケトウダラ", "アマダイ" };
        static bool toggles[] = { true, false, false, false, false };

        // Simple selection popup (if you want to show the current selection inside the Button itself,
        // you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
        if (ImGui::Button("選択.."))
            ImGui::OpenPopup("my_select_popup");
        ImGui::SameLine();
        ImGui::TextUnformatted(selected_fish == -1 ? "<なし>" : names[selected_fish]);
        if (ImGui::BeginPopup("my_select_popup"))
        {
            ImGui::SeparatorText("水槽の中身");
            for (int i = 0; i < IM_COUNTOF(names); i++)
                if (ImGui::Selectable(names[i]))
                    selected_fish = i;
            ImGui::EndPopup();
        }

        // Showing a menu with toggles
        if (ImGui::Button("Toggle.."))
            ImGui::OpenPopup("my_toggle_popup");
        if (ImGui::BeginPopup("my_toggle_popup"))
        {
            for (int i = 0; i < IM_COUNTOF(names); i++)
                ImGui::MenuItem(names[i], "", &toggles[i]);
            if (ImGui::BeginMenu("サブメニュー"))
            {
                ImGui::MenuItem("クリックしてね");
                ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::Text("ここにツールチップ");
            ImGui::SetItemTooltip("ポップアップの上に出るツールチップです");

            if (ImGui::Button("重ねたポップアップ"))
                ImGui::OpenPopup("another popup");
            if (ImGui::BeginPopup("another popup"))
            {
                for (int i = 0; i < IM_COUNTOF(names); i++)
                    ImGui::MenuItem(names[i], "", &toggles[i]);
                if (ImGui::BeginMenu("サブメニュー"))
                {
                    ImGui::MenuItem("クリックしてね");
                    if (ImGui::Button("重ねたポップアップ"))
                        ImGui::OpenPopup("another popup");
                    if (ImGui::BeginPopup("another popup"))
                    {
                        ImGui::Text("ここが一番最後です。");
                        ImGui::EndPopup();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
            ImGui::EndPopup();
        }

        // Call the more complete ShowExampleMenuFile which we use in various places of this demo
        if (ImGui::Button("メニュー付き.."))
            ImGui::OpenPopup("my_file_popup");
        if (ImGui::BeginPopup("my_file_popup", ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("ファイル"))
                {
                    ShowExampleMenuFile();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("編集"))
                {
                    ImGui::MenuItem("ダミー");
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::Text("ポップアップからこんにちは!");
            ImGui::Button("これはダミーのボタンです..");
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("コンテキストメニュー"))
    {
        IMGUI_DEMO_MARKER("Popups/Context menus");
        HelpMarker("「Context」系の関数は、特定の項目やウィンドウの識別子にポップアップを結び付けるための簡単なヘルパーです。");

        // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
        //     if (id == 0)
        //         id = GetItemID(); // Use last item id
        //     if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
        //         OpenPopup(id);
        //     return BeginPopup(id);
        // For advanced uses you may want to replicate and customize this code.
        // See more details in BeginPopupContextItem().

        // Example 1
        // When used after an item that has an ID (e.g. Button), we can skip providing an ID to BeginPopupContextItem(),
        // and BeginPopupContextItem() will use the last item ID as the popup ID.
        {
            const char* names[5] = { "ラベル1", "ラベル2", "ラベル3", "ラベル4", "ラベル5" };
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                if (ImGui::Selectable(names[n], selected == n))
                    selected = n;
                if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
                {
                    selected = n;
                    ImGui::Text("これは \"%s\" のポップアップです!", names[n]);
                    if (ImGui::Button("閉じる"))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
                ImGui::SetItemTooltip("右クリックでポップアップを開きます");
            }
        }

        // Example 2
        // Popup on a Text() element which doesn't have an identifier: we need to provide an identifier to BeginPopupContextItem().
        // Using an explicit identifier is also convenient if you want to activate the popups from different locations.
        {
            HelpMarker("Text() の要素には安定した識別子がないため、こちらで指定する必要があります。");
            static float value = 0.5f;
            ImGui::Text("値 = %.3f <-- (1) このテキストを右クリック", value);
            if (ImGui::BeginPopupContextItem("my popup"))
            {
                if (ImGui::Selectable("0 にする")) value = 0.0f;
                if (ImGui::Selectable("円周率にする")) value = 3.1415f;
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
                ImGui::EndPopup();
            }

            // We can also use OpenPopupOnItemClick() to toggle the visibility of a given popup.
            // Here we make it that right-clicking this other text element opens the same popup as above.
            // The popup itself will be submitted by the code above.
            ImGui::Text("(2) または、このテキストを右クリック");
            ImGui::OpenPopupOnItemClick("my popup", ImGuiPopupFlags_MouseButtonRight);

            // Back to square one: manually open the same popup.
            if (ImGui::Button("(3) または、このボタンをクリック"))
                ImGui::OpenPopup("my popup");
        }

        // Example 3
        // When using BeginPopupContextItem() with an implicit identifier (NULL == use last item ID),
        // we need to make sure your item identifier is stable.
        // In this example we showcase altering the item label while preserving its identifier, using the ### operator (see FAQ).
        {
            HelpMarker("項目の ID に紐づけたポップアップ ID の使用例です。項目のラベルは変化しますが、### 演算子で ID は安定させています。");
            static char name[32] = "ラベル1";
            char buf[64];
            sprintf(buf, "ボタン: %s###Button", name); // ### operator override ID ignoring the preceding label
            ImGui::Button(buf);
            if (ImGui::BeginPopupContextItem())
            {
                ImGui::Text("名前を編集:");
                ImGui::InputText("##edit", name, IM_COUNTOF(name));
                if (ImGui::Button("閉じる"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::SameLine(); ImGui::Text("(<-- ここを右クリック)");
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("モーダル"))
    {
        IMGUI_DEMO_MARKER("Popups/Modals");
        ImGui::TextWrapped("モーダルウィンドウはポップアップに似ていますが、外側をクリックしても閉じられません。");

        if (ImGui::Button("削除.."))
            ImGui::OpenPopup("削除しますか?");

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("削除しますか?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("これらの素敵なファイルはすべて削除されます。\nこの操作は元に戻せません!");
            ImGui::Separator();

            //static int unused_i = 0;
            //ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

            static bool dont_ask_me_next_time = false;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::Checkbox("次回から確認しない", &dont_ask_me_next_time);
            ImGui::PopStyleVar();

            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("キャンセル", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        if (ImGui::Button("モーダルを重ねる.."))
            ImGui::OpenPopup("重ねたモーダル 1");
        if (ImGui::BeginPopupModal("重ねたモーダル 1", NULL, ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("ファイル"))
                {
                    if (ImGui::MenuItem("何かのメニュー項目")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::Text("1つ目の重ねモーダルからこんにちは\n背後に style.Colors[ImGuiCol_ModalWindowDimBg] を使っています。");

            // Testing behavior of widgets stacking their own regular popups over the modal.
            static int item = 1;
            static float color[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            ImGui::Combo("コンボ", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
            ImGui::ColorEdit4("色", color);

            if (ImGui::Button("さらにモーダルを追加.."))
                ImGui::OpenPopup("重ねたモーダル 2");

            // Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which
            // will close the popup. Note that the visibility state of popups is owned by imgui, so the input value
            // of the bool actually doesn't matter here.
            bool unused_open = true;
            if (ImGui::BeginPopupModal("重ねたモーダル 2", &unused_open))
            {
                ImGui::Text("2つ目の重ねモーダルからこんにちは!");
                ImGui::ColorEdit4("色", color); // Allow opening another nested popup
                if (ImGui::Button("閉じる"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            if (ImGui::Button("閉じる"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("通常のウィンドウ内のメニュー"))
    {
        IMGUI_DEMO_MARKER("Popups/Menus inside a regular window");
        ImGui::TextWrapped("ここでは通常のウィンドウにメニュー項目を追加できるかを試しています。かなり珍しい使い方ですが、動作するはずです!");
        ImGui::Separator();

        ImGui::MenuItem("メニュー項目", "Ctrl+M");
        if (ImGui::BeginMenu("通常のウィンドウ内のメニュー"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::TreePop();
    }
}

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate ImVector<MyItem> template if this structure is defined inside the demo function)
namespace
{
// We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
// This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
// But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (ImGuiTableSortSpec::ColumnIndex)
// If you don't use sorting, you will generally never care about giving column an ID!
enum MyItemColumnID
{
    MyItemColumnID_ID,
    MyItemColumnID_Name,
    MyItemColumnID_Action,
    MyItemColumnID_Quantity,
    MyItemColumnID_Description
};

struct MyItem
{
    int         ID;
    const char* Name;
    int         Quantity;

    // We have a problem which is affecting _only this demo_ and should not affect your code:
    // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
    // however qsort doesn't allow passing user data to comparing function.
    // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
    // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
    // We could technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
    // very often by the sorting algorithm it would be a little wasteful.
    static const ImGuiTableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, MyItem* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), MyItem::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const MyItem* a = (const MyItem*)lhs;
        const MyItem* b = (const MyItem*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
            case MyItemColumnID_ID:             delta = (a->ID - b->ID);                break;
            case MyItemColumnID_Name:           delta = (strcmp(a->Name, b->Name));     break;
            case MyItemColumnID_Quantity:       delta = (a->Quantity - b->Quantity);    break;
            case MyItemColumnID_Description:    delta = (strcmp(a->Name, b->Name));     break;
            default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        // qsort() is instable so always return a way to differentiate items.
        // Your own compare function may want to avoid fallback on implicit sort specs.
        // e.g. a Name compare if it wasn't already part of the sort specs.
        return a->ID - b->ID;
    }
};
const ImGuiTableSortSpecs* MyItem::s_current_sort_specs = NULL;
}

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, (float)(int)(style.FramePadding.y * 0.60f));
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, (float)(int)(style.ItemSpacing.y * 0.60f));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}

// Show a combo box with a choice of sizing policies
static void EditTableSizingFlags(ImGuiTableFlags* p_flags)
{
    struct EnumDesc { ImGuiTableFlags Value; const char* Name; const char* Tooltip; };
    static const EnumDesc policies[] =
    {
        { ImGuiTableFlags_None,               "既定",                            "既定のサイズ調整ポリシーを使います:\n- ScrollX が有効な場合、またはホストウィンドウに ImGuiWindowFlags_AlwaysAutoResize がある場合は ImGuiTableFlags_SizingFixedFit。\n- それ以外は ImGuiTableFlags_SizingStretchSame。" },
        { ImGuiTableFlags_SizingFixedFit,     "ImGuiTableFlags_SizingFixedFit",     "カラムは既定で _WidthFixed (リサイズ可能な場合) または _WidthAuto (リサイズ不可の場合) になり、内容の幅に合わせます。" },
        { ImGuiTableFlags_SizingFixedSame,    "ImGuiTableFlags_SizingFixedSame",    "すべてのカラムが同じ幅になり、内容の最大幅に合わせます。\n暗黙的に ImGuiTableFlags_Resizable を無効化し、ImGuiTableFlags_NoKeepColumnsVisible を有効化します。" },
        { ImGuiTableFlags_SizingStretchProp,  "ImGuiTableFlags_SizingStretchProp",  "カラムは既定で _WidthStretch になり、それぞれの幅に比例した重みを持ちます。" },
        { ImGuiTableFlags_SizingStretchSame,  "ImGuiTableFlags_SizingStretchSame",  "カラムは既定で _WidthStretch になり、すべて同じ重みを持ちます。" }
    };
    int idx;
    for (idx = 0; idx < IM_COUNTOF(policies); idx++)
        if (policies[idx].Value == (*p_flags & ImGuiTableFlags_SizingMask_))
            break;
    const char* preview_text = (idx < IM_COUNTOF(policies)) ? policies[idx].Name + (idx > 0 ? strlen("ImGuiTableFlags") : 0) : "";
    if (ImGui::BeginCombo("サイズ調整ポリシー", preview_text))
    {
        for (int n = 0; n < IM_COUNTOF(policies); n++)
            if (ImGui::Selectable(policies[n].Name, idx == n))
                *p_flags = (*p_flags & ~ImGuiTableFlags_SizingMask_) | policies[n].Value;
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
        for (int m = 0; m < IM_COUNTOF(policies); m++)
        {
            ImGui::Separator();
            ImGui::Text("%s:", policies[m].Name);
            ImGui::Separator();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().IndentSpacing * 0.5f);
            ImGui::TextUnformatted(policies[m].Tooltip);
        }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void EditTableColumnsFlags(ImGuiTableColumnFlags* p_flags)
{
    ImGui::CheckboxFlags("_Disabled", p_flags, ImGuiTableColumnFlags_Disabled); ImGui::SameLine(); HelpMarker("全体を無効にするフラグです (コンテキストメニューからも隠します)");
    ImGui::CheckboxFlags("_DefaultHide", p_flags, ImGuiTableColumnFlags_DefaultHide);
    ImGui::CheckboxFlags("_DefaultSort", p_flags, ImGuiTableColumnFlags_DefaultSort);
    if (ImGui::CheckboxFlags("_WidthStretch", p_flags, ImGuiTableColumnFlags_WidthStretch))
        *p_flags &= ~(ImGuiTableColumnFlags_WidthMask_ ^ ImGuiTableColumnFlags_WidthStretch);
    if (ImGui::CheckboxFlags("_WidthFixed", p_flags, ImGuiTableColumnFlags_WidthFixed))
        *p_flags &= ~(ImGuiTableColumnFlags_WidthMask_ ^ ImGuiTableColumnFlags_WidthFixed);
    ImGui::CheckboxFlags("_NoResize", p_flags, ImGuiTableColumnFlags_NoResize);
    ImGui::CheckboxFlags("_NoReorder", p_flags, ImGuiTableColumnFlags_NoReorder);
    ImGui::CheckboxFlags("_NoHide", p_flags, ImGuiTableColumnFlags_NoHide);
    ImGui::CheckboxFlags("_NoClip", p_flags, ImGuiTableColumnFlags_NoClip);
    ImGui::CheckboxFlags("_NoSort", p_flags, ImGuiTableColumnFlags_NoSort);
    ImGui::CheckboxFlags("_NoSortAscending", p_flags, ImGuiTableColumnFlags_NoSortAscending);
    ImGui::CheckboxFlags("_NoSortDescending", p_flags, ImGuiTableColumnFlags_NoSortDescending);
    ImGui::CheckboxFlags("_NoHeaderLabel", p_flags, ImGuiTableColumnFlags_NoHeaderLabel);
    ImGui::CheckboxFlags("_NoHeaderWidth", p_flags, ImGuiTableColumnFlags_NoHeaderWidth);
    ImGui::CheckboxFlags("_PreferSortAscending", p_flags, ImGuiTableColumnFlags_PreferSortAscending);
    ImGui::CheckboxFlags("_PreferSortDescending", p_flags, ImGuiTableColumnFlags_PreferSortDescending);
    ImGui::CheckboxFlags("_IndentEnable", p_flags, ImGuiTableColumnFlags_IndentEnable); ImGui::SameLine(); HelpMarker("カラム 0 の既定値");
    ImGui::CheckboxFlags("_IndentDisable", p_flags, ImGuiTableColumnFlags_IndentDisable); ImGui::SameLine(); HelpMarker("カラム 1 以降の既定値");
    ImGui::CheckboxFlags("_AngledHeader", p_flags, ImGuiTableColumnFlags_AngledHeader);
}

static void ShowTableColumnsStatusFlags(ImGuiTableColumnFlags flags)
{
    ImGui::CheckboxFlags("_IsEnabled", &flags, ImGuiTableColumnFlags_IsEnabled);
    ImGui::CheckboxFlags("_IsVisible", &flags, ImGuiTableColumnFlags_IsVisible);
    ImGui::CheckboxFlags("_IsSorted", &flags, ImGuiTableColumnFlags_IsSorted);
    ImGui::CheckboxFlags("_IsHovered", &flags, ImGuiTableColumnFlags_IsHovered);
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowTables()
//-----------------------------------------------------------------------------

static void DemoWindowTables()
{
    //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader("テーブルとカラム"))
        return;

    // Using those as a base value to create width/height that are factor of the size of our font
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    ImGui::PushID("Tables");

    int open_action = -1;
    if (ImGui::Button("すべて展開"))
        open_action = 1;
    ImGui::SameLine();
    if (ImGui::Button("すべて折りたたむ"))
        open_action = 0;
    ImGui::SameLine();

    // Options
    static bool disable_indent = false;
    ImGui::Checkbox("ツリーのインデントを無効にする", &disable_indent);
    ImGui::SameLine();
    HelpMarker("ツリーノードのインデントを無効にして、デモのテーブルがウィンドウの幅いっぱいを使えるようにします。");
    ImGui::Separator();
    if (disable_indent)
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);

    // About Styling of tables
    // Most settings are configured on a per-table basis via the flags passed to BeginTable() and TableSetupColumns APIs.
    // There are however a few settings that a shared and part of the ImGuiStyle structure:
    //   style.CellPadding                          // Padding within each cell
    //   style.Colors[ImGuiCol_TableHeaderBg]       // Table header background
    //   style.Colors[ImGuiCol_TableBorderStrong]   // Table outer and header borders
    //   style.Colors[ImGuiCol_TableBorderLight]    // Table inner borders
    //   style.Colors[ImGuiCol_TableRowBg]          // Table row background when ImGuiTableFlags_RowBg is enabled (even rows)
    //   style.Colors[ImGuiCol_TableRowBgAlt]       // Table row background when ImGuiTableFlags_RowBg is enabled (odds rows)

    // Demos
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("基本"))
    {
        IMGUI_DEMO_MARKER("Tables/Basic");
        // Here we will showcase three different ways to output a table.
        // They are very simple variations of a same thing!

        // [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the column.
        // In many situations, this is the most flexible and easy to use pattern.
        HelpMarker("TableNextRow() を使い、各セルの「前に」 TableSetColumnIndex() をループ内で呼んでいます。");
        if (ImGui::BeginTable("table1", 3))
        {
            for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("行 %d 列 %d", row, column);
                }
            }
            ImGui::EndTable();
        }

        // [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop + TableSetColumnIndex().
        // This is generally more convenient when you have code manually submitting the contents of each column.
        HelpMarker("TableNextRow() を使い、各セルの「前に」 TableNextColumn() を手動で呼んでいます。");
        if (ImGui::BeginTable("table2", 3))
        {
            for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("行 %d", row);
                ImGui::TableNextColumn();
                ImGui::Text("何かの内容");
                ImGui::TableNextColumn();
                ImGui::Text("123.456");
            }
            ImGui::EndTable();
        }

        // [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
        // as TableNextColumn() will automatically wrap around and create new rows as needed.
        // This is generally more convenient when your cells all contains the same type of data.
        HelpMarker(
            "TableNextColumn() だけを使う方法です。すべてのセルが同じ種類の内容を持つテーブルでは"
            "便利なことが多いです。\n これは Columns API の古い NextColumn() 関数にも近く、"
            "Columns から Tables への移行を助けるために用意されています。");
        if (ImGui::BeginTable("table3", 3))
        {
            for (int item = 0; item < 14; item++)
            {
                ImGui::TableNextColumn();
                ImGui::Text("項目 %d", item);
            }
            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("枠線と背景"))
    {
        IMGUI_DEMO_MARKER("Tables/Borders, background");
        // Expose a few Borders related flags interactively
        enum ContentsType { CT_Text, CT_FillButton };
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        static bool display_headers = false;
        static int contents_type = CT_Text;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
        ImGui::CheckboxFlags("ImGuiTableFlags_Borders", &flags, ImGuiTableFlags_Borders);
        ImGui::SameLine(); HelpMarker("ImGuiTableFlags_Borders\n = ImGuiTableFlags_BordersInnerV\n | ImGuiTableFlags_BordersOuterV\n | ImGuiTableFlags_BordersInnerH\n | ImGuiTableFlags_BordersOuterH");
        ImGui::Indent();

        ImGui::CheckboxFlags("ImGuiTableFlags_BordersH", &flags, ImGuiTableFlags_BordersH);
        ImGui::Indent();
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterH", &flags, ImGuiTableFlags_BordersOuterH);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerH", &flags, ImGuiTableFlags_BordersInnerH);
        ImGui::Unindent();

        ImGui::CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
        ImGui::Indent();
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags, ImGuiTableFlags_BordersOuterV);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags, ImGuiTableFlags_BordersInnerV);
        ImGui::Unindent();

        ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuter", &flags, ImGuiTableFlags_BordersOuter);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersInner", &flags, ImGuiTableFlags_BordersInner);
        ImGui::Unindent();

        ImGui::AlignTextToFramePadding(); ImGui::Text("セルの内容:");
        ImGui::SameLine(); ImGui::RadioButton("テキスト", &contents_type, CT_Text);
        ImGui::SameLine(); ImGui::RadioButton("塗りつぶしボタン", &contents_type, CT_FillButton);
        ImGui::Checkbox("ヘッダーを表示", &display_headers);
        ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody); ImGui::SameLine(); HelpMarker("本体のカラムの縦線を無効にします (ヘッダーには常に表示されます)");
        PopStyleCompact();

        if (ImGui::BeginTable("table1", 3, flags))
        {
            // Display headers so we can inspect their interaction with borders
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them now. See other sections for details)
            if (display_headers)
            {
                ImGui::TableSetupColumn("1列目");
                ImGui::TableSetupColumn("2列目");
                ImGui::TableSetupColumn("3列目");
                ImGui::TableHeadersRow();
            }

            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    char buf[32];
                    sprintf(buf, "こんにちは %d,%d", column, row);
                    if (contents_type == CT_Text)
                        ImGui::TextUnformatted(buf);
                    else if (contents_type == CT_FillButton)
                        ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("リサイズ可能・ストレッチ"))
    {
        IMGUI_DEMO_MARKER("Tables/Resizable, stretch");
        // By default, if we don't enable ScrollX the sizing policy for each column is "Stretch"
        // All columns maintain a sizing weight, and they will occupy all available width.
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
        ImGui::SameLine(); HelpMarker(
            "_Resizable フラグを使うと _BordersInnerV フラグも自動的に有効になります。"
            "これのチェックを外してもリサイズ用の境界線が表示され続けるのはそのためです。");
        PopStyleCompact();

        if (ImGui::BeginTable("table1", 3, flags))
        {
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("こんにちは %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("リサイズ可能・固定幅"))
    {
        IMGUI_DEMO_MARKER("Tables/Resizable, fixed");
        // Here we use ImGuiTableFlags_SizingFixedFit (even though _ScrollX is not set)
        // So columns will adopt the "Fixed" policy and will maintain a fixed width regardless of the whole available width (unless table is small)
        // If there is not enough available width to fit all columns, they will however be resized down.
        // FIXME-TABLE: Providing a stretch-on-init would make sense especially for tables which don't have saved settings
        HelpMarker(
            "_Resizable と _SizingFixedFit フラグを使っています。\n"
            "横スクロールを使いたい場合は、固定幅のカラムのほうが理にかなっていることが多いです。\n\n"
            "カラムの境界をダブルクリックすると、内容に合わせて幅を自動調整します。");
        PushStyleCompact();
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
        ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
        PopStyleCompact();

        if (ImGui::BeginTable("table1", 3, flags))
        {
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("こんにちは %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("リサイズ可能・混在"))
    {
        IMGUI_DEMO_MARKER("Tables/Resizable, mixed");
        HelpMarker(
            "TableSetupColumn() を使って、カラムごとにリサイズの方針を変えています。\n\n"
            "固定幅とストレッチを混在させる場合は、通常は末尾の1つ、多くても2つのカラムだけを _WidthStretch にするとよいでしょう。");
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

        if (ImGui::BeginTable("table1", 3, flags))
        {
            ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("%s %d,%d", (column == 2) ? "ストレッチ" : "固定", column, row);
                }
            }
            ImGui::EndTable();
        }
        if (ImGui::BeginTable("table2", 6, flags))
        {
            ImGui::TableSetupColumn("AAA", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableSetupColumn("DDD", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("EEE", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("FFF", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide);
            ImGui::TableHeadersRow();
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 6; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("%s %d,%d", (column >= 3) ? "ストレッチ" : "固定", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("並べ替え・表示切替・ヘッダー付き"))
    {
        IMGUI_DEMO_MARKER("Tables/Reorderable, hideable, with headers");
        HelpMarker(
            "カラムのヘッダーをドラッグすると順序を入れ替えられます。\n\n"
            "ヘッダーを右クリックするとコンテキストメニューが開きます。");
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
        ImGui::CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
        ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody);
        ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags, ImGuiTableFlags_NoBordersInBodyUntilResize); ImGui::SameLine(); HelpMarker("リサイズのためにホバーするまで、本体のカラムの縦線を表示しません (ヘッダーには常に表示されます)");
        ImGui::CheckboxFlags("ImGuiTableFlags_HighlightHoveredColumn", &flags, ImGuiTableFlags_HighlightHoveredColumn);
        PopStyleCompact();

        if (ImGui::BeginTable("table1", 3, flags))
        {
            // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
            // (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
            ImGui::TableSetupColumn("1列目");
            ImGui::TableSetupColumn("2列目");
            ImGui::TableSetupColumn("3列目");
            ImGui::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("こんにちは %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }

        // Use outer_size.x == 0.0f instead of default to make the table as tight as possible
        // (only valid when no scrolling and no stretch column)
        if (ImGui::BeginTable("table2", 3, flags | ImGuiTableFlags_SizingFixedFit, ImVec2(0.0f, 0.0f)))
        {
            ImGui::TableSetupColumn("1列目");
            ImGui::TableSetupColumn("2列目");
            ImGui::TableSetupColumn("3列目");
            ImGui::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("固定 %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("余白 (Padding)"))
    {
        IMGUI_DEMO_MARKER("Tables/Padding");
        // First example: showcase use of padding flags and effect of BorderOuterV/BorderInnerV on X padding.
        // We don't expose BorderOuterH/BorderInnerH here because they have no effect on X padding.
        HelpMarker(
            "カラムの端が見える機能を使うときは、外側の余白を有効にしたいことがよくあります:\n"
            "例:\n"
            "- BorderOuterV\n"
            "- 何らかの行選択\n"
            "そのため、BorderOuterV を有効にすると既定で PadOuterX になります。"
            "PadOuterX や NoPadOuterX を使えば既定値を上書きできます。\n\n"
            "実際の余白の値は style.CellPadding を使います。\n\n"
            "このデモでは、横線が既定の水平余白に影響しないことを強調するため、横線を表示していません。");

        static ImGuiTableFlags flags1 = ImGuiTableFlags_BordersV;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_PadOuterX", &flags1, ImGuiTableFlags_PadOuterX);
        ImGui::SameLine(); HelpMarker("いちばん外側の余白を有効にします (ImGuiTableFlags_BordersOuterV が設定されている場合の既定)");
        ImGui::CheckboxFlags("ImGuiTableFlags_NoPadOuterX", &flags1, ImGuiTableFlags_NoPadOuterX);
        ImGui::SameLine(); HelpMarker("いちばん外側の余白を無効にします (ImGuiTableFlags_BordersOuterV が設定されていない場合の既定)");
        ImGui::CheckboxFlags("ImGuiTableFlags_NoPadInnerX", &flags1, ImGuiTableFlags_NoPadInnerX);
        ImGui::SameLine(); HelpMarker("カラム間の内側の余白を無効にします (BordersOuterV が有効なら内側の余白は2倍、無効なら1つ分)");
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags1, ImGuiTableFlags_BordersOuterV);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags1, ImGuiTableFlags_BordersInnerV);
        static bool show_headers = false;
        ImGui::Checkbox("show_headers", &show_headers);
        PopStyleCompact();

        if (ImGui::BeginTable("table_padding", 3, flags1))
        {
            if (show_headers)
            {
                ImGui::TableSetupColumn("1列目");
                ImGui::TableSetupColumn("2列目");
                ImGui::TableSetupColumn("3列目");
                ImGui::TableHeadersRow();
            }

            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    if (row == 0)
                    {
                        ImGui::Text("残り %.2f", ImGui::GetContentRegionAvail().x);
                    }
                    else
                    {
                        char buf[32];
                        sprintf(buf, "こんにちは %d,%d", column, row);
                        ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
                    }
                    //if (ImGui::TableGetColumnFlags() & ImGuiTableColumnFlags_IsHovered)
                    //    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 100, 0, 255));
                }
            }
            ImGui::EndTable();
        }

        // Second example: set style.CellPadding to (0.0) or a custom value.
        // FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
        HelpMarker("style.CellPadding に (0,0) や任意の値を設定する例です。");
        static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        static ImVec2 cell_padding(0.0f, 0.0f);
        static bool show_widget_frame_bg = true;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Borders", &flags2, ImGuiTableFlags_Borders);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersH", &flags2, ImGuiTableFlags_BordersH);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersV", &flags2, ImGuiTableFlags_BordersV);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersInner", &flags2, ImGuiTableFlags_BordersInner);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuter", &flags2, ImGuiTableFlags_BordersOuter);
        ImGui::CheckboxFlags("ImGuiTableFlags_RowBg", &flags2, ImGuiTableFlags_RowBg);
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags2, ImGuiTableFlags_Resizable);
        ImGui::Checkbox("show_widget_frame_bg", &show_widget_frame_bg);
        ImGui::SliderFloat2("CellPadding", &cell_padding.x, 0.0f, 10.0f, "%.0f");
        PopStyleCompact();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
        if (ImGui::BeginTable("table_padding_2", 3, flags2))
        {
            static char text_bufs[3 * 5][16]; // Mini text storage for 3x5 cells
            static bool init = true;
            if (!show_widget_frame_bg)
                ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
            for (int cell = 0; cell < 3 * 5; cell++)
            {
                ImGui::TableNextColumn();
                if (init)
                    strcpy(text_bufs[cell], "編集してね");
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushID(cell);
                ImGui::InputText("##cell", text_bufs[cell], IM_COUNTOF(text_bufs[cell]));
                ImGui::PopID();
            }
            if (!show_widget_frame_bg)
                ImGui::PopStyleColor();
            init = false;
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();

        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("サイズ調整ポリシー"))
    {
        IMGUI_DEMO_MARKER("Tables/Explicit widths");
        static ImGuiTableFlags flags1 = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags1, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags1, ImGuiTableFlags_NoHostExtendX);
        PopStyleCompact();

        static ImGuiTableFlags sizing_policy_flags[4] = { ImGuiTableFlags_SizingFixedFit, ImGuiTableFlags_SizingFixedSame, ImGuiTableFlags_SizingStretchProp, ImGuiTableFlags_SizingStretchSame };
        for (int table_n = 0; table_n < 4; table_n++)
        {
            ImGui::PushID(table_n);
            ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 30);
            EditTableSizingFlags(&sizing_policy_flags[table_n]);

            // To make it easier to understand the different sizing policy,
            // For each policy: we display one table where the columns have equal contents width,
            // and one where the columns have different contents width.
            if (ImGui::BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1))
            {
                for (int row = 0; row < 3; row++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("おやおや");
                    ImGui::TableNextColumn(); ImGui::Text("おやおや");
                    ImGui::TableNextColumn(); ImGui::Text("おやおや");
                }
                ImGui::EndTable();
            }
            if (ImGui::BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1))
            {
                for (int row = 0; row < 3; row++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("AAAA");
                    ImGui::TableNextColumn(); ImGui::Text("BBBBBBBB");
                    ImGui::TableNextColumn(); ImGui::Text("CCCCCCCCCCCC");
                }
                ImGui::EndTable();
            }
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("応用");
        ImGui::SameLine();
        HelpMarker(
            "このセクションでは、スクロールの有無やカラムの内容に応じて、さまざまなサイズ調整ポリシーが"
            "どのような効果を持つかを実際に試せます。");

        enum ContentsType { CT_ShowWidth, CT_ShortText, CT_LongText, CT_Button, CT_FillButton, CT_InputText };
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
        static int contents_type = CT_ShowWidth;
        static int column_count = 3;

        PushStyleCompact();
        ImGui::PushID("Advanced");
        ImGui::PushItemWidth(TEXT_BASE_WIDTH * 30);
        EditTableSizingFlags(&flags);
        ImGui::Combo("内容", &contents_type, "幅を表示\0短いテキスト\0長いテキスト\0ボタン\0塗りつぶしボタン\0InputText\0");
        if (contents_type == CT_FillButton)
        {
            ImGui::SameLine();
            HelpMarker(
                "右揃え (例: size.x = -FLT_MIN) を使うと、内容の幅がカラムの自動幅に影響し、"
                "それがまた内容の幅に影響する、というループが生まれる点に注意してください。");
        }
        ImGui::DragInt("カラム数", &column_count, 0.1f, 1, 64, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_PreciseWidths", &flags, ImGuiTableFlags_PreciseWidths);
        ImGui::SameLine(); HelpMarker("余った幅をストレッチ列に分配する処理を無効にします (幅 100 のテーブルに3カラムある場合の幅の割り当て: このフラグなし: 33,33,34。あり: 33,33,33)。カラム数が多いと、リサイズの動きが滑らかでなくなります。");
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
        ImGui::CheckboxFlags("ImGuiTableFlags_NoClip", &flags, ImGuiTableFlags_NoClip);
        ImGui::PopItemWidth();
        ImGui::PopID();
        PopStyleCompact();

        if (ImGui::BeginTable("table2", column_count, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 7)))
        {
            for (int cell = 0; cell < 10 * column_count; cell++)
            {
                ImGui::TableNextColumn();
                int column = ImGui::TableGetColumnIndex();
                int row = ImGui::TableGetRowIndex();

                ImGui::PushID(cell);
                char label[32];
                static char text_buf[32] = "";
                sprintf(label, "こんにちは %d,%d", column, row);
                switch (contents_type)
                {
                case CT_ShortText:  ImGui::TextUnformatted(label); break;
                case CT_LongText:   ImGui::Text("%s テキスト %d,%d\n2行にわたります..", column == 0 ? "長い" : "とても長い", column, row); break;
                case CT_ShowWidth:  ImGui::Text("W: %.1f", ImGui::GetContentRegionAvail().x); break;
                case CT_Button:     ImGui::Button(label); break;
                case CT_FillButton: ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f)); break;
                case CT_InputText:  ImGui::SetNextItemWidth(-FLT_MIN); ImGui::InputText("##", text_buf, IM_COUNTOF(text_buf)); break;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("縦スクロールとクリッピング"))
    {
        IMGUI_DEMO_MARKER("Tables/Vertical scrolling, with clipping");
        HelpMarker(
            "ここでは ScrollY を有効にしています。スクロール可能な内容を収めるための子ウィンドウが作られます。\n\n"
            "また、ImGuiListClipper を使って大量の項目の送信を仮想化する例も示しています。");
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (ImGui::BeginTable("table_scrolly", 3, flags, outer_size))
        {
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableSetupColumn("1列目", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("2列目", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("3列目", ImGuiTableColumnFlags_None);
            ImGui::TableHeadersRow();

            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(1000);
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    ImGui::TableNextRow();
                    for (int column = 0; column < 3; column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        ImGui::Text("こんにちは %d,%d", column, row);
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("横スクロール"))
    {
        IMGUI_DEMO_MARKER("Tables/Horizontal scrolling");
        HelpMarker(
            "ScrollX を有効にすると、既定のサイズ調整ポリシーは ImGuiTableFlags_SizingFixedFit になります。"
            "横スクロールがある状態でカラムを自動的に引き伸ばしてもあまり意味がないためです。\n\n"
            "また現在のバージョンでは、ScrollX とあわせて ScrollY も有効にしたい場面がほとんどです。"
            "コンテナウィンドウが内容に合わせて縦に自動拡張しないためです"
            "(この点は将来のバージョンで改善されるかもしれません)。");
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        static int freeze_cols = 1;
        static int freeze_rows = 1;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
        ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
        ImGui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
        ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
        ImGui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
        PopStyleCompact();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (ImGui::BeginTable("table_scrollx", 7, flags, outer_size))
        {
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
            ImGui::TableSetupColumn("行番号", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
            ImGui::TableSetupColumn("1列目");
            ImGui::TableSetupColumn("2列目");
            ImGui::TableSetupColumn("3列目");
            ImGui::TableSetupColumn("4列目");
            ImGui::TableSetupColumn("5列目");
            ImGui::TableSetupColumn("6列目");
            ImGui::TableHeadersRow();
            for (int row = 0; row < 20; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 7; column++)
                {
                    // Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible or performing width measurement.
                    // Because here we know that:
                    // - A) all our columns are contributing the same to row height
                    // - B) column 0 is always visible,
                    // We only always submit this one column and can skip others.
                    // More advanced per-column clipping behaviors may benefit from polling the status flags via TableGetColumnFlags().
                    if (!ImGui::TableSetColumnIndex(column) && column > 0)
                        continue;
                    if (column == 0)
                        ImGui::Text("行 %d", row);
                    else
                        ImGui::Text("こんにちは世界 %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Stretch + ScrollX");
        ImGui::SameLine();
        HelpMarker(
            "ストレッチ列と ScrollX を組み合わせる例です。"
            "かなり珍しい使い方で、テーブルに 'inner_width' を指定したときにだけ意味があります!\n"
            "明示的に指定しない場合 inner_width は outer_size.x と等しくなるため、ストレッチ列と"
            "ScrollX を組み合わせても意味がありません。");
        static ImGuiTableFlags flags2 = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
        static float inner_width = 1000.0f;
        PushStyleCompact();
        ImGui::PushID("flags3");
        ImGui::PushItemWidth(TEXT_BASE_WIDTH * 30);
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", &flags2, ImGuiTableFlags_ScrollX);
        ImGui::DragFloat("inner_width", &inner_width, 1.0f, 0.0f, FLT_MAX, "%.1f");
        ImGui::PopItemWidth();
        ImGui::PopID();
        PopStyleCompact();
        if (ImGui::BeginTable("table2", 7, flags2, outer_size, inner_width))
        {
            for (int cell = 0; cell < 20 * 7; cell++)
            {
                ImGui::TableNextColumn();
                ImGui::Text("こんにちは世界 %d,%d", ImGui::TableGetColumnIndex(), ImGui::TableGetRowIndex());
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("カラムのフラグ"))
    {
        IMGUI_DEMO_MARKER("Tables/Columns flags");
        // Create a first table just to show all the options/flags we want to make visible in our example!
        const int column_count = 3;
        const char* column_names[column_count] = { "1列目", "2列目", "3列目" };
        static ImGuiTableColumnFlags column_flags[column_count] = { ImGuiTableColumnFlags_DefaultSort, ImGuiTableColumnFlags_None, ImGuiTableColumnFlags_DefaultHide };
        static ImGuiTableColumnFlags column_flags_out[column_count] = { 0, 0, 0 }; // Output from TableGetColumnFlags()

        if (ImGui::BeginTable("table_columns_flags_checkboxes", column_count, ImGuiTableFlags_None))
        {
            PushStyleCompact();
            for (int column = 0; column < column_count; column++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(column);
                ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation across columns
                ImGui::Text("'%s'", column_names[column]);
                ImGui::Spacing();
                ImGui::Text("入力フラグ:");
                EditTableColumnsFlags(&column_flags[column]);
                ImGui::Spacing();
                ImGui::Text("出力フラグ:");
                ImGui::BeginDisabled();
                ShowTableColumnsStatusFlags(column_flags_out[column]);
                ImGui::EndDisabled();
                ImGui::PopID();
            }
            PopStyleCompact();
            ImGui::EndTable();
        }

        // Create the real table we care about for the example!
        // We use a scrolling table to be able to showcase the difference between the _IsEnabled and _IsVisible flags above,
        // otherwise in a non-scrolling table columns are always visible (unless using ImGuiTableFlags_NoKeepColumnsVisible
        // + resizing the parent window down).
        const ImGuiTableFlags flags
            = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable;
        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 9);
        if (ImGui::BeginTable("table_columns_flags", column_count, flags, outer_size))
        {
            bool has_angled_header = false;
            for (int column = 0; column < column_count; column++)
            {
                has_angled_header |= (column_flags[column] & ImGuiTableColumnFlags_AngledHeader) != 0;
                ImGui::TableSetupColumn(column_names[column], column_flags[column]);
            }
            if (has_angled_header)
                ImGui::TableAngledHeadersRow();
            ImGui::TableHeadersRow();
            for (int column = 0; column < column_count; column++)
                column_flags_out[column] = ImGui::TableGetColumnFlags(column);
            float indent_step = (float)((int)TEXT_BASE_WIDTH / 2);
            for (int row = 0; row < 8; row++)
            {
                // Add some indentation to demonstrate usage of per-column IndentEnable/IndentDisable flags.
                ImGui::Indent(indent_step);
                ImGui::TableNextRow();
                for (int column = 0; column < column_count; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("%s %s", (column == 0) ? "インデント" : "こんにちは", ImGui::TableGetColumnName(column));
                }
            }
            ImGui::Unindent(indent_step * 8.0f);

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("カラムの幅"))
    {
        IMGUI_DEMO_MARKER("Tables/Columns widths");
        HelpMarker("TableSetupColumn() を使って既定の幅を設定する例です。");

        static ImGuiTableFlags flags1 = ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBodyUntilResize;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags1, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags1, ImGuiTableFlags_NoBordersInBodyUntilResize);
        PopStyleCompact();
        if (ImGui::BeginTable("table1", 3, flags1))
        {
            // We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
            ImGui::TableSetupColumn("1列目", ImGuiTableColumnFlags_WidthFixed, 100.0f); // Default to 100.0f
            ImGui::TableSetupColumn("2列目", ImGuiTableColumnFlags_WidthFixed, 200.0f); // Default to 200.0f
            ImGui::TableSetupColumn("3列目", ImGuiTableColumnFlags_WidthFixed);       // Default to auto
            ImGui::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    if (row == 0)
                        ImGui::Text("(w: %5.1f)", ImGui::GetContentRegionAvail().x);
                    else
                        ImGui::Text("こんにちは %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }

        HelpMarker(
            "TableSetupColumn() を使って幅を明示的に設定する例です。\n\n_NoKeepColumnsVisible が設定されていない限り、"
            "幅を指定した固定カラムでも、親に十分な空間がなければ縮められることがあります。");

        static ImGuiTableFlags flags2 = ImGuiTableFlags_None;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_NoKeepColumnsVisible", &flags2, ImGuiTableFlags_NoKeepColumnsVisible);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags2, ImGuiTableFlags_BordersInnerV);
        ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags2, ImGuiTableFlags_BordersOuterV);
        PopStyleCompact();
        if (ImGui::BeginTable("table2", 4, flags2))
        {
            // We could also set ImGuiTableFlags_SizingFixedFit on the table and then all columns
            // will default to ImGuiTableColumnFlags_WidthFixed.
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 30.0f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 4; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    if (row == 0)
                        ImGui::Text("(w: %5.1f)", ImGui::GetContentRegionAvail().x);
                    else
                        ImGui::Text("こんにちは %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("入れ子のテーブル"))
    {
        IMGUI_DEMO_MARKER("Tables/Nested tables");
        HelpMarker("テーブルのセルの中に別のテーブルを埋め込む例です。");

        if (ImGui::BeginTable("table_nested1", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
        {
            ImGui::TableSetupColumn("A0");
            ImGui::TableSetupColumn("A1");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            ImGui::Text("A0 行 0");
            {
                float rows_height = (TEXT_BASE_HEIGHT * 2.0f) + (ImGui::GetStyle().CellPadding.y * 2.0f);
                if (ImGui::BeginTable("table_nested2", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
                {
                    ImGui::TableSetupColumn("B0");
                    ImGui::TableSetupColumn("B1");
                    ImGui::TableHeadersRow();

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, rows_height);
                    ImGui::TableNextColumn();
                    ImGui::Text("B0 行 0");
                    ImGui::TableNextColumn();
                    ImGui::Text("B1 行 0");
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, rows_height);
                    ImGui::TableNextColumn();
                    ImGui::Text("B0 行 1");
                    ImGui::TableNextColumn();
                    ImGui::Text("B1 行 1");

                    ImGui::EndTable();
                }
            }
            ImGui::TableNextColumn(); ImGui::Text("A1 行 0");
            ImGui::TableNextColumn(); ImGui::Text("A0 行 1");
            ImGui::TableNextColumn(); ImGui::Text("A1 行 1");
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("行の高さ"))
    {
        IMGUI_DEMO_MARKER("Tables/Row height");
        HelpMarker(
            "TableNextRow() には 'min_row_height' を渡せます。\n\n行の上下には 'style.CellPadding.y' の余白が付くため、"
            "実際の最小行高は常に 'style.CellPadding.y * 2.0f' 以上になります。\n\n"
            "「最大」の行高は指定できません。行ごとに固有のクリッピング矩形が必要になるためです。");
        if (ImGui::BeginTable("table_row_height", 1, ImGuiTableFlags_Borders))
        {
            for (int row = 0; row < 8; row++)
            {
                float min_row_height = (float)(int)(TEXT_BASE_HEIGHT * 0.30f * row + ImGui::GetStyle().CellPadding.y * 2.0f);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, min_row_height);
                ImGui::TableNextColumn();
                ImGui::Text("min_row_height = %.2f", min_row_height);
            }
            ImGui::EndTable();
        }

        HelpMarker(
            "SameLine(0,0) を使って、セル間で現在の行の高さを共有する例です。\n\n"
            "テーブルの行の高さと「現在の行の高さ」は別物である点に注意してください。"
            "1つのセルが複数行を含むこともあるためです。");
        if (ImGui::BeginTable("table_share_lineheight", 2, ImGuiTableFlags_Borders))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::ColorButton("##1", ImVec4(0.13f, 0.26f, 0.40f, 1.0f), ImGuiColorEditFlags_None, ImVec2(40, 40));
            ImGui::TableNextColumn();
            ImGui::Text("1行目");
            ImGui::Text("2行目");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::ColorButton("##2", ImVec4(0.13f, 0.26f, 0.40f, 1.0f), ImGuiColorEditFlags_None, ImVec2(40, 40));
            ImGui::TableNextColumn();
            ImGui::SameLine(0.0f, 0.0f); // Reuse line height from previous column
            ImGui::Text("1行目 (SameLine(0,0) 使用)");
            ImGui::Text("2行目");

            ImGui::EndTable();
        }

        HelpMarker("行ごとに CellPadding.y を変える例です。CellPadding.x はテーブル全体で固定される点に注意してください。");
        if (ImGui::BeginTable("table_changing_cellpadding_y", 1, ImGuiTableFlags_Borders))
        {
            ImGuiStyle& style = ImGui::GetStyle();
            for (int row = 0; row < 8; row++)
            {
                if ((row % 3) == 2)
                    ImGui::PushStyleVarY(ImGuiStyleVar_CellPadding, 20.0f);
                ImGui::TableNextRow(ImGuiTableRowFlags_None);
                ImGui::TableNextColumn();
                ImGui::Text("CellPadding.y = %.2f", style.CellPadding.y);
                if ((row % 3) == 2)
                    ImGui::PopStyleVar();
            }
            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("外側のサイズ"))
    {
        IMGUI_DEMO_MARKER("Tables/Outer size");
        // Showcasing use of ImGuiTableFlags_NoHostExtendX and ImGuiTableFlags_NoHostExtendY
        // Important to that note how the two flags have slightly different behaviors!
        ImGui::Text("NoHostExtendX と NoHostExtendY を使う:");
        PushStyleCompact();
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
        ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
        ImGui::SameLine(); HelpMarker("outer_size.x の値を無視して、外側の幅をカラムに自動で合わせます。\n\nScrollX/ScrollY が無効で、ストレッチ列を使っていない場合にのみ利用できます。");
        ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendY", &flags, ImGuiTableFlags_NoHostExtendY);
        ImGui::SameLine(); HelpMarker("外側の高さを outer_size.y でぴったり止めます (テーブルが上限を超えて自動拡張するのを防ぎます)。\n\nScrollX/ScrollY が無効な場合にのみ利用できます。上限を超えたデータは切り取られて見えなくなります。");
        PopStyleCompact();

        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 5.5f);
        if (ImGui::BeginTable("table1", 3, flags, outer_size))
        {
            for (int row = 0; row < 10; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("セル %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::SameLine();
        ImGui::Text("こんにちは!");

        ImGui::Spacing();

        ImGui::Text("サイズを明示的に指定する:");
        if (ImGui::BeginTable("table2", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
        {
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("セル %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }
        ImGui::SameLine();
        if (ImGui::BeginTable("table3", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(TEXT_BASE_WIDTH * 30, 0.0f)))
        {
            const float rows_height = TEXT_BASE_HEIGHT * 1.5f + ImGui::GetStyle().CellPadding.y * 2.0f;
            for (int row = 0; row < 3; row++)
            {
                ImGui::TableNextRow(0, rows_height);
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("セル %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("背景色"))
    {
        IMGUI_DEMO_MARKER("Tables/Background color");
        static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
        static int row_bg_type = 1;
        static int row_bg_target = 1;
        static int cell_bg_type = 1;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_Borders", &flags, ImGuiTableFlags_Borders);
        ImGui::CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
        ImGui::SameLine(); HelpMarker("ImGuiTableFlags_RowBg を使うと、RowBg0 にスタイルから取得した交互の色が自動的に設定されます。");
        ImGui::Combo("行の背景タイプ", (int*)&row_bg_type, "なし\0赤\0グラデーション\0");
        ImGui::Combo("行の背景ターゲット", (int*)&row_bg_target, "RowBg0\0RowBg1\0"); ImGui::SameLine(); HelpMarker("RowBg0 を対象にすると奇数行/偶数行の交互色を上書きし、\nRowBg1 を対象にするとその色に重ねて合成します。");
        ImGui::Combo("セルの背景タイプ", (int*)&cell_bg_type, "なし\0青\0"); ImGui::SameLine(); HelpMarker("ここでは B1->C2 のセルに色を付けています。");
        IM_ASSERT(row_bg_type >= 0 && row_bg_type <= 2);
        IM_ASSERT(row_bg_target >= 0 && row_bg_target <= 1);
        IM_ASSERT(cell_bg_type >= 0 && cell_bg_type <= 1);
        PopStyleCompact();

        if (ImGui::BeginTable("table1", 5, flags))
        {
            for (int row = 0; row < 6; row++)
            {
                ImGui::TableNextRow();

                // Demonstrate setting a row background color with 'ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBgX, ...)'
                // We use a transparent color so we can see the one behind in case our target is RowBg1 and RowBg0 was already targeted by the ImGuiTableFlags_RowBg flag.
                if (row_bg_type != 0)
                {
                    ImU32 row_bg_color = ImGui::GetColorU32(row_bg_type == 1 ? ImVec4(0.7f, 0.3f, 0.3f, 0.65f) : ImVec4(0.2f + row * 0.1f, 0.2f, 0.2f, 0.65f)); // Flat or Gradient?
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0 + row_bg_target, row_bg_color);
                }

                // Fill cells
                for (int column = 0; column < 5; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("%c%c", 'A' + row, '0' + column);

                    // Change background of Cells B1->C2
                    // Demonstrate setting a cell background color with 'ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ...)'
                    // (the CellBg color will be blended over the RowBg and ColumnBg colors)
                    // We can also pass a column number as a third parameter to TableSetBgColor() and do this outside the column loop.
                    if (row >= 1 && row <= 2 && column >= 1 && column <= 2 && cell_bg_type == 1)
                    {
                        ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("ツリービュー"))
    {
        IMGUI_DEMO_MARKER("Tables/Tree view");
        static ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

        static ImGuiTreeNodeFlags tree_node_flags_base = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesFull;
        ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanFullWidth",  &tree_node_flags_base, ImGuiTreeNodeFlags_SpanFullWidth);
        ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanLabelWidth",  &tree_node_flags_base, ImGuiTreeNodeFlags_SpanLabelWidth);
        ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanAllColumns", &tree_node_flags_base, ImGuiTreeNodeFlags_SpanAllColumns);
        ImGui::CheckboxFlags("ImGuiTreeNodeFlags_LabelSpanAllColumns", &tree_node_flags_base, ImGuiTreeNodeFlags_LabelSpanAllColumns);
        ImGui::SameLine(); HelpMarker("他のカラムに内容を表示しないと分かっている場合に便利です");

        HelpMarker("カラムごとにインデントをどう適用するかは「カラムのフラグ」のセクションで設定できます。");
        if (ImGui::BeginTable("3ways", 3, table_flags))
        {
            // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
            ImGui::TableSetupColumn("名前", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("サイズ", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
            ImGui::TableSetupColumn("種類", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
            ImGui::TableHeadersRow();

            // Simple storage to output a dummy file-system.
            struct MyTreeNode
            {
                const char*     Name;
                const char*     Type;
                int             Size;
                int             ChildIdx;
                int             ChildCount;
                static void DisplayNode(const MyTreeNode* node, const MyTreeNode* all_nodes)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    const bool is_folder = (node->ChildCount > 0);

                    ImGuiTreeNodeFlags node_flags = tree_node_flags_base;
                    if (node != &all_nodes[0])
                        node_flags &= ~ImGuiTreeNodeFlags_LabelSpanAllColumns; // Only demonstrate this on the root node.

                    if (is_folder)
                    {
                        bool open = ImGui::TreeNodeEx(node->Name, node_flags);
                        if ((node_flags & ImGuiTreeNodeFlags_LabelSpanAllColumns) == 0)
                        {
                            ImGui::TableNextColumn();
                            ImGui::TextDisabled("--");
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(node->Type);
                        }
                        if (open)
                        {
                            for (int child_n = 0; child_n < node->ChildCount; child_n++)
                                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
                            ImGui::TreePop();
                        }
                    }
                    else
                    {
                        ImGui::TreeNodeEx(node->Name, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", node->Size);
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(node->Type);
                    }
                }
            };
            static const MyTreeNode nodes[] =
            {
                { "長い名前のルート",          "フォルダー",       -1,       1, 3    }, // 0
                { "音楽",                        "フォルダー",       -1,       4, 2    }, // 1
                { "テクスチャ",                     "フォルダー",       -1,       6, 3    }, // 2
                { "desktop.ini",                  "システムファイル",  1024,    -1,-1    }, // 3
                { "File1_a.wav",                  "音声ファイル",   123000,  -1,-1    }, // 4
                { "File1_b.wav",                  "音声ファイル",   456000,  -1,-1    }, // 5
                { "Image001.png",                 "画像ファイル",   203128,  -1,-1    }, // 6
                { "Copy of Image001.png",         "画像ファイル",   203256,  -1,-1    }, // 7
                { "Copy of Image001 (Final2).png","画像ファイル",   203512,  -1,-1    }, // 8
            };

            MyTreeNode::DisplayNode(&nodes[0], nodes);

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("項目の幅"))
    {
        IMGUI_DEMO_MARKER("Tables/Item width");
        HelpMarker(
            "PushItemWidth() の使い方と、それがカラムごとに保持される様子を示します。\n\n"
            "リサイズ不可で自動サイズの固定カラムでは、右揃えなどのために内容の幅を"
            "問い合わせてもあまり意味がない点に注意してください。");
        if (ImGui::BeginTable("table_item_width", 3, ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("小さめ");
            ImGui::TableSetupColumn("半分");
            ImGui::TableSetupColumn("右揃え");
            ImGui::TableHeadersRow();

            for (int row = 0; row < 3; row++)
            {
                ImGui::TableNextRow();
                if (row == 0)
                {
                    // Setup ItemWidth once (instead of setting up every time, which is also possible but less efficient)
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(TEXT_BASE_WIDTH * 3.0f); // Small
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-ImGui::GetContentRegionAvail().x * 0.5f);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushItemWidth(-FLT_MIN); // Right-aligned
                }

                // Draw our contents
                static float dummy_f = 0.0f;
                ImGui::PushID(row);
                ImGui::TableSetColumnIndex(0);
                ImGui::SliderFloat("float0", &dummy_f, 0.0f, 1.0f);
                ImGui::TableSetColumnIndex(1);
                ImGui::SliderFloat("float1", &dummy_f, 0.0f, 1.0f);
                ImGui::TableSetColumnIndex(2);
                ImGui::SliderFloat("##float2", &dummy_f, 0.0f, 1.0f); // No visible label since right-aligned
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // Demonstrate using TableHeader() calls instead of TableHeadersRow()
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("カスタムヘッダー"))
    {
        IMGUI_DEMO_MARKER("Tables/Custom headers");
        const int COLUMNS_COUNT = 3;
        if (ImGui::BeginTable("table_custom_headers", COLUMNS_COUNT, ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
        {
            ImGui::TableSetupColumn("アンズ");
            ImGui::TableSetupColumn("バナナ");
            ImGui::TableSetupColumn("サクランボ");

            // Dummy entire-column selection storage
            // FIXME: It would be nice to actually demonstrate full-featured selection using those checkbox.
            static bool column_selected[3] = {};

            // Instead of calling TableHeadersRow() we'll submit custom headers ourselves.
            // (A different approach is also possible:
            //    - Specify ImGuiTableColumnFlags_NoHeaderLabel in some TableSetupColumn() call.
            //    - Call TableHeadersRow() normally. This will submit TableHeader() with no name.
            //    - Then call TableSetColumnIndex() to position yourself in the column and submit your stuff e.g. Checkbox().)
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for (int column = 0; column < COLUMNS_COUNT; column++)
            {
                ImGui::TableSetColumnIndex(column);
                const char* column_name = ImGui::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
                ImGui::PushID(column);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::Checkbox("##checkall", &column_selected[column]);
                ImGui::PopStyleVar();
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::TableHeader(column_name);
                ImGui::PopID();
            }

            // Submit table contents
            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    char buf[32];
                    sprintf(buf, "セル %d,%d", column, row);
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Selectable(buf, column_selected[column]);
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // Demonstrate using ImGuiTableColumnFlags_AngledHeader flag to create angled headers
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("斜めヘッダー"))
    {
        IMGUI_DEMO_MARKER("Tables/Angled headers");
        const char* column_names[] = { "トラック", "cabasa", "ride", "smash", "tom-hi", "tom-mid", "tom-low", "hihat-o", "hihat-c", "snare-s", "snare-c", "clap", "rim", "kick" };
        const int columns_count = IM_COUNTOF(column_names);
        const int rows_count = 12;

        static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_HighlightHoveredColumn;
        static ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;
        static bool bools[columns_count * rows_count] = {}; // Dummy storage selection storage
        static int frozen_cols = 1;
        static int frozen_rows = 2;
        ImGui::CheckboxFlags("_ScrollX", &table_flags, ImGuiTableFlags_ScrollX);
        ImGui::CheckboxFlags("_ScrollY", &table_flags, ImGuiTableFlags_ScrollY);
        ImGui::CheckboxFlags("_Resizable", &table_flags, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("_Sortable", &table_flags, ImGuiTableFlags_Sortable);
        ImGui::CheckboxFlags("_NoBordersInBody", &table_flags, ImGuiTableFlags_NoBordersInBody);
        ImGui::CheckboxFlags("_HighlightHoveredColumn", &table_flags, ImGuiTableFlags_HighlightHoveredColumn);
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        ImGui::SliderInt("固定するカラム数", &frozen_cols, 0, 2);
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        ImGui::SliderInt("固定する行数", &frozen_rows, 0, 2);
        ImGui::CheckboxFlags("ヘッダーをカラム幅の計算に含めない", &column_flags, ImGuiTableColumnFlags_NoHeaderWidth);

        if (ImGui::TreeNode("スタイル設定"))
        {
            ImGui::SameLine();
            HelpMarker("便利のため、このデモから一部の ImGuiStyle の値を操作できるようにしています。");
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::SliderAngle("style.TableAngledHeadersAngle", &ImGui::GetStyle().TableAngledHeadersAngle, -50.0f, +50.0f);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::SliderFloat2("style.TableAngledHeadersTextAlign", (float*)&ImGui::GetStyle().TableAngledHeadersTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::TreePop();
        }

        if (ImGui::BeginTable("table_angled_headers", columns_count, table_flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 12)))
        {
            ImGui::TableSetupColumn(column_names[0], ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder);
            for (int n = 1; n < columns_count; n++)
                ImGui::TableSetupColumn(column_names[n], column_flags);
            ImGui::TableSetupScrollFreeze(frozen_cols, frozen_rows);

            ImGui::TableAngledHeadersRow(); // Draw angled headers for all columns with the ImGuiTableColumnFlags_AngledHeader flag.
            ImGui::TableHeadersRow();       // Draw remaining headers and allow access to context-menu and other functions.
            for (int row = 0; row < rows_count; row++)
            {
                ImGui::PushID(row);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("トラック %d", row);
                for (int column = 1; column < columns_count; column++)
                    if (ImGui::TableSetColumnIndex(column))
                    {
                        ImGui::PushID(column);
                        ImGui::Checkbox("", &bools[row * columns_count + column]);
                        ImGui::PopID();
                    }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // Demonstrate creating custom context menus inside columns,
    // while playing it nice with context menus provided by TableHeadersRow()/TableHeader()
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("コンテキストメニュー"))
    {
        IMGUI_DEMO_MARKER("Tables/Context menus");
        HelpMarker(
            "既定では、TableHeadersRow()/TableHeader() の行を右クリックすると標準のコンテキストメニューが開きます。\n"
            "ImGuiTableFlags_ContextMenuInBody を使うと、カラムの本体を右クリックしたときにも開けます。");
        static ImGuiTableFlags flags1 = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders | ImGuiTableFlags_ContextMenuInBody;

        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_ContextMenuInBody", &flags1, ImGuiTableFlags_ContextMenuInBody);
        PopStyleCompact();

        // Context Menus: first example
        // [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
        // [1.2] Right-click in columns also open the default table context menu (if ImGuiTableFlags_ContextMenuInBody is set)
        const int COLUMNS_COUNT = 3;
        if (ImGui::BeginTable("table_context_menu", COLUMNS_COUNT, flags1))
        {
            ImGui::TableSetupColumn("1列目");
            ImGui::TableSetupColumn("2列目");
            ImGui::TableSetupColumn("3列目");

            // [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
            ImGui::TableHeadersRow();

            // Submit dummy contents
            for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < COLUMNS_COUNT; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("セル %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }

        // Context Menus: second example
        // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
        // [2.2] Right-click on the ".." to open a custom popup
        // [2.3] Right-click in columns to open another custom popup
        HelpMarker(
            "テーブルのコンテキストメニュー (ヘッダー上)、項目のコンテキストメニュー (ボタン上)、"
            "カラムごとのカスタムコンテキストメニュー (カラム本体上) を組み合わせる例です。");
        ImGuiTableFlags flags2 = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders;
        if (ImGui::BeginTable("table_context_menu_2", COLUMNS_COUNT, flags2))
        {
            ImGui::TableSetupColumn("1列目");
            ImGui::TableSetupColumn("2列目");
            ImGui::TableSetupColumn("3列目");

            // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
            ImGui::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < COLUMNS_COUNT; column++)
                {
                    // Submit dummy contents
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("セル %d,%d", column, row);
                    ImGui::SameLine();

                    // [2.2] Right-click on the ".." to open a custom popup
                    ImGui::PushID(row * COLUMNS_COUNT + column);
                    ImGui::SmallButton("..");
                    if (ImGui::BeginPopupContextItem())
                    {
                        ImGui::Text("これはセル %d,%d の Button(\"..\") 用のポップアップです", column, row);
                        if (ImGui::Button("閉じる"))
                            ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }

            // [2.3] Right-click anywhere in columns to open another custom popup
            // (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with ImGuiPopupFlags_NoOpenOverExistingPopup
            // to manage popup priority as the popups triggers, here "are we hovering a column" are overlapping)
            int hovered_column = -1;
            for (int column = 0; column < COLUMNS_COUNT + 1; column++)
            {
                ImGui::PushID(column);
                if (ImGui::TableGetColumnFlags(column) & ImGuiTableColumnFlags_IsHovered)
                    hovered_column = column;
                if (hovered_column == column && !ImGui::IsAnyItemHovered() && ImGui::IsMouseReleased(1))
                    ImGui::OpenPopup("MyPopup");
                if (ImGui::BeginPopup("MyPopup"))
                {
                    if (column == COLUMNS_COUNT)
                        ImGui::Text("これは最後のカラムより後ろの未使用スペース用のカスタムポップアップです。");
                    else
                        ImGui::Text("これはカラム %d 用のカスタムポップアップです", column);
                    if (ImGui::Button("閉じる"))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
            ImGui::Text("ホバー中のカラム: %d", hovered_column);
        }
        ImGui::TreePop();
    }

    // Demonstrate creating multiple tables with the same ID
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("同期されたインスタンス"))
    {
        IMGUI_DEMO_MARKER("Tables/Synced instances");
        HelpMarker("同じ識別子を持つ複数のテーブルは、設定・幅・表示状態・順序などを共有します。");

        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings;
        ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
        ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
        ImGui::CheckboxFlags("ImGuiTableFlags_SizingFixedFit", &flags, ImGuiTableFlags_SizingFixedFit);
        ImGui::CheckboxFlags("ImGuiTableFlags_HighlightHoveredColumn", &flags, ImGuiTableFlags_HighlightHoveredColumn);
        for (int n = 0; n < 3; n++)
        {
            char buf[32];
            sprintf(buf, "同期テーブル %d", n);
            bool open = ImGui::CollapsingHeader(buf, ImGuiTreeNodeFlags_DefaultOpen);
            if (open && ImGui::BeginTable("Table", 3, flags, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 5)))
            {
                ImGui::TableSetupColumn("1列目");
                ImGui::TableSetupColumn("2列目");
                ImGui::TableSetupColumn("3列目");
                ImGui::TableHeadersRow();
                const int cell_count = (n == 1) ? 27 : 9; // Make second table have a scrollbar to verify that additional decoration is not affecting column positions.
                for (int cell = 0; cell < cell_count; cell++)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("このセル %d", cell);
                }
                ImGui::EndTable();
            }
        }
        ImGui::TreePop();
    }

    // Demonstrate using Sorting facilities
    // This is a simplified version of the "Advanced" example, where we mostly focus on the code necessary to handle sorting.
    // Note that the "Advanced" example also showcase manually triggering a sort (e.g. if item quantities have been modified)
    static const char* template_items_names[] =
    {
        "バナナ", "リンゴ", "サクランボ", "スイカ", "グレープフルーツ", "イチゴ", "マンゴー",
        "キウイ", "オレンジ", "パイナップル", "ブルーベリー", "スモモ", "ココナッツ", "洋ナシ", "アンズ"
    };
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("並べ替え"))
    {
        IMGUI_DEMO_MARKER("Tables/Sorting");
        // Create item list
        static ImVector<MyItem> items;
        if (items.Size == 0)
        {
            items.resize(50, MyItem());
            for (int n = 0; n < items.Size; n++)
            {
                const int template_n = n % IM_COUNTOF(template_items_names);
                MyItem& item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (n * n - n) % 20; // Assign default quantities
            }
        }

        // Options
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollY;
        PushStyleCompact();
        ImGui::CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
        ImGui::SameLine(); HelpMarker("並べ替えが有効な場合: Shift を押しながらヘッダーをクリックすると複数カラムで並べ替えできます。TableGetSortSpecs() が (SpecsCount > 1) の情報を返すことがあります。");
        ImGui::CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
        ImGui::SameLine(); HelpMarker("並べ替えが有効な場合: 並べ替えなしの状態も許可し、既定の並べ替えを無効にします。TableGetSortSpecs() が (SpecsCount == 0) の情報を返すことがあります。");
        PopStyleCompact();

        if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            // Demonstrate using a mixture of flags among available sort-related flags:
            // - ImGuiTableColumnFlags_DefaultSort
            // - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending / ImGuiTableColumnFlags_NoSortDescending
            // - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
            ImGui::TableSetupColumn("ID",       ImGuiTableColumnFlags_DefaultSort          | ImGuiTableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_ID);
            ImGui::TableSetupColumn("名前",                                                  ImGuiTableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Name);
            ImGui::TableSetupColumn("操作",   ImGuiTableColumnFlags_NoSort               | ImGuiTableColumnFlags_WidthFixed,   0.0f, MyItemColumnID_Action);
            ImGui::TableSetupColumn("数量", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Quantity);
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                if (sort_specs->SpecsDirty)
                {
                    MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                    sort_specs->SpecsDirty = false;
                }

            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    MyItem* item = &items[row_n];
                    ImGui::PushID(item->ID);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%04d", item->ID);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(item->Name);
                    ImGui::TableNextColumn();
                    ImGui::SmallButton("なし");
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item->Quantity);
                    ImGui::PopID();
                }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // In this example we'll expose most table flags and settings.
    // For specific flags and settings refer to the corresponding section for more detailed explanation.
    // This section is mostly useful to experiment with combining certain flags or settings with each others.
    //ImGui::SetNextItemOpen(true, ImGuiCond_Once); // [DEBUG]
    if (open_action != -1)
        ImGui::SetNextItemOpen(open_action != 0);
    if (ImGui::TreeNode("応用"))
    {
        IMGUI_DEMO_MARKER("Tables/Advanced");
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
            | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
            | ImGuiTableFlags_SizingFixedFit;
        static ImGuiTableColumnFlags columns_base_flags = ImGuiTableColumnFlags_None;

        enum ContentsType { CT_Text, CT_Button, CT_SmallButton, CT_FillButton, CT_Selectable, CT_SelectableSpanRow };
        static int contents_type = CT_SelectableSpanRow;
        const char* contents_type_names[] = { "テキスト", "ボタン", "小さいボタン", "塗りつぶしボタン", "選択項目", "選択項目 (行全体)" };
        static int freeze_cols = 1;
        static int freeze_rows = 1;
        static int items_count = IM_COUNTOF(template_items_names) * 2;
        static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
        static float row_min_height = 0.0f; // Auto
        static float inner_width_with_scroll = 0.0f; // Auto-extend
        static bool outer_size_enabled = true;
        static bool show_headers = true;
        static bool show_wrapped_text = false;
        //static ImGuiTextFilter filter;
        //ImGui::SetNextItemOpen(true, ImGuiCond_Once); // FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affect column sizing
        if (ImGui::TreeNode("オプション"))
        {
            // Make the UI compact because there are so many fields
            PushStyleCompact();
            ImGui::PushItemWidth(TEXT_BASE_WIDTH * 28.0f);

            if (ImGui::TreeNodeEx("機能:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
                ImGui::CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
                ImGui::CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
                ImGui::CheckboxFlags("ImGuiTableFlags_Sortable", &flags, ImGuiTableFlags_Sortable);
                ImGui::CheckboxFlags("ImGuiTableFlags_NoSavedSettings", &flags, ImGuiTableFlags_NoSavedSettings);
                ImGui::CheckboxFlags("ImGuiTableFlags_ContextMenuInBody", &flags, ImGuiTableFlags_ContextMenuInBody);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("装飾:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
                ImGui::CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
                ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags, ImGuiTableFlags_BordersOuterV);
                ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags, ImGuiTableFlags_BordersInnerV);
                ImGui::CheckboxFlags("ImGuiTableFlags_BordersH", &flags, ImGuiTableFlags_BordersH);
                ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterH", &flags, ImGuiTableFlags_BordersOuterH);
                ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerH", &flags, ImGuiTableFlags_BordersInnerH);
                ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody); ImGui::SameLine(); HelpMarker("本体のカラムの縦線を無効にします (ヘッダーには常に表示されます)");
                ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags, ImGuiTableFlags_NoBordersInBodyUntilResize); ImGui::SameLine(); HelpMarker("リサイズのためにホバーするまで、本体のカラムの縦線を表示しません (ヘッダーには常に表示されます)");
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("サイズ調整:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                EditTableSizingFlags(&flags);
                ImGui::SameLine(); HelpMarker("この「応用」デモではカラムごとにポリシーを上書きしているため、テーブル全体のこれらの設定は通常より効果が小さくなります。");
                ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
                ImGui::SameLine(); HelpMarker("outer_size.x の値を無視して、外側の幅をカラムに自動で合わせます。\n\nScrollX/ScrollY が無効で、ストレッチ列を使っていない場合にのみ利用できます。");
                ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendY", &flags, ImGuiTableFlags_NoHostExtendY);
                ImGui::SameLine(); HelpMarker("外側の高さを outer_size.y でぴったり止めます (テーブルが上限を超えて自動拡張するのを防ぎます)。\n\nScrollX/ScrollY が無効な場合にのみ利用できます。上限を超えたデータは切り取られて見えなくなります。");
                ImGui::CheckboxFlags("ImGuiTableFlags_NoKeepColumnsVisible", &flags, ImGuiTableFlags_NoKeepColumnsVisible);
                ImGui::SameLine(); HelpMarker("ScrollX が無効な場合にのみ利用できます。");
                ImGui::CheckboxFlags("ImGuiTableFlags_PreciseWidths", &flags, ImGuiTableFlags_PreciseWidths);
                ImGui::SameLine(); HelpMarker("余った幅をストレッチ列に分配する処理を無効にします (幅 100 のテーブルに3カラムある場合の幅の割り当て: このフラグなし: 33,33,34。あり: 33,33,33)。カラム数が多いと、リサイズの動きが滑らかでなくなります。");
                ImGui::CheckboxFlags("ImGuiTableFlags_NoClip", &flags, ImGuiTableFlags_NoClip);
                ImGui::SameLine(); HelpMarker("カラムごとのクリッピング矩形を無効にします (描画コマンド数が減り、項目が他のカラムにはみ出せるようになります)。通常 ScrollFreeze 系のオプションとは併用できません。");
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("余白:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::CheckboxFlags("ImGuiTableFlags_PadOuterX", &flags, ImGuiTableFlags_PadOuterX);
                ImGui::CheckboxFlags("ImGuiTableFlags_NoPadOuterX", &flags, ImGuiTableFlags_NoPadOuterX);
                ImGui::CheckboxFlags("ImGuiTableFlags_NoPadInnerX", &flags, ImGuiTableFlags_NoPadInnerX);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("スクロール:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
                ImGui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
                ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
                ImGui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("並べ替え:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
                ImGui::SameLine(); HelpMarker("並べ替えが有効な場合: Shift を押しながらヘッダーをクリックすると複数カラムで並べ替えできます。TableGetSortSpecs() が (SpecsCount > 1) の情報を返すことがあります。");
                ImGui::CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
                ImGui::SameLine(); HelpMarker("並べ替えが有効な場合: 並べ替えなしの状態も許可し、既定の並べ替えを無効にします。TableGetSortSpecs() が (SpecsCount == 0) の情報を返すことがあります。");
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("ヘッダー:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("show_headers", &show_headers);
                ImGui::CheckboxFlags("ImGuiTableFlags_HighlightHoveredColumn", &flags, ImGuiTableFlags_HighlightHoveredColumn);
                ImGui::CheckboxFlags("ImGuiTableColumnFlags_AngledHeader", &columns_base_flags, ImGuiTableColumnFlags_AngledHeader);
                ImGui::SameLine(); HelpMarker("すべてのカラムで AngledHeader を有効にします。幅の狭いカラムを選んで有効にするのが最適です (デモの「斜めヘッダー」セクションを参照)。");
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("その他:", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("show_wrapped_text", &show_wrapped_text);

                ImGui::DragFloat2("##OuterSize", &outer_size_value.x);
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Checkbox("outer_size", &outer_size_enabled);
                ImGui::SameLine();
                HelpMarker("スクロールが無効な場合 (ScrollX と ScrollY のどちらも未設定):\n"
                    "- テーブルは親ウィンドウに直接出力されます。\n"
                    "- OuterSize.x < 0.0f にするとテーブルが右揃えになります。\n"
                    "- OuterSize.x = 0.0f にすると、ストレッチ列がない限りテーブルは内容に合わせて狭くなります。\n"
                    "- そのとき OuterSize.y はテーブルの最小サイズとなり、行数が多ければ縦に伸びます (NoHostExtendY が設定されている場合を除く)。");

                // From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
                // To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
                ImGui::DragFloat("inner_width (ScrollX 有効時)", &inner_width_with_scroll, 1.0f, 0.0f, FLT_MAX);

                ImGui::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
                ImGui::SameLine(); HelpMarker("Selectable 項目の高さを指定します。");

                ImGui::DragInt("items_count", &items_count, 0.1f, 0, 9999);
                ImGui::Combo("items_type (先頭カラム)", &contents_type, contents_type_names, IM_COUNTOF(contents_type_names));
                //filter.Draw("filter");
                ImGui::TreePop();
            }

            ImGui::PopItemWidth();
            PopStyleCompact();
            ImGui::Spacing();
            ImGui::TreePop();
        }

        // Update item list if we changed the number of items
        static ImVector<MyItem> items;
        static ImVector<int> selection;
        static bool items_need_sort = false;
        if (items.Size != items_count)
        {
            items.resize(items_count, MyItem());
            for (int n = 0; n < items_count; n++)
            {
                const int template_n = n % IM_COUNTOF(template_items_names);
                MyItem& item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (template_n == 3) ? 10 : (template_n == 4) ? 20 : 0; // Assign default quantities
            }
        }

        const ImDrawList* parent_draw_list = ImGui::GetWindowDrawList();
        const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
        ImVec2 table_scroll_cur, table_scroll_max; // For debug display
        const ImDrawList* table_draw_list = NULL;  // "

        // Submit table
        const float inner_width_to_use = (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
        if (ImGui::BeginTable("table_advanced", 6, flags, outer_size_enabled ? outer_size_value : ImVec2(0, 0), inner_width_to_use))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            ImGui::TableSetupColumn("ID",           columns_base_flags | ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
            ImGui::TableSetupColumn("名前",         columns_base_flags | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
            ImGui::TableSetupColumn("操作",       columns_base_flags | ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
            ImGui::TableSetupColumn("数量",     columns_base_flags | ImGuiTableColumnFlags_PreferSortDescending, 0.0f, MyItemColumnID_Quantity);
            ImGui::TableSetupColumn("説明",  columns_base_flags | ((flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch), 0.0f, MyItemColumnID_Description);
            ImGui::TableSetupColumn("非表示",       columns_base_flags |  ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort);
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

            // Sort our data if sort specs have been changed!
            ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
            if (sort_specs && sort_specs->SpecsDirty)
                items_need_sort = true;
            if (sort_specs && items_need_sort && items.Size > 1)
            {
                MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                sort_specs->SpecsDirty = false;
            }
            items_need_sort = false;

            // Take note of whether we are currently sorting based on the Quantity field,
            // we will use this to trigger sorting when we know the data of this column has been modified.
            const bool sorts_specs_using_quantity = (ImGui::TableGetColumnFlags(3) & ImGuiTableColumnFlags_IsSorted) != 0;

            // Show headers
            if (show_headers && (columns_base_flags & ImGuiTableColumnFlags_AngledHeader) != 0)
                ImGui::TableAngledHeadersRow();
            if (show_headers)
                ImGui::TableHeadersRow();

            // Show data
            // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
#if 1
            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
            // Without clipper
            {
                for (int row_n = 0; row_n < items.Size; row_n++)
#endif
                {
                    MyItem* item = &items[row_n];
                    //if (!filter.PassFilter(item->Name))
                    //    continue;

                    const bool item_is_selected = selection.contains(item->ID);
                    ImGui::PushID(item->ID);
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

                    // For the demo purpose we can select among different type of items submitted in the first column
                    ImGui::TableSetColumnIndex(0);
                    char label[32];
                    sprintf(label, "%04d", item->ID);
                    if (contents_type == CT_Text)
                        ImGui::TextUnformatted(label);
                    else if (contents_type == CT_Button)
                        ImGui::Button(label);
                    else if (contents_type == CT_SmallButton)
                        ImGui::SmallButton(label);
                    else if (contents_type == CT_FillButton)
                        ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));
                    else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
                    {
                        ImGuiSelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap : ImGuiSelectableFlags_None;
                        if (ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, row_min_height)))
                        {
                            if (ImGui::GetIO().KeyCtrl)
                            {
                                if (item_is_selected)
                                    selection.find_erase_unsorted(item->ID);
                                else
                                    selection.push_back(item->ID);
                            }
                            else
                            {
                                selection.clear();
                                selection.push_back(item->ID);
                            }
                        }
                    }

                    if (ImGui::TableSetColumnIndex(1))
                        ImGui::TextUnformatted(item->Name);

                    // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                    // and we are currently sorting on the column showing the Quantity.
                    // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                    // You will probably need some extra logic if you want to automatically sort when a specific entry changes.
                    if (ImGui::TableSetColumnIndex(2))
                    {
                        if (ImGui::SmallButton("切る")) { item->Quantity += 1; }
                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("食べる")) { item->Quantity -= 1; }
                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                    }

                    if (ImGui::TableSetColumnIndex(3))
                        ImGui::Text("%d", item->Quantity);

                    ImGui::TableSetColumnIndex(4);
                    if (show_wrapped_text)
                        ImGui::TextWrapped("Lorem ipsum dolor sit amet");
                    else
                        ImGui::Text("Lorem ipsum dolor sit amet");

                    if (ImGui::TableSetColumnIndex(5))
                        ImGui::Text("1234");

                    ImGui::PopID();
                }
            }

            // Store some info to display debug details below
            table_scroll_cur = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
            table_scroll_max = ImVec2(ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY());
            table_draw_list = ImGui::GetWindowDrawList();
            ImGui::EndTable();
        }
        static bool show_debug_details = false;
        ImGui::Checkbox("デバッグの詳細", &show_debug_details);
        if (show_debug_details && table_draw_list)
        {
            ImGui::SameLine(0.0f, 0.0f);
            const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
            if (table_draw_list == parent_draw_list)
                ImGui::Text(": DrawCmd: +%d (同じウィンドウ内)",
                    table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
            else
                ImGui::Text(": DrawCmd: +%d (子ウィンドウ内), スクロール: (%.f/%.f) (%.f/%.f)",
                    table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();

    DemoWindowColumns();

    if (disable_indent)
        ImGui::PopStyleVar();
}

// Demonstrate old/legacy Columns API!
// [2020: Columns are under-featured and not maintained. Prefer using the more flexible and powerful BeginTable() API!]
static void DemoWindowColumns()
{
    bool open = ImGui::TreeNode("旧来の Columns API");
    ImGui::SameLine();
    HelpMarker("Columns() は古い API です! より柔軟で強力な BeginTable() API を使うことをおすすめします!");
    if (!open)
        return;

    // Basic columns
    if (ImGui::TreeNode("基本"))
    {
        IMGUI_DEMO_MARKER("Columns (legacy API)/Basic");
        ImGui::Text("枠線なし:");
        ImGui::Columns(3, "mycolumns3", false);  // 3-ways, no border
        ImGui::Separator();
        for (int n = 0; n < 14; n++)
        {
            char label[32];
            sprintf(label, "項目 %d", n);
            if (ImGui::Selectable(label)) {}
            //if (ImGui::Button(label, ImVec2(-FLT_MIN,0.0f))) {}
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::Text("枠線あり:");
        ImGui::Columns(4, "mycolumns"); // 4-ways, with border
        ImGui::Separator();
        ImGui::Text("ID"); ImGui::NextColumn();
        ImGui::Text("名前"); ImGui::NextColumn();
        ImGui::Text("パス"); ImGui::NextColumn();
        ImGui::Text("ホバー"); ImGui::NextColumn();
        ImGui::Separator();
        const char* names[3] = { "1つ目", "2つ目", "3つ目" };
        const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
        static int selected = -1;
        for (int i = 0; i < 3; i++)
        {
            char label[32];
            sprintf(label, "%04d", i);
            if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
                selected = i;
            bool hovered = ImGui::IsItemHovered();
            ImGui::NextColumn();
            ImGui::Text(names[i]); ImGui::NextColumn();
            ImGui::Text(paths[i]); ImGui::NextColumn();
            ImGui::Text("%d", hovered); ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("枠線"))
    {
        IMGUI_DEMO_MARKER("Columns (legacy API)/Borders");
        // NB: Future columns API should allow automatic horizontal borders.
        static bool h_borders = true;
        static bool v_borders = true;
        static int columns_count = 4;
        const int lines_count = 3;
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        ImGui::DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d カラム");
        if (columns_count < 2)
            columns_count = 2;
        ImGui::SameLine();
        ImGui::Checkbox("水平", &h_borders);
        ImGui::SameLine();
        ImGui::Checkbox("垂直", &v_borders);
        ImGui::Columns(columns_count, NULL, v_borders);
        for (int i = 0; i < columns_count * lines_count; i++)
        {
            if (h_borders && ImGui::GetColumnIndex() == 0)
                ImGui::Separator();
            ImGui::PushID(i);
            ImGui::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
            ImGui::Text("幅 %.2f", ImGui::GetColumnWidth());
            ImGui::Text("残り %.2f", ImGui::GetContentRegionAvail().x);
            ImGui::Text("オフセット %.2f", ImGui::GetColumnOffset());
            ImGui::Text("切り取られそうな長いテキスト");
            ImGui::Button("ボタン", ImVec2(-FLT_MIN, 0.0f));
            ImGui::PopID();
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        if (h_borders)
            ImGui::Separator();
        ImGui::TreePop();
    }

    // Create multiple items in a same cell before switching to next column
    if (ImGui::TreeNode("さまざまな項目の混在"))
    {
        IMGUI_DEMO_MARKER("Columns (legacy API)/Mixed items");
        ImGui::Columns(3, "mixed");
        ImGui::Separator();

        ImGui::Text("こんにちは");
        ImGui::Button("バナナ");
        ImGui::NextColumn();

        ImGui::Text("ImGui");
        ImGui::Button("リンゴ");
        static float foo = 1.0f;
        ImGui::InputFloat("赤", &foo, 0.05f, 0, "%.3f");
        ImGui::Text("ここに追加の1行。");
        ImGui::NextColumn();

        ImGui::Text("セーラーさん");
        ImGui::Button("ヤグルマギク");
        static float bar = 1.0f;
        ImGui::InputFloat("青", &bar, 0.05f, 0, "%.3f");
        ImGui::NextColumn();

        if (ImGui::CollapsingHeader("カテゴリ A")) { ImGui::Text("なんとかかんとか"); } ImGui::NextColumn();
        if (ImGui::CollapsingHeader("カテゴリ B")) { ImGui::Text("なんとかかんとか"); } ImGui::NextColumn();
        if (ImGui::CollapsingHeader("カテゴリ C")) { ImGui::Text("なんとかかんとか"); } ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }

    // Word wrapping
    if (ImGui::TreeNode("テキストの折り返し"))
    {
        IMGUI_DEMO_MARKER("Columns (legacy API)/Word-wrapping");
        ImGui::Columns(2, "word-wrapping");
        ImGui::Separator();
        ImGui::TextWrapped("素早い茶色のキツネがのんびり屋の犬を飛び越えます。");
        ImGui::TextWrapped("こんにちは (左)");
        ImGui::NextColumn();
        ImGui::TextWrapped("素早い茶色のキツネがのんびり屋の犬を飛び越えます。");
        ImGui::TextWrapped("こんにちは (右)");
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("横スクロール"))
    {
        IMGUI_DEMO_MARKER("Columns (legacy API)/Horizontal Scrolling");
        ImGui::SetNextWindowContentSize(ImVec2(1500.0f, 0.0f));
        ImVec2 child_size = ImVec2(0, ImGui::GetFontSize() * 20.0f);
        ImGui::BeginChild("##ScrollingRegion", child_size, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Columns(10);

        // Also demonstrate using clipper for large vertical lists
        int ITEMS_COUNT = 2000;
        ImGuiListClipper clipper;
        clipper.Begin(ITEMS_COUNT);
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                for (int j = 0; j < 10; j++)
                {
                    ImGui::Text("行 %d 列 %d...", i, j);
                    ImGui::NextColumn();
                }
        }
        ImGui::Columns(1);
        ImGui::EndChild();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("ツリー"))
    {
        IMGUI_DEMO_MARKER("Columns (legacy API)/Tree");
        ImGui::Columns(2, "tree", true);
        for (int x = 0; x < 3; x++)
        {
            bool open1 = ImGui::TreeNode((void*)(intptr_t)x, "ノード%d", x);
            ImGui::NextColumn();
            ImGui::Text("ノードの内容");
            ImGui::NextColumn();
            if (open1)
            {
                for (int y = 0; y < 3; y++)
                {
                    bool open2 = ImGui::TreeNode((void*)(intptr_t)y, "ノード%d.%d", x, y);
                    ImGui::NextColumn();
                    ImGui::Text("ノードの内容");
                    if (open2)
                    {
                        ImGui::Text("さらに内容");
                        if (ImGui::TreeNode("カラム内のツリー"))
                        {
                            ImGui::Text("素早い茶色のキツネがのんびり屋の犬を飛び越えます");
                            ImGui::TreePop();
                        }
                    }
                    ImGui::NextColumn();
                    if (open2)
                        ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::Columns(1);
        ImGui::TreePop();
    }

    ImGui::TreePop();
}

//-----------------------------------------------------------------------------
// [SECTION] DemoWindowInputs()
//-----------------------------------------------------------------------------

static void DemoWindowInputs()
{
    if (ImGui::CollapsingHeader("入力とフォーカス"))
    {
        ImGuiIO& io = ImGui::GetIO();

        // Display inputs submitted to ImGuiIO
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        bool inputs_opened = ImGui::TreeNode("入力");
        ImGui::SameLine();
        HelpMarker(
            "これは簡略表示です。より詳しい入力状態は次で確認できます:\n"
            "- 「ツール->メトリクス/デバッガー->Inputs」\n"
            "- 「ツール->デバッグログ->IO」");
        if (inputs_opened)
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Inputs");
            if (ImGui::IsMousePosValid())
                ImGui::Text("マウス位置: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                ImGui::Text("マウス位置: <無効>");
            ImGui::Text("マウス移動量: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            ImGui::Text("押されているボタン:");
            for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f 秒)", i, io.MouseDownDuration[i]); }
            ImGui::Text("ホイール: %.1f", io.MouseWheel);
            ImGui::Text("クリック回数:");
            for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++) if (io.MouseClickedCount[i] > 0) { ImGui::SameLine(); ImGui::Text("b%d: %d", i, io.MouseClickedCount[i]); }

            // We iterate both legacy native range and named ImGuiKey ranges. This is a little unusual/odd but this allows
            // displaying the data for old/new backends.
            // User code should never have to go through such hoops!
            // You can generally iterate between ImGuiKey_NamedKey_BEGIN and ImGuiKey_NamedKey_END.
            struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
            ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
            ImGui::Text("押されているキー:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }
            ImGui::Text("修飾キー: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
            ImGui::Text("文字キュー:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.

            ImGui::TreePop();
        }

        // Display ImGuiIO output flags
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        bool outputs_opened = ImGui::TreeNode("出力");
        ImGui::SameLine();
        HelpMarker(
            "io.WantCaptureMouse と io.WantCaptureKeyboard の値は通常 Dear ImGui が設定し、"
            "アプリケーションに入力の振り分け方を伝えます。値が true のときは、"
            "Dear ImGui がその入力を必要としているので、アプリ側では無視することが期待されます。\n\n"
            "もっとも典型的な例: ウィンドウにマウスを乗せると Dear ImGui は io.WantCaptureMouse を true にし、"
            "アプリ側はマウス入力を無視すべきです (実際には、これらのフラグの決まり方には"
            "もっと多くの細かい規則があります)。");
        if (outputs_opened)
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Outputs");
            ImGui::Text("io.WantCaptureMouse: %d", io.WantCaptureMouse);
            ImGui::Text("io.WantCaptureMouseUnlessPopupClose: %d", io.WantCaptureMouseUnlessPopupClose);
            ImGui::Text("io.WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
            ImGui::Text("io.WantTextInput: %d", io.WantTextInput);
            ImGui::Text("io.WantSetMousePos: %d", io.WantSetMousePos);
            ImGui::Text("io.NavActive: %d, io.NavVisible: %d", io.NavActive, io.NavVisible);

            IMGUI_DEMO_MARKER("Inputs & Focus/Outputs/WantCapture override");
            if (ImGui::TreeNode("WantCapture の上書き"))
            {
                HelpMarker(
                    "色の付いたキャンバスにマウスを乗せると io.WantCaptureXXX の値が上書きされます。\n"
                    "通常 (「なし」の場合) は、ホバー中は io.WantCaptureKeyboard が false で、"
                    "クリック時に true になることに注目してください。");
                static int capture_override_mouse = -1;
                static int capture_override_keyboard = -1;
                const char* capture_override_desc[] = { "なし", "false にする", "true にする" };
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 15);
                ImGui::SliderInt("SetNextFrameWantCaptureMouse() on hover", &capture_override_mouse, -1, +1, capture_override_desc[capture_override_mouse + 1], ImGuiSliderFlags_AlwaysClamp);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * 15);
                ImGui::SliderInt("SetNextFrameWantCaptureKeyboard() on hover", &capture_override_keyboard, -1, +1, capture_override_desc[capture_override_keyboard + 1], ImGuiSliderFlags_AlwaysClamp);

                ImGui::ColorButton("##panel", ImVec4(0.7f, 0.1f, 0.7f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(128.0f, 96.0f)); // Dummy item
                if (ImGui::IsItemHovered() && capture_override_mouse != -1)
                    ImGui::SetNextFrameWantCaptureMouse(capture_override_mouse == 1);
                if (ImGui::IsItemHovered() && capture_override_keyboard != -1)
                    ImGui::SetNextFrameWantCaptureKeyboard(capture_override_keyboard == 1);

                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        // Demonstrate using Shortcut() and Routing Policies.
        // The general flow is:
        // - Code interested in a chord (e.g. "Ctrl+A") declares their intent.
        // - Multiple locations may be interested in same chord! Routing helps find a winner.
        // - Every frame, we resolve all claims and assign one owner if the modifiers are matching.
        // - The lower-level function is 'bool SetShortcutRouting()', returns true when caller got the route.
        // - Most of the times, SetShortcutRouting() is not called directly. User mostly calls Shortcut() with routing flags.
        // - If you call Shortcut() WITHOUT any routing option, it uses ImGuiInputFlags_RouteFocused.
        // TL;DR: Most uses will simply be:
        // - Shortcut(ImGuiMod_Ctrl | ImGuiKey_A); // Use ImGuiInputFlags_RouteFocused policy.
        if (ImGui::TreeNode("ショートカット"))
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Shortcuts");
            static ImGuiInputFlags route_options = ImGuiInputFlags_Repeat;
            static ImGuiInputFlags route_type = ImGuiInputFlags_RouteFocused;
            ImGui::CheckboxFlags("ImGuiInputFlags_Repeat", &route_options, ImGuiInputFlags_Repeat);
            ImGui::RadioButton("ImGuiInputFlags_RouteActive", &route_type, ImGuiInputFlags_RouteActive);
            ImGui::RadioButton("ImGuiInputFlags_RouteFocused (default)", &route_type, ImGuiInputFlags_RouteFocused);
            ImGui::Indent();
            ImGui::BeginDisabled(route_type != ImGuiInputFlags_RouteFocused);
            ImGui::CheckboxFlags("ImGuiInputFlags_RouteOverActive##0", &route_options, ImGuiInputFlags_RouteOverActive);
            ImGui::EndDisabled();
            ImGui::Unindent();
            ImGui::RadioButton("ImGuiInputFlags_RouteGlobal", &route_type, ImGuiInputFlags_RouteGlobal);
            ImGui::Indent();
            ImGui::BeginDisabled(route_type != ImGuiInputFlags_RouteGlobal);
            ImGui::CheckboxFlags("ImGuiInputFlags_RouteOverFocused", &route_options, ImGuiInputFlags_RouteOverFocused);
            ImGui::CheckboxFlags("ImGuiInputFlags_RouteOverActive", &route_options, ImGuiInputFlags_RouteOverActive);
            ImGui::CheckboxFlags("ImGuiInputFlags_RouteUnlessBgFocused", &route_options, ImGuiInputFlags_RouteUnlessBgFocused);
            ImGui::EndDisabled();
            ImGui::Unindent();
            ImGui::RadioButton("ImGuiInputFlags_RouteAlways", &route_type, ImGuiInputFlags_RouteAlways);
            ImGuiInputFlags flags = route_type | route_options; // Merged flags
            if (route_type != ImGuiInputFlags_RouteGlobal)
                flags &= ~(ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteOverActive | ImGuiInputFlags_RouteUnlessBgFocused);

            ImGui::SeparatorText("Using SetNextItemShortcut()");
            ImGui::Text("Ctrl+S");
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_S, flags | ImGuiInputFlags_Tooltip);
            ImGui::Button("保存");
            ImGui::Text("Alt+F");
            ImGui::SetNextItemShortcut(ImGuiMod_Alt | ImGuiKey_F, flags | ImGuiInputFlags_Tooltip);
            static float f = 0.5f;
            ImGui::SliderFloat("係数", &f, 0.0f, 1.0f);

            ImGui::SeparatorText("Using Shortcut()");
            const float line_height = ImGui::GetTextLineHeightWithSpacing();
            const ImGuiKeyChord key_chord = ImGuiMod_Ctrl | ImGuiKey_A;

            ImGui::Text("Ctrl+A");
            ImGui::Text("IsWindowFocused: %d, Shortcut: %s", ImGui::IsWindowFocused(), ImGui::Shortcut(key_chord, flags) ? "押された" : "...");

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 1.0f, 0.1f));

            ImGui::BeginChild("WindowA", ImVec2(-FLT_MIN, line_height * 14), true);
            ImGui::Text("Ctrl+A を押して、どこが受け取るか見てみましょう!");
            ImGui::Separator();

            // 1: Window polling for Ctrl+A
            ImGui::Text("(WindowA 内)");
            ImGui::Text("IsWindowFocused: %d, Shortcut: %s", ImGui::IsWindowFocused(), ImGui::Shortcut(key_chord, flags) ? "押された" : "...");

            // 2: InputText also polling for Ctrl+A: it always uses _RouteFocused internally (gets priority when active)
            // (Commented because the owner-aware version of Shortcut() is still in imgui_internal.h)
            //char str[16] = "Press Ctrl+A";
            //ImGui::Spacing();
            //ImGui::InputText("InputTextB", str, IM_COUNTOF(str), ImGuiInputTextFlags_ReadOnly);
            //ImGuiID item_id = ImGui::GetItemID();
            //ImGui::SameLine(); HelpMarker("Internal widgets always use _RouteFocused");
            //ImGui::Text("IsWindowFocused: %d, Shortcut: %s", ImGui::IsWindowFocused(), ImGui::Shortcut(key_chord, flags, item_id) ? "PRESSED" : "...");

            // 3: Dummy child is not claiming the route: focusing them shouldn't steal route away from WindowA
            ImGui::BeginChild("ChildD", ImVec2(-FLT_MIN, line_height * 4), true);
            ImGui::Text("(ChildD 内: 同じショートカットを使っていない)");
            ImGui::Text("IsWindowFocused: %d", ImGui::IsWindowFocused());
            ImGui::EndChild();

            // 4: Child window polling for Ctrl+A. It is deeper than WindowA and gets priority when focused.
            ImGui::BeginChild("ChildE", ImVec2(-FLT_MIN, line_height * 4), true);
            ImGui::Text("(ChildE 内: 同じショートカットを使っている)");
            ImGui::Text("IsWindowFocused: %d, Shortcut: %s", ImGui::IsWindowFocused(), ImGui::Shortcut(key_chord, flags) ? "押された" : "...");
            ImGui::EndChild();

            // 5: In a popup
            if (ImGui::Button("ポップアップを開く"))
                ImGui::OpenPopup("PopupF");
            if (ImGui::BeginPopup("PopupF"))
            {
                ImGui::Text("(PopupF 内)");
                ImGui::Text("IsWindowFocused: %d, Shortcut: %s", ImGui::IsWindowFocused(), ImGui::Shortcut(key_chord, flags) ? "押された" : "...");
                // (Commented because the owner-aware version of Shortcut() is still in imgui_internal.h)
                //ImGui::InputText("InputTextG", str, IM_COUNTOF(str), ImGuiInputTextFlags_ReadOnly);
                //ImGui::Text("IsWindowFocused: %d, Shortcut: %s", ImGui::IsWindowFocused(), ImGui::Shortcut(key_chord, flags, ImGui::GetItemID()) ? "PRESSED" : "...");
                ImGui::EndPopup();
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::TreePop();
        }

        // Display mouse cursors
        if (ImGui::TreeNode("マウスカーソル"))
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Mouse Cursors");
            const char* mouse_cursors_names[] = { "Arrow", "TextInput", "ResizeAll", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE", "Hand", "Wait", "Progress", "NotAllowed" };
            IM_ASSERT(IM_COUNTOF(mouse_cursors_names) == ImGuiMouseCursor_COUNT);

            ImGuiMouseCursor current = ImGui::GetMouseCursor();
            const char* cursor_name = (current >= ImGuiMouseCursor_Arrow) && (current < ImGuiMouseCursor_COUNT) ? mouse_cursors_names[current] : "N/A";
            ImGui::Text("現在のマウスカーソル = %d: %s", current, cursor_name);
            ImGui::BeginDisabled(true);
            ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors", &io.BackendFlags, ImGuiBackendFlags_HasMouseCursors);
            ImGui::EndDisabled();

            ImGui::Text("マウスを乗せるとカーソルの種類が分かります:");
            ImGui::SameLine(); HelpMarker(
                "ImGui::GetMouseCursor() の戻り値に応じて、アプリ側で異なるマウスカーソルを描画できます。"
                "ソフトウェアカーソル描画 (io.MouseDrawCursor) が有効なら ImGui が適切なカーソルを描画しますが、"
                "そうでない場合はバックエンド側で対応する必要があります。");
            for (int i = 0; i < ImGuiMouseCursor_COUNT; i++)
            {
                char label[32];
                sprintf(label, "マウスカーソル %d: %s", i, mouse_cursors_names[i]);
                ImGui::Bullet(); ImGui::Selectable(label, false);
                if (ImGui::IsItemHovered())
                    ImGui::SetMouseCursor(i);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Tab による移動"))
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Tabbing");
            ImGui::Text("Tab / Shift+Tab でキーボード編集可能な項目を順に移動できます。");
            static char buf[32] = "こんにちは";
            ImGui::InputText("1", buf, IM_COUNTOF(buf));
            ImGui::InputText("2", buf, IM_COUNTOF(buf));
            ImGui::InputText("3", buf, IM_COUNTOF(buf));
            ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, true);
            ImGui::InputText("4 (Tab でスキップ)", buf, IM_COUNTOF(buf));
            ImGui::SameLine(); HelpMarker("TAB や Shift+Tab での移動時に、この項目は飛ばされます。");
            ImGui::PopItemFlag();
            ImGui::InputText("5", buf, IM_COUNTOF(buf));
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("コードからフォーカスを移す"))
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Focus from code");
            bool focus_1 = ImGui::Button("1 にフォーカス"); ImGui::SameLine();
            bool focus_2 = ImGui::Button("2 にフォーカス"); ImGui::SameLine();
            bool focus_3 = ImGui::Button("3 にフォーカス");
            int has_focus = 0;
            static char buf[128] = "ボタンをクリックするとフォーカスが移ります";

            if (focus_1) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("1", buf, IM_COUNTOF(buf));
            if (ImGui::IsItemActive()) has_focus = 1;

            if (focus_2) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("2", buf, IM_COUNTOF(buf));
            if (ImGui::IsItemActive()) has_focus = 2;

            ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, true);
            if (focus_3) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("3 (Tab でスキップ)", buf, IM_COUNTOF(buf));
            if (ImGui::IsItemActive()) has_focus = 3;
            ImGui::SameLine(); HelpMarker("TAB や Shift+Tab での移動時に、この項目は飛ばされます。");
            ImGui::PopItemFlag();

            if (has_focus)
                ImGui::Text("フォーカスのある項目: %d", has_focus);
            else
                ImGui::Text("フォーカスのある項目: <なし>");

            // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
            static float f3[3] = { 0.0f, 0.0f, 0.0f };
            int focus_ahead = -1;
            if (ImGui::Button("X にフォーカス")) { focus_ahead = 0; } ImGui::SameLine();
            if (ImGui::Button("Y にフォーカス")) { focus_ahead = 1; } ImGui::SameLine();
            if (ImGui::Button("Z にフォーカス")) { focus_ahead = 2; }
            if (focus_ahead != -1) ImGui::SetKeyboardFocusHere(focus_ahead);
            ImGui::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

            ImGui::TextWrapped("注: コードから最後に使った項目に再びフォーカスしても、カーソル位置と選択範囲は保持されます。");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("ドラッグ"))
        {
            IMGUI_DEMO_MARKER("Inputs & Focus/Dragging");
            ImGui::TextWrapped("任意のウィジェット上でのドラッグ量は ImGui::GetMouseDragDelta(0) で取得できます。");
            for (int button = 0; button < 3; button++)
            {
                ImGui::Text("IsMouseDragging(%d):", button);
                ImGui::Text("  既定のしきい値: %d,", ImGui::IsMouseDragging(button));
                ImGui::Text("  しきい値 0: %d,", ImGui::IsMouseDragging(button, 0.0f));
                ImGui::Text("  大きいしきい値: %d,", ImGui::IsMouseDragging(button, 20.0f));
            }

            ImGui::Button("ドラッグしてね");
            if (ImGui::IsItemActive())
                ImGui::GetForegroundDrawList()->AddLine(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f); // Draw a line between the button and the mouse cursor

            // Drag operations gets "unlocked" when the mouse has moved past a certain threshold
            // (the default threshold is stored in io.MouseDragThreshold). You can request a lower or higher
            // threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
            ImVec2 value_raw = ImGui::GetMouseDragDelta(0, 0.0f);
            ImVec2 value_with_lock_threshold = ImGui::GetMouseDragDelta(0);
            ImVec2 mouse_delta = io.MouseDelta;
            ImGui::Text("GetMouseDragDelta(0):");
            ImGui::Text("  既定のしきい値: (%.1f, %.1f)", value_with_lock_threshold.x, value_with_lock_threshold.y);
            ImGui::Text("  しきい値 0: (%.1f, %.1f)", value_raw.x, value_raw.y);
            ImGui::Text("io.MouseDelta: (%.1f, %.1f)", mouse_delta.x, mouse_delta.y);
            ImGui::TreePop();
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] About Window / ShowAboutWindow()
// Access from Dear ImGui Demo -> Tools -> About
//-----------------------------------------------------------------------------

void ImGui::ShowAboutWindow(bool* p_open)
{
    if (!ImGui::Begin("Dear ImGui について", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }
    IMGUI_DEMO_MARKER("Tools/About Dear ImGui");
    ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

    ImGui::TextLinkOpenURL("ホームページ", "https://github.com/ocornut/imgui");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("FAQ", "https://github.com/ocornut/imgui/blob/master/docs/FAQ.md");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("Wiki", "https://github.com/ocornut/imgui/wiki");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("拡張機能", "https://github.com/ocornut/imgui/wiki/Useful-Extensions");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("リリース", "https://github.com/ocornut/imgui/releases");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("支援について", "https://github.com/ocornut/imgui/wiki/Funding");

    ImGui::Separator();
    ImGui::Text("(c) 2014-2026 Omar Cornut");
    ImGui::Text("Omar Cornut と Dear ImGui のすべてのコントリビューターによって開発されています。");
    ImGui::Text("Dear ImGui は MIT ライセンスで提供されています。詳しくは LICENSE を参照してください。");
    ImGui::Text("会社で利用している場合は、プロジェクトへの支援もご検討ください。");

    static bool show_config_info = false;
    ImGui::Checkbox("設定/ビルド情報", &show_config_info);
    if (show_config_info)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        bool copy_to_clipboard = ImGui::Button("クリップボードにコピー");
        ImVec2 child_size = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18);
        ImGui::BeginChild(ImGui::GetID("cfg_infos"), child_size, ImGuiChildFlags_FrameStyle);
        if (copy_to_clipboard)
        {
            ImGui::LogToClipboard();
            ImGui::LogText("// (Copy from the next line. Keep the ``` markers for formatting.)\n");
            ImGui::LogText("```cpp\n"); // Back quotes will make text appears without formatting when pasting on GitHub
        }

        ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
        ImGui::Separator();
        ImGui::Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
        ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef IMGUI_ENABLE_TEST_ENGINE
        ImGui::Text("define: IMGUI_ENABLE_TEST_ENGINE");
#endif
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
        ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
        ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
        ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
        ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
        ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
        ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
        ImGui::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
        ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
        ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
        ImGui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
        ImGui::Text("define: __clang_version__=%s", __clang_version__);
#endif
#ifdef __EMSCRIPTEN__
        ImGui::Text("define: __EMSCRIPTEN__");
#ifdef __EMSCRIPTEN_MAJOR__
        ImGui::Text("Emscripten: %d.%d.%d", __EMSCRIPTEN_MAJOR__, __EMSCRIPTEN_MINOR__, __EMSCRIPTEN_TINY__);
#else
        ImGui::Text("Emscripten: %d.%d.%d", __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__);
#endif
#endif
#ifdef IMGUI_HAS_VIEWPORT
        ImGui::Text("define: IMGUI_HAS_VIEWPORT");
#endif
#ifdef IMGUI_HAS_DOCK
        ImGui::Text("define: IMGUI_HAS_DOCK");
#endif
#ifdef NDEBUG
        ImGui::Text("define: NDEBUG");
#endif

        // Heuristic to detect no-op IM_ASSERT() macros
        // - This is designed so people opening bug reports would convey and notice that they have disabled asserts for Dear ImGui code.
        // - 16 is > strlen("((void)(_EXPR))") which we suggested in our imconfig.h template as a possible way to disable.
        int assert_runs_expression = 0;
        IM_ASSERT(++assert_runs_expression);
        int assert_expand_len = (int)strlen(IM_STRINGIFY((IM_ASSERT(true))));
        bool assert_maybe_disabled = (!assert_runs_expression || assert_expand_len <= 16);
        ImGui::Text("IM_ASSERT: runs expression: %s. expand size: %s%s",
            assert_runs_expression ? "OK" : "KO", (assert_expand_len > 16) ? "OK" : "KO", assert_maybe_disabled ? " (MAYBE DISABLED?!)" : "");
        if (assert_maybe_disabled)
        {
            ImGui::SameLine();
            HelpMarker("IM_ASSERT() は既定で assert() を呼びます。NDEBUG を定義してコンパイルすると通常 assert() は取り除かれますが、プログラマーの間違いを知らせるためにアサートを使っているので推奨しません!");
        }

        ImGui::Separator();
        ImGui::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
        ImGui::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
        ImGui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        ImGui::Text(" NavEnableKeyboard");
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         ImGui::Text(" NavEnableGamepad");
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  ImGui::Text(" NoMouse");
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      ImGui::Text(" NoMouseCursorChange");
        if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)               ImGui::Text(" NoKeyboard");
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)            ImGui::Text(" DockingEnable");
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)          ImGui::Text(" ViewportsEnable");
        if (io.MouseDrawCursor)                                         ImGui::Text("io.MouseDrawCursor");
        if (io.ConfigDpiScaleFonts)                                     ImGui::Text("io.ConfigDpiScaleFonts");
        if (io.ConfigDpiScaleViewports)                                 ImGui::Text("io.ConfigDpiScaleViewports");
        if (io.ConfigViewportsNoAutoMerge)                              ImGui::Text("io.ConfigViewportsNoAutoMerge");
        if (io.ConfigViewportsNoTaskBarIcon)                            ImGui::Text("io.ConfigViewportsNoTaskBarIcon");
        if (io.ConfigViewportsNoDecoration)                             ImGui::Text("io.ConfigViewportsNoDecoration");
        if (io.ConfigViewportsNoDefaultParent)                          ImGui::Text("io.ConfigViewportsNoDefaultParent");
        if (io.ConfigDockingNoSplit)                                    ImGui::Text("io.ConfigDockingNoSplit");
        if (io.ConfigDockingNoDockingOver)                              ImGui::Text("io.ConfigDockingNoDockingOver");
        if (io.ConfigDockingWithShift)                                  ImGui::Text("io.ConfigDockingWithShift");
        if (io.ConfigDockingAlwaysTabBar)                               ImGui::Text("io.ConfigDockingAlwaysTabBar");
        if (io.ConfigDockingTransparentPayload)                         ImGui::Text("io.ConfigDockingTransparentPayload");
        if (io.ConfigMacOSXBehaviors)                                   ImGui::Text("io.ConfigMacOSXBehaviors");
        if (io.ConfigNavMoveSetMousePos)                                ImGui::Text("io.ConfigNavMoveSetMousePos");
        if (io.ConfigNavCaptureKeyboard)                                ImGui::Text("io.ConfigNavCaptureKeyboard");
        if (io.ConfigInputTextCursorBlink)                              ImGui::Text("io.ConfigInputTextCursorBlink");
        if (io.ConfigWindowsResizeFromEdges)                            ImGui::Text("io.ConfigWindowsResizeFromEdges");
        if (io.ConfigWindowsMoveFromTitleBarOnly)                       ImGui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
        if (io.ConfigMemoryCompactTimer >= 0.0f)                        ImGui::Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
        ImGui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
        if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             ImGui::Text(" HasGamepad");
        if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        ImGui::Text(" HasMouseCursors");
        if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         ImGui::Text(" HasSetMousePos");
        if (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports)   ImGui::Text(" PlatformHasViewports");
        if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)ImGui::Text(" HasMouseHoveredViewport");
        if (io.BackendFlags & ImGuiBackendFlags_HasParentViewport)      ImGui::Text(" HasParentViewport");
        if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)   ImGui::Text(" RendererHasVtxOffset");
        if (io.BackendFlags & ImGuiBackendFlags_RendererHasTextures)    ImGui::Text(" RendererHasTextures");
        if (io.BackendFlags & ImGuiBackendFlags_RendererHasViewports)   ImGui::Text(" RendererHasViewports");
        ImGui::Separator();
        ImGui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexData->Width, io.Fonts->TexData->Height);
        ImGui::Text("io.Fonts->FontLoaderName: %s", io.Fonts->FontLoaderName ? io.Fonts->FontLoaderName : "NULL");
        ImGui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
        ImGui::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui::Separator();
        ImGui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
        ImGui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
        ImGui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
        ImGui::Text("style.FrameRounding: %.2f", style.FrameRounding);
        ImGui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
        ImGui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
        ImGui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

        if (copy_to_clipboard)
        {
            ImGui::LogText("\n```\n");
            ImGui::LogFinish();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------
// - ShowStyleSelector()
// - ShowStyleEditor()
//-----------------------------------------------------------------------------

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
bool ImGui::ShowStyleSelector(const char* label)
{
    // FIXME: This is a bit tricky to get right as style are functions, they don't register a name nor the fact that one is active.
    // So we keep track of last active one among our limited selection.
    static int style_idx = -1;
    const char* style_names[] = { "ダーク", "ライト", "クラシック" };
    bool ret = false;
    if (ImGui::BeginCombo(label, (style_idx >= 0 && style_idx < IM_COUNTOF(style_names)) ? style_names[style_idx] : ""))
    {
        for (int n = 0; n < IM_COUNTOF(style_names); n++)
        {
            if (ImGui::Selectable(style_names[n], style_idx == n, ImGuiSelectableFlags_SelectOnNav))
            {
                style_idx = n;
                ret = true;
                switch (style_idx)
                {
                case 0: ImGui::StyleColorsDark(); break;
                case 1: ImGui::StyleColorsLight(); break;
                case 2: ImGui::StyleColorsClassic(); break;
                }
            }
            else if (style_idx == n)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return ret;
}

static const char* GetTreeLinesFlagsName(ImGuiTreeNodeFlags flags)
{
    if (flags == ImGuiTreeNodeFlags_DrawLinesNone) return "DrawLinesNone";
    if (flags == ImGuiTreeNodeFlags_DrawLinesFull) return "DrawLinesFull";
    if (flags == ImGuiTreeNodeFlags_DrawLinesToNodes) return "DrawLinesToNodes";
    return "";
}

// We omit the ImGui:: prefix in this function, as we don't expect user to be copy and pasting this code.
void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
    IMGUI_DEMO_MARKER("Tools/Style Editor");
    // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    ImGuiStyle& style = GetStyle();
    static ImGuiStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    // The logic behind dynamically changing 'max_border_size' is to not encourage people to increase border size too much: it'll likely reveal lots of subtle rendering artifacts and this isn't a priority right now.
    // Note that _MainScale is currently internal PLEASE DO NOT USE IN YOUR CODE.
    const float default_border_size = (float)(int)style._MainScale;
    const float max_border_size = IM_MAX(default_border_size, 2.0f);

    PushItemWidth(GetWindowWidth() * 0.50f);

    {
        // General
        SeparatorText("全般");
        if ((GetIO().BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0)
        {
            BulletText("警告: ImGuiBackendFlags_RendererHasTextures が設定されていないため、\nフォントの拡大縮小は滑らかになりません!");
            BulletText("手順については次を参照してください:");
            SameLine();
            TextLinkOpenURL("docs/BACKENDS.md", "https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md");
        }

        if (ShowStyleSelector("配色##Selector"))
            ref_saved_style = style;
        ShowFontSelector("フォント##Selector");
        if (DragFloat("FontSizeBase", &style.FontSizeBase, 0.20f, 5.0f, 100.0f, "%.0f"))
            style._NextFrameFontSizeBase = style.FontSizeBase; // FIXME: Temporary hack until we finish remaining work.
        SameLine(0.0f, 0.0f); Text(" (out %.2f)", GetFontSize());
        DragFloat("FontScaleMain", &style.FontScaleMain, 0.02f, 0.5f, 4.0f);
        BeginDisabled(GetIO().ConfigDpiScaleFonts);
        DragFloat("FontScaleDpi", &style.FontScaleDpi, 0.02f, 0.5f, 4.0f);
        SetItemTooltip("io.ConfigDpiScaleFonts が設定されている場合、この値は自動的に上書きされます。");
        EndDisabled();

        // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
        if (SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
            style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
        { bool border = (style.WindowBorderSize > 0.0f); if (Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? default_border_size : 0.0f; } }
        SameLine();
        { bool border = (style.FrameBorderSize > 0.0f);  if (Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? default_border_size : 0.0f; } }
        SameLine();
        { bool border = (style.PopupBorderSize > 0.0f);  if (Checkbox("PopupBorder", &border)) { style.PopupBorderSize = border ? default_border_size : 0.0f; } }
    }

    // Save/Revert button
    if (Button("参照を保存"))
        *ref = ref_saved_style = style;
    SameLine();
    if (Button("参照に戻す"))
        style = *ref;
    SameLine();
    HelpMarker(
        "保存/復元はメモリ上の一時的な領域に対して行われます。既定の配色定義は変更されません。"
        "どこかに保存したい場合は、下の「エクスポート」を使ってください。");

    SeparatorText("詳細");
    if (BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (BeginTabItem("サイズ"))
        {
            SeparatorText("基本");
            SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");

            SeparatorText("枠線");
            SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, max_border_size, "%.0f");

            SeparatorText("角の丸み");
            SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("MenuItemRounding", &style.MenuItemRounding, 0.0f, 12.0f, "%.0f");
            // NB: SelectableRounding is intentionally NOT made visible here. We don't want to encourage people using that.

            SeparatorText("スクロールバー");
            SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("ScrollbarPadding", &style.ScrollbarPadding, 0.0f, 10.0f, "%.0f");

            SeparatorText("タブ");
            SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("TabBarBorderSize", &style.TabBarBorderSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("TabBarOverlineSize", &style.TabBarOverlineSize, 0.0f, IM_MAX(3.0f, max_border_size), "%.0f");
            SameLine(); HelpMarker("オーバーラインは、ImGuiTabBarFlags_DrawSelectedOverline が設定されている場合に選択中のタブの上にだけ描かれます。");
            DragFloat("TabMinWidthBase", &style.TabMinWidthBase, 0.5f, 1.0f, 500.0f, "%.0f");
            DragFloat("TabMinWidthShrink", &style.TabMinWidthShrink, 0.5f, 1.0f, 500.0f, "%0.f");
            DragFloat("TabCloseButtonMinWidthSelected", &style.TabCloseButtonMinWidthSelected, 0.5f, -1.0f, 100.0f, (style.TabCloseButtonMinWidthSelected < 0.0f) ? "%.0f (常に表示)" : "%.0f");
            DragFloat("TabCloseButtonMinWidthUnselected", &style.TabCloseButtonMinWidthUnselected, 0.5f, -1.0f, 100.0f, (style.TabCloseButtonMinWidthUnselected < 0.0f) ? "%.0f (常に表示)" : "%.0f");
            SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");

            SeparatorText("テーブル");
            SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
            SliderAngle("TableAngledHeadersAngle", &style.TableAngledHeadersAngle, -50.0f, +50.0f);
            SliderFloat2("TableAngledHeadersTextAlign", (float*)&style.TableAngledHeadersTextAlign, 0.0f, 1.0f, "%.2f");

            SeparatorText("ツリー");
            bool combo_open = BeginCombo("TreeLinesFlags", GetTreeLinesFlagsName(style.TreeLinesFlags));
            SameLine();
            HelpMarker("[実験的機能] ツリーの線はすべての状況で正しく動くとは限らず (例: クリッパー使用時)、走査のわずかなオーバーヘッドが生じることがあります。\n\nImGuiTreeNodeFlags_DrawLinesFull は ImGuiTreeNodeFlags_DrawLinesToNode より高速です。");
            if (combo_open)
            {
                const ImGuiTreeNodeFlags options[] = { ImGuiTreeNodeFlags_DrawLinesNone, ImGuiTreeNodeFlags_DrawLinesFull, ImGuiTreeNodeFlags_DrawLinesToNodes };
                for (ImGuiTreeNodeFlags option : options)
                    if (Selectable(GetTreeLinesFlagsName(option), style.TreeLinesFlags == option))
                        style.TreeLinesFlags = option;
                EndCombo();
            }
            SliderFloat("TreeLinesSize", &style.TreeLinesSize, 0.0f, max_border_size, "%.0f");
            SliderFloat("TreeLinesRounding", &style.TreeLinesRounding, 0.0f, 12.0f, "%.0f");

            SeparatorText("ウィンドウ");
            SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            SliderFloat("WindowBorderHoverPadding", &style.WindowBorderHoverPadding, 1.0f, 20.0f, "%.0f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "なし\0左\0右\0"))
                style.WindowMenuButtonPosition = (ImGuiDir)(window_menu_button_position - 1);

            SeparatorText("ウィジェット");
            SliderFloat("ColorMarkerSize", &style.ColorMarkerSize, 0.0f, 8.0f, "%.0f");
            Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "左\0右\0");
            SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            SameLine(); HelpMarker("この配置は、ボタンがテキストより大きい場合に適用されます。");
            SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
            SameLine(); HelpMarker("この配置は、Selectable がテキストより大きい場合に適用されます。");
            SliderFloat("SeparatorSize", &style.SeparatorSize, 0.0f, 10.0f, "%.0f");
            SliderFloat("SeparatorTextBorderSize", &style.SeparatorTextBorderSize, 0.0f, 10.0f, "%.0f");
            SliderFloat2("SeparatorTextAlign", (float*)&style.SeparatorTextAlign, 0.0f, 1.0f, "%.2f");
            SliderFloat2("SeparatorTextPadding", (float*)&style.SeparatorTextPadding, 0.0f, 40.0f, "%.0f");
            SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
            SliderFloat("ImageRounding", &style.ImageRounding, 0.0f, 12.0f, "%.0f");
            SliderFloat("ImageBorderSize", &style.ImageBorderSize, 0.0f, max_border_size, "%.0f");

            SeparatorText("ドッキング");
            //SetCursorPosX(GetCursorPosX() + CalcItemWidth() - GetFrameHeight());
            Checkbox("DockingNodeHasCloseButton", &style.DockingNodeHasCloseButton);
            SliderFloat("DockingSeparatorSize", &style.DockingSeparatorSize, 0.0f, 12.0f, "%.0f");

            SeparatorText("ツールチップ");
            for (int n = 0; n < 2; n++)
                if (TreeNodeEx(n == 0 ? "HoverFlagsForTooltipMouse" : "HoverFlagsForTooltipNav"))
                {
                    ImGuiHoveredFlags* p = (n == 0) ? &style.HoverFlagsForTooltipMouse : &style.HoverFlagsForTooltipNav;
                    CheckboxFlags("ImGuiHoveredFlags_DelayNone", p, ImGuiHoveredFlags_DelayNone);
                    CheckboxFlags("ImGuiHoveredFlags_DelayShort", p, ImGuiHoveredFlags_DelayShort);
                    CheckboxFlags("ImGuiHoveredFlags_DelayNormal", p, ImGuiHoveredFlags_DelayNormal);
                    CheckboxFlags("ImGuiHoveredFlags_Stationary", p, ImGuiHoveredFlags_Stationary);
                    CheckboxFlags("ImGuiHoveredFlags_NoSharedDelay", p, ImGuiHoveredFlags_NoSharedDelay);
                    TreePop();
                }

            SeparatorText("その他");
            SliderFloat2("DisplayWindowPadding", (float*)&style.DisplayWindowPadding, 0.0f, 30.0f, "%.0f"); SameLine(); HelpMarker("通常のウィンドウに適用されます: 画面の端に近づけたときに、見える状態を保つ量です。");
            SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f"); SameLine(); HelpMarker("すべてのウィンドウ・メニュー・ポップアップ・ツールチップに適用されます: 内容の表示を避ける量です。画面の端が見えない場合 (例: スケーリングが未設定のテレビ) は調整してください。");

            EndTabItem();
        }

        if (BeginTabItem("配色"))
        {
            static int output_dest = 0;
            static bool output_only_modified = true;
            if (Button("エクスポート"))
            {
                if (output_dest == 0)
                    LogToClipboard();
                else
                    LogToTTY();
                LogText("ImVec4* colors = GetStyle().Colors;" IM_NEWLINE);
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const ImVec4& col = style.Colors[i];
                    const char* name = GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                        LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE,
                            name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                LogFinish();
            }
            SameLine(); SetNextItemWidth(GetFontSize() * 10); Combo("##output_type", &output_dest, "クリップボードへ\0TTY へ\0");
            SameLine(); Checkbox("変更した色のみ", &output_only_modified);

            static ImGuiTextFilter filter;
            filter.Draw("色を絞り込み", GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = 0;
            if (RadioButton("不透明", alpha_flags == ImGuiColorEditFlags_AlphaOpaque))       { alpha_flags = ImGuiColorEditFlags_AlphaOpaque; } SameLine();
            if (RadioButton("アルファ",  alpha_flags == ImGuiColorEditFlags_None))              { alpha_flags = ImGuiColorEditFlags_None; } SameLine();
            if (RadioButton("両方",   alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf))  { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } SameLine();
            HelpMarker(
                "色の一覧で:\n"
                "色の四角を左クリックするとカラーピッカーが開き、\n"
                "右クリックすると編集オプションのメニューが開きます。");

            SetNextWindowSizeConstraints(ImVec2(0.0f, GetTextLineHeightWithSpacing() * 10), ImVec2(FLT_MAX, FLT_MAX));
            BeginChild("##colors", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            PushItemWidth(GetFontSize() * -12);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                PushID(i);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
                if (Button("?"))
                    DebugFlashStyleColor((ImGuiCol)i);
                SetItemTooltip("指定した色を点滅させて、どこで使われているかを確認します。");
                SameLine();
#endif
                ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // so instead of "Save"/"Revert" you'd use icons!
                    // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
                    SameLine(0.0f, style.ItemInnerSpacing.x); if (Button("保存")) { ref->Colors[i] = style.Colors[i]; }
                    SameLine(0.0f, style.ItemInnerSpacing.x); if (Button("元に戻す")) { style.Colors[i] = ref->Colors[i]; }
                }
                SameLine(0.0f, style.ItemInnerSpacing.x);
                TextUnformatted(name);
                PopID();
            }
            PopItemWidth();
            EndChild();

            EndTabItem();
        }

        if (BeginTabItem("フォント"))
        {
            ImGuiIO& io = GetIO();
            ImFontAtlas* atlas = io.Fonts;
            ShowFontAtlas(atlas);

            // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
            // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows Ctrl+Click text to get out of bounds).
            /*
            SeparatorText("Legacy Scaling");
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            HelpMarker(
                "Those are old settings provided for convenience.\n"
                "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
                "rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.\n"
                "Using those settings here will give you poor quality results.");
            PushItemWidth(GetFontSize() * 8);
            DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything
            //static float window_scale = 1.0f;
            //if (DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
            //    SetWindowFontScale(window_scale);
            PopItemWidth();
            */

            EndTabItem();
        }

        if (BeginTabItem("描画"))
        {
            Checkbox("線のアンチエイリアス", &style.AntiAliasedLines);
            SameLine();
            HelpMarker("線のアンチエイリアスを無効にする場合は、スタイルの枠線も無効にしたくなるでしょう。");

            Checkbox("線のアンチエイリアスにテクスチャを使う", &style.AntiAliasedLinesUseTex);
            SameLine();
            HelpMarker("テクスチャデータを使う高速な線です。バックエンドがバイリニアフィルタリング (ポイント/ニアレストではなく) で描画する必要があります。");

            Checkbox("塗りつぶしのアンチエイリアス", &style.AntiAliasedFill);
            PushItemWidth(GetFontSize() * 8);
            DragFloat("曲線分割の許容誤差", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;

            // When editing the "Circle Segment Max Error" value, draw a preview of its effect on auto-tessellated circles.
            DragFloat("円分割の最大誤差", &style.CircleTessellationMaxError , 0.005f, 0.10f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            const bool show_samples = IsItemActive();
            if (show_samples)
                SetNextWindowPos(GetCursorScreenPos());
            if (show_samples && BeginTooltip())
            {
                TextUnformatted("(R = 半径、N = おおよそのセグメント数)");
                Spacing();
                ImDrawList* draw_list = GetWindowDrawList();
                const float min_widget_width = CalcTextSize("R: MMM\nN: MMM").x;
                for (int n = 0; n < 8; n++)
                {
                    const float RAD_MIN = 5.0f;
                    const float RAD_MAX = 70.0f;
                    const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

                    BeginGroup();

                    // N is not always exact here due to how PathArcTo() function work internally
                    Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));

                    const float canvas_width = IM_MAX(min_widget_width, rad * 2.0f);
                    const float offset_x     = floorf(canvas_width * 0.5f);
                    const float offset_y     = floorf(RAD_MAX);

                    const ImVec2 p1 = GetCursorScreenPos();
                    draw_list->AddCircle(ImVec2(p1.x + offset_x, p1.y + offset_y), rad, GetColorU32(ImGuiCol_Text));
                    Dummy(ImVec2(canvas_width, RAD_MAX * 2));

                    /*
                    const ImVec2 p2 = GetCursorScreenPos();
                    draw_list->AddCircleFilled(ImVec2(p2.x + offset_x, p2.y + offset_y), rad, GetColorU32(ImGuiCol_Text));
                    Dummy(ImVec2(canvas_width, RAD_MAX * 2));
                    */

                    EndGroup();
                    SameLine();
                }
                EndTooltip();
            }
            SameLine();
            HelpMarker("円のプリミティブを \"num_segments == 0\" で描画すると、分割数が自動的に計算されます。");

            DragFloat("全体のアルファ", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            DragFloat("無効時のアルファ", &style.DisabledAlpha, 0.005f, 0.0f, 1.0f, "%.2f"); SameLine(); HelpMarker("無効な項目に追加でかけるアルファ係数です (現在の Alpha の値に掛け合わされます)。");
            PopItemWidth();

            EndTabItem();
        }

        EndTabBar();
    }
    PopItemWidth();
}

//-----------------------------------------------------------------------------
// [SECTION] User Guide / ShowUserGuide()
//-----------------------------------------------------------------------------

// We omit the ImGui:: prefix in this function, as we don't expect user to be copy and pasting this code.
void ImGui::ShowUserGuide()
{
    ImGuiIO& io = GetIO();
    BulletText("タイトルバーをダブルクリックするとウィンドウを折りたためます。");
    BulletText(
        "右下隅や境界をクリックしてドラッグするとウィンドウのサイズを変えられます。\n"
        "(ダブルクリックで内容に合わせて自動調整)");
    BulletText("スライダーやドラッグボックスを Ctrl+クリックすると、値をテキストで入力できます。");
    BulletText("Tab / Shift+Tab でキーボード編集可能な項目を順に移動できます。");
    BulletText("Ctrl+Tab / Ctrl+Shift+Tab でウィンドウを切り替えられます。");
    if (io.FontAllowUserScaling)
        BulletText("Ctrl+マウスホイールでウィンドウの内容を拡大縮小できます。");
    BulletText("テキスト入力中:\n");
    Indent();
    BulletText("Ctrl+左/右 で単語単位の移動。");
    BulletText("Ctrl+A またはダブルクリックで全選択。");
    BulletText("Ctrl+X/C/V でクリップボードの切り取り/コピー/貼り付け。");
    BulletText("Ctrl+Z で元に戻す、Ctrl+Y / Ctrl+Shift+Z でやり直し。");
    BulletText("Escape で変更を取り消し。");
    Unindent();
    BulletText("キーボード操作が有効な場合:");
    Indent();
    BulletText("矢印キーや Home/End/PageUp/PageDown で移動。");
    BulletText("スペースでウィジェットを操作。");
    BulletText("Enter でウィジェットにテキストを入力。");
    BulletText("Escape でウィジェットの操作を終了、ポップアップを閉じる、\n子ウィンドウやメニュー層から抜ける、フォーカスを解除。");
    BulletText("Alt でウィンドウのメニュー層へ移動。");
    BulletText("Menu キーまたは Shift+F10 でコンテキストメニューを開く。");
    Unindent();
    BulletText("ゲームパッド操作が有効な場合:");
    Indent();
    BulletText("方向パッド: 移動 / 値の調整 / リサイズ (ウィンドウ操作モード時)。");
    BulletText("%s ボタン: 決定 / 開く / 切り替え。長押し: テキスト入力付きで決定。", io.ConfigNavSwapGamepadButtons ? "東" : "南");
    BulletText("%s ボタン: キャンセル / 閉じる / 抜ける。", io.ConfigNavSwapGamepadButtons ? "南" : "東");
    BulletText("西ボタン: メニューの切り替え。長押しでウィンドウ操作モード (フォーカス/移動/リサイズ)。");
    BulletText("北ボタン: コンテキストメニューを開く。");
    BulletText("L1/R1: 値の調整を遅く/速く、前/次へフォーカス (ウィンドウ操作モード時)。");
    Unindent();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowExampleAppMainMenuBar()
// - ShowExampleMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the ImGuiWindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("ファイル"))
        {
            IMGUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("編集"))
        {
            IMGUI_DEMO_MARKER("Menu/Edit");
            if (ImGui::MenuItem("元に戻す", "Ctrl+Z")) {}
            if (ImGui::MenuItem("やり直す", "Ctrl+Y", false, false)) {} // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("切り取り", "Ctrl+X")) {}
            if (ImGui::MenuItem("コピー", "Ctrl+C")) {}
            if (ImGui::MenuItem("貼り付け", "Ctrl+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

// Note that shortcuts are currently provided for display only
// (future version will add explicit flags to BeginMenu() to request processing shortcuts)
static void ShowExampleMenuFile()
{
    IMGUI_DEMO_MARKER("Examples/Menu");
    ImGui::MenuItem("(デモ用メニュー)", NULL, false, false);
    if (ImGui::MenuItem("新規作成")) {}
    if (ImGui::MenuItem("開く", "Ctrl+O")) {}
    if (ImGui::BeginMenu("最近使ったファイル"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("その他.."))
        {
            ImGui::MenuItem("こんにちは");
            ImGui::MenuItem("セーラーさん");
            if (ImGui::BeginMenu("再帰.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("保存", "Ctrl+S")) {}
    if (ImGui::MenuItem("名前を付けて保存..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("オプション"))
    {
        IMGUI_DEMO_MARKER("Examples/Menu/Options");
        static bool enabled = true;
        ImGui::MenuItem("有効", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 5.0f), ImGuiChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            ImGui::Text("スクロールするテキスト %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("値", &f, 0.0f, 1.0f);
        ImGui::InputFloat("入力", &f, 0.1f);
        ImGui::Combo("コンボ", &n, "はい\0いいえ\0たぶん\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("配色"))
    {
        IMGUI_DEMO_MARKER("Examples/Menu/Colors");
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("オプション")) // <-- Append!
    {
        IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
        static bool b = true;
        ImGui::Checkbox("何かのオプション", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("無効", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("チェック済み", NULL, true)) {}
    ImGui::Separator();
    if (ImGui::MenuItem("終了", "Alt+F4")) {}
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
struct ExampleAppConsole
{
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    ExampleAppConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");
        AutoScroll = true;
        ScrollToBottom = false;
        AddLog("Dear ImGui へようこそ!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            ImGui::MemFree(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = ImGui::MemAlloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            ImGui::MemFree(Items[i]);
        Items.clear();
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_COUNTOF(buf), fmt, args);
        buf[IM_COUNTOF(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void    Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }
        IMGUI_DEMO_MARKER("Examples/Console");

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("コンソールを閉じる"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped(
            "この例では、簡単な色分け・補完 (TAB キー)・履歴 (上下キー) を備えたコンソールを実装しています。"
            "もっと本格的に作る場合は、タイムスタンプや発信元などの追加データも一緒に保存するとよいでしょう。");
        ImGui::TextWrapped("ヘルプを見るには 'HELP' と入力してください。");

        // TODO: display items starting from the bottom

        if (ImGui::SmallButton("デバッグテキストを追加"))  { AddLog("%d 何かのテキスト", Items.Size); AddLog("さらにテキスト"); AddLog("ここにとても重要なメッセージを表示!"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("デバッグエラーを追加")) { AddLog("[error] 何か問題が発生しました"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("クリア"))           { ClearLog(); }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("コピー");
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("自動スクロール", &AutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_Tooltip);
        if (ImGui::Button("オプション"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        Filter.Draw("絞り込み (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        ImGuiStyle& style = ImGui::GetStyle();
        const float footer_height_to_reserve = style.SeparatorSize + style.ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("クリア")) ClearLog();
                ImGui::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      ImGuiListClipper clipper;
            //      clipper.Begin(Items.Size);
            //      while (clipper.Step())
            //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            // - That your items are evenly spaced (same height)
            // - That you have cheap random access to your elements (you can access them given their index,
            //   without processing all the ones before)
            // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
            // We would need random-access on the post-filtered list.
            // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
            // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
            // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
            // to improve this example code!
            // If your items are of variable height:
            // - Split them into same height items would be simpler and facilitate random-seeking into your list.
            // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                ImGui::LogToClipboard();
            for (const char* item : Items)
            {
                if (!Filter.PassFilter(item))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                ImVec4 color;
                bool has_color = false;
                if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                if (has_color)
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(item);
                if (has_color)
                    ImGui::PopStyleColor();
            }
            if (copy_to_clipboard)
                ImGui::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            ScrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        if (ImGui::InputText("入力", InputBuf, IM_COUNTOF(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void    ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                ImGui::MemFree(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
        {
            ClearLog();
        }
        else if (Stricmp(command_line, "HELP") == 0)
        {
            AddLog("コマンド一覧:");
            for (int i = 0; i < Commands.Size; i++)
                AddLog("- %s", Commands[i]);
        }
        else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
        }
        else
        {
            AddLog("不明なコマンド: '%s'\n", command_line);
        }

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("\"%.*s\" に一致するものがありません!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputting "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("候補:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
        case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
};

static void ShowExampleAppConsole(bool* p_open)
{
    static ExampleAppConsole console;
    console.Draw("サンプル: コンソール", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Image Viewer / ShowExampleAppImageViewer()
//-----------------------------------------------------------------------------

static void ShowExampleAppImageViewer(bool* p_open)
{
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    ImTextureRef tex_ref = atlas->TexRef; // We don't have access to other textures in this demo!
    int tex_w = atlas->TexData->Width;
    int tex_h = atlas->TexData->Height;
    if (ImGui::Begin("サンプル: 画像ビューアー", p_open))
    {
        static ExampleImageViewerData image_viewer;
        ExampleImageViewer_DrawOptions(&image_viewer);
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImVec2 canvas_min_size = ImGui::IsWindowAppearing() ? ImVec2(3.0f * tex_w, 4.0f * tex_h) : ImVec2(1.0f, 1.0f);
        canvas_size = ImVec2(IM_MAX(canvas_size.x, canvas_min_size.x), IM_MAX(canvas_size.y, canvas_min_size.y));
        ExampleImageViewer_DrawCanvas(&image_viewer, canvas_size, tex_ref, tex_w, tex_h);
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("自動スクロール", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("オプション"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("クリア");
        ImGui::SameLine();
        bool copy = ImGui::Button("コピー");
        ImGui::SameLine();
        Filter.Draw("絞り込み", -100.0f);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
                ImGuiListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::End();
    }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
    static ExampleAppLog log;

    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("サンプル: ログ", p_open);
    IMGUI_DEMO_MARKER("Examples/Log");
    if (ImGui::SmallButton("[デバッグ] 5 件追加"))
    {
        static int counter = 0;
        const char* categories[3] = { "info", "warn", "error" };
        const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
        for (int n = 0; n < 5; n++)
        {
            const char* category = categories[counter % IM_COUNTOF(categories)];
            const char* word = words[counter % IM_COUNTOF(words)];
            log.AddLog("[%05d] [%s] こんにちは、現在時刻は %.1f、単語はこちら: '%s'\n",
                ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
            counter++;
        }
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("サンプル: ログ", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("サンプル: シンプルなレイアウト", p_open, ImGuiWindowFlags_MenuBar))
    {
        IMGUI_DEMO_MARKER("Examples/Simple layout");
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("ファイル"))
            {
                if (ImGui::MenuItem("閉じる", "Ctrl+W")) { *p_open = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Left
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
            for (int i = 0; i < 100; i++)
            {
                char label[128];
                sprintf(label, "マイオブジェクト %d", i);
                if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SelectOnNav))
                    selected = i;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("マイオブジェクト: %d", selected);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("説明"))
                {
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("詳細"))
                {
                    ImGui::Text("ID: 0123456789");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if (ImGui::Button("元に戻す")) {}
            ImGui::SameLine();
            if (ImGui::Button("保存")) {}
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
//-----------------------------------------------------------------------------
// Some of the interactions are a bit lack-luster:
// - We would want pressing validating or leaving the filter to somehow restore focus.
// - We may want more advanced filtering (child nodes) and clipper support: both will need extra work.
// - We would want to customize some keyboard interactions to easily keyboard navigate between the tree and the properties.
//-----------------------------------------------------------------------------

struct ExampleAppPropertyEditor
{
    ImGuiTextFilter     Filter;
    ExampleTreeNode*    SelectedNode = NULL;
    bool                UseClipper = false;

    void Draw(ExampleTreeNode* root_node)
    {
        IMGUI_DEMO_MARKER("Examples/Property editor");

        // Left side: draw tree
        // - Currently using a table to benefit from RowBg feature
        // - Our tree node are all of equal height, facilitating the use of a clipper.
        if (ImGui::BeginChild("##tree", ImVec2(300, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
        {
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            ImGui::Checkbox("クリッパーを使う", &UseClipper);
            ImGui::SameLine();
            ImGui::Text("(ルートノード %d 個)", root_node->Childs.Size);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
            if (ImGui::InputTextWithHint("##Filter", "incl,-excl", Filter.InputBuf, IM_COUNTOF(Filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
                Filter.Build();
            ImGui::PopItemFlag();

            if (ImGui::BeginTable("##list", 1, ImGuiTableFlags_RowBg))
            {
                if (UseClipper)
                    DrawClippedTree(root_node);
                else
                    DrawTree(root_node);
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        // Right side: draw properties
        ImGui::SameLine();

        ImGui::BeginGroup(); // Lock X position
        if (ExampleTreeNode* node = SelectedNode)
        {
            ImGui::Text("%s", node->Name);
            ImGui::TextDisabled("UID: 0x%08X", node->UID);
            ImGui::Separator();
            if (ImGui::BeginTable("##properties", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
            {
                // Push object ID after we entered the table, so table is shared for all objects
                ImGui::PushID((int)node->UID);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger
                if (node->HasData)
                {
                    // In a typical application, the structure description would be derived from a data-driven system.
                    // - We try to mimic this with our ExampleMemberInfo structure and the ExampleTreeNodeMemberInfos[] array.
                    // - Limits and some details are hard-coded to simplify the demo.
                    for (const ExampleMemberInfo& field_desc : ExampleTreeNodeMemberInfos)
                    {
                        ImGui::TableNextRow();
                        ImGui::PushID(field_desc.Name);
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextUnformatted(field_desc.Name);
                        ImGui::TableNextColumn();
                        void* field_ptr = (void*)(((unsigned char*)node) + field_desc.Offset);
                        switch (field_desc.DataType)
                        {
                        case ImGuiDataType_Bool:
                        {
                            IM_ASSERT(field_desc.DataCount == 1);
                            ImGui::Checkbox("##Editor", (bool*)field_ptr);
                            break;
                        }
                        case ImGuiDataType_S32:
                        {
                            int v_min = INT_MIN, v_max = INT_MAX;
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, 1.0f, &v_min, &v_max);
                            break;
                        }
                        case ImGuiDataType_Float:
                        {
                            float v_min = 0.0f, v_max = 1.0f;
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::SliderScalarN("##Editor", field_desc.DataType, field_ptr, field_desc.DataCount, &v_min, &v_max);
                            break;
                        }
                        case ImGuiDataType_String:
                        {
                            ImGui::InputText("##Editor", reinterpret_cast<char*>(field_ptr), 28);
                            break;
                        }
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::PopID();
                ImGui::EndTable();
            }
        }
        ImGui::EndGroup();
    }

    // Custom search filter
    // - Here we apply on root node only.
    // - This does a case insensitive stristr which is pretty heavy. In a real large-scale app you would likely store a filtered list which in turns would be trivial to linearize.
    inline bool IsNodePassingFilter(ExampleTreeNode* node)
    {
        return node->Parent->Parent != NULL || Filter.PassFilter(node->Name);
    }

    // Basic version, recursive. This is how you would generally draw a tree.
    // - Simple but going to be noticeably costly if you have a large amount of nodes as DrawTreeNode() is called for all of them.
    // - On my desktop PC (2020), for 10K nodes in an optimized build this takes ~1.2 ms
    // - Unlike arrays or grids which are very easy to clip, trees are currently more difficult to clip.
    void DrawTree(ExampleTreeNode* node)
    {
        for (ExampleTreeNode* child : node->Childs)
            if (IsNodePassingFilter(child) && DrawTreeNode(child))
            {
                DrawTree(child);
                ImGui::TreePop();
            }
    }

    // More advanced version. Use a alternative clipping technique: fast-forwarding through non-visible chunks.
    // - On my desktop PC (2020), for 10K nodes in an optimized build this takes ~0.1 ms
    //   (in ExampleTree_CreateDemoTree(), change 'int ROOT_ITEMS_COUNT = 10000' to try with this amount of root nodes).
    // - 1. Use clipper with indeterminate count (items_count = INT_MAX): we need to call SeekCursorForItem() at the end once we know the count.
    // - 2. Use SetNextItemStorageID() to specify ID used for open/close storage, making it easy to call TreeNodeGetOpen() on any arbitrary node.
    // - 3. Linearize tree during traversal: our tree data structure makes it easy to access sibling and parents.
    // - Unlike clipping for a regular array or grid which may be done using random access limited to visible areas,
    //   this technique requires traversing most accessible nodes. This could be made more optimal with extra work,
    //   but this is a decent simplicity<>speed trade-off.
    // See https://github.com/ocornut/imgui/issues/3823 for discussions about this.
    void DrawClippedTree(ExampleTreeNode* root_node)
    {
        ExampleTreeNode* node = root_node->Childs[0]; // First node
        ImGuiListClipper clipper;
        clipper.Begin(INT_MAX);
        while (clipper.Step())
            while (clipper.UserIndex < clipper.DisplayEnd && node != NULL)
                node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);

        // Keep going to count nodes and submit final count so we have a reliable scrollbar.
        // - One could consider caching this value and only refreshing it occasionally e.g. window is focused and an action occurs.
        // - Incorrect but cheap approximation would be to use 'clipper_current_idx = IM_MAX(clipper_current_idx, root_node->Childs.Size)' instead.
        // - If either of those is implemented, the general cost will approach zero when scrolling is at the top of the tree.
        while (node != NULL)
            node = DrawClippedTreeNodeAndAdvanceToNext(&clipper, node);
        //clipper.UserIndex = IM_MAX(clipper.UserIndex, root_node->Childs.Size); // <-- Cheap approximation instead of while() loop above.
        clipper.SeekCursorForItem(clipper.UserIndex);
    }

    ExampleTreeNode* DrawClippedTreeNodeAndAdvanceToNext(ImGuiListClipper* clipper, ExampleTreeNode* node)
    {
        if (IsNodePassingFilter(node))
        {
            // Draw node if within visible range
            bool is_open = false;
            if (clipper->UserIndex >= clipper->DisplayStart && clipper->UserIndex < clipper->DisplayEnd)
            {
                is_open = DrawTreeNode(node);
            }
            else
            {
                is_open = (node->Childs.Size > 0 && ImGui::TreeNodeGetOpen((ImGuiID)node->UID));
                if (is_open)
                    ImGui::TreePush(node->Name);
            }
            clipper->UserIndex++;

            // Next node: recurse into childs
            if (is_open)
                return node->Childs[0];
        }

        // Next node: next sibling, otherwise move back to parent
        while (node != NULL)
        {
            if (node->IndexInParent + 1 < node->Parent->Childs.Size)
                return node->Parent->Childs[node->IndexInParent + 1];
            node = node->Parent;
            if (node->Parent == NULL)
                break;
            ImGui::TreePop();
        }
        return NULL;
    }

    // To support node with same name we incorporate node->UID into the item ID.
    // (this would more naturally be done using PushID(node->UID) + TreeNodeEx(node->Name, tree_flags),
    //   but it would require in DrawClippedTreeNodeAndAdvanceToNext() to add PushID() before TreePush(), and PopID() after TreePop(),
    //   so instead we use TreeNodeEx(node->UID, tree_flags, "%s", node->Name) here)
    bool DrawTreeNode(ExampleTreeNode* node)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
        tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to want to add selection afterwards
        tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;  // Left arrow support
        tree_flags |= ImGuiTreeNodeFlags_SpanFullWidth;         // Span full width for easier mouse reach
        tree_flags |= ImGuiTreeNodeFlags_DrawLinesToNodes;      // Always draw hierarchy outlines
        if (node == SelectedNode)
            tree_flags |= ImGuiTreeNodeFlags_Selected;          // Draw selection highlight
        if (node->Childs.Size == 0)
            tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen; // Use _NoTreePushOnOpen + set is_open=false to avoid unnecessarily push/pop on leaves.
        if (node->DataMyBool == false)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::SetNextItemStorageID((ImGuiID)node->UID);        // Use node->UID as storage id
        bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)node->UID, tree_flags, "%s", node->Name);
        if (node->Childs.Size == 0)
            is_open = false;
        if (node->DataMyBool == false)
            ImGui::PopStyleColor();
        if (ImGui::IsItemFocused())
            SelectedNode = node;
        return is_open;
    }
};

// Demonstrate creating a simple property editor.
static void ShowExampleAppPropertyEditor(bool* p_open, ImGuiDemoWindowData* demo_data)
{
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("サンプル: プロパティエディター", p_open))
    {
        ImGui::End();
        return;
    }

    IMGUI_DEMO_MARKER("Examples/Property Editor");
    static ExampleAppPropertyEditor property_editor;
    if (demo_data->DemoTree == NULL)
        demo_data->DemoTree = ExampleTree_CreateDemoTree();
    property_editor.Draw(demo_data->DemoTree);

    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("サンプル: 長いテキストの表示", p_open))
    {
        ImGui::End();
        return;
    }
    IMGUI_DEMO_MARKER("Examples/Long text display");

    static int test_type = 0;
    static ImGuiTextBuffer log;
    static int lines = 0;
    ImGui::Text("とても長い量のテキストを表示します。");
    ImGui::Combo("テストの種類", &test_type,
        "TextUnformatted() を1回だけ呼ぶ\0"
        "Text() を複数回呼ぶ (クリッピングあり)\0"
        "Text() を複数回呼ぶ (クリッピングなし・低速)\0");
    ImGui::Text("バッファの内容: %d 行、%d バイト", lines, log.size());
    if (ImGui::Button("クリア")) { log.clear(); lines = 0; }
    ImGui::SameLine();
    if (ImGui::Button("1000 行追加"))
    {
        for (int i = 0; i < 1000; i++)
            log.appendf("%i 素早い茶色のキツネがのんびり屋の犬を飛び越えます\n", lines + i);
        lines += 1000;
    }
    ImGui::BeginChild("Log");
    switch (test_type)
    {
    case 0:
        // Single call to TextUnformatted() with a big buffer
        ImGui::TextUnformatted(log.begin(), log.end());
        break;
    case 1:
        {
            // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGuiListClipper helper.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGuiListClipper clipper;
            clipper.Begin(lines);
            while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                    ImGui::Text("%i 素早い茶色のキツネがのんびり屋の犬を飛び越えます", i);
            ImGui::PopStyleVar();
            break;
        }
    case 2:
        // Multiple calls to Text(), not clipped (slow)
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        for (int i = 0; i < lines; i++)
            ImGui::Text("%i 素早い茶色のキツネがのんびり屋の犬を飛び越えます", i);
        ImGui::PopStyleVar();
        break;
    }
    ImGui::EndChild();
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool* p_open)
{
    if (!ImGui::Begin("サンプル: 自動リサイズウィンドウ", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }
    IMGUI_DEMO_MARKER("Examples/Auto-resizing window");

    static int lines = 10;
    ImGui::TextUnformatted(
        "ウィンドウは毎フレーム、内容のサイズに合わせてリサイズされます。\n"
        "内容を出力するためにウィンドウサイズを問い合わせると\n"
        "フィードバックループになってしまうので、避けたほうがよいでしょう。");
    ImGui::SliderInt("行数", &lines, 1, 20);
    for (int i = 0; i < lines; i++)
        ImGui::Text("%*sこれは %d 行目です", i * 4, "", i); // Pad with space to extend size horizontally
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window with custom resize constraints.
// Note that size constraints currently don't work on a docked window (when in 'docking' branch)
static void ShowExampleAppConstrainedResize(bool* p_open)
{
    struct CustomConstraints
    {
        // Helper functions to demonstrate programmatic constraints
        // FIXME: This doesn't take account of decoration size (e.g. title bar), library should make this easier.
        // FIXME: None of the three demos works consistently when resizing from borders.
        static void AspectRatio(ImGuiSizeCallbackData* data)
        {
            float aspect_ratio = *(float*)data->UserData;
            data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
        }
        static void Square(ImGuiSizeCallbackData* data)
        {
            data->DesiredSize.x = data->DesiredSize.y = IM_MAX(data->DesiredSize.x, data->DesiredSize.y);
        }
        static void Step(ImGuiSizeCallbackData* data)
        {
            float step = *(float*)data->UserData;
            data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step);
        }
    };

    const char* test_desc[] =
    {
        "100x100 から 500x500 の間",
        "最低でも 100x100",
        "縦だけリサイズ + 現在の幅を固定",
        "横だけリサイズ + 現在の高さを固定",
        "幅は 400 から 500 の間",
        "高さは最低 400",
        "カスタム: アスペクト比 16:9",
        "カスタム: 常に正方形",
        "カスタム: 100 刻み",
    };

    // Options
    static bool auto_resize = false;
    static bool window_padding = true;
    static int type = 6; // Aspect Ratio
    static int display_lines = 10;

    // Submit constraint
    float aspect_ratio = 16.0f / 9.0f;
    float fixed_step = 100.0f;
    if (type == 0) ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(500, 500));         // Between 100x100 and 500x500
    if (type == 1) ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    if (type == 2) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 0),    ImVec2(-1, FLT_MAX));      // Resize vertical + lock current width
    if (type == 3) ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1),    ImVec2(FLT_MAX, -1));      // Resize horizontal + lock current height
    if (type == 4) ImGui::SetNextWindowSizeConstraints(ImVec2(400, -1),  ImVec2(500, -1));          // Width Between and 400 and 500
    if (type == 5) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 400),  ImVec2(-1, FLT_MAX));      // Height at least 400
    if (type == 6) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::AspectRatio, (void*)&aspect_ratio);   // Aspect ratio
    if (type == 7) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                              // Always Square
    if (type == 8) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0),     ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)&fixed_step);            // Fixed Step

    // Submit window
    if (!window_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const ImGuiWindowFlags window_flags = auto_resize ? ImGuiWindowFlags_AlwaysAutoResize : 0;
    const bool window_open = ImGui::Begin("サンプル: リサイズ制限付きウィンドウ", p_open, window_flags);
    if (!window_padding)
        ImGui::PopStyleVar();
    IMGUI_DEMO_MARKER("Examples/Constrained Resizing window");
    if (window_open)
    {
        if (ImGui::GetIO().KeyShift)
        {
            // Display a dummy viewport (in your real app you would likely use ImageButton() to display a texture)
            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::ColorButton("viewport", ImVec4(0.5f, 0.2f, 0.5f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, avail_size);
            ImGui::SetCursorScreenPos(ImVec2(pos.x + 10, pos.y + 10));
            ImGui::Text("%.2f x %.2f", avail_size.x, avail_size.y);
        }
        else
        {
            ImGui::Text("(Shift を押している間はダミーのビューポートを表示)");
            if (ImGui::IsWindowDocked())
                ImGui::Text("警告: ウィンドウがドッキングされているとサイズ制約は機能しません!");
            if (ImGui::Button("200x200 にする")) { ImGui::SetWindowSize(ImVec2(200, 200)); } ImGui::SameLine();
            if (ImGui::Button("500x500 にする")) { ImGui::SetWindowSize(ImVec2(500, 500)); } ImGui::SameLine();
            if (ImGui::Button("800x200 にする")) { ImGui::SetWindowSize(ImVec2(800, 200)); }
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 20);
            ImGui::Combo("制約", &type, test_desc, IM_COUNTOF(test_desc));
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 20);
            ImGui::DragInt("行数", &display_lines, 0.2f, 1, 100);
            ImGui::Checkbox("自動リサイズ", &auto_resize);
            ImGui::Checkbox("ウィンドウの余白", &window_padding);
            for (int i = 0; i < display_lines; i++)
                ImGui::Text("%*sこんにちは、セーラーさん! 例として十分に長い行にしています。", i * 4, "");
        }
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool* p_open)
{
    static int location = 0;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (location >= 0)
    {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    else if (location == -2)
    {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("サンプル: シンプルなオーバーレイ", p_open, window_flags))
    {
        IMGUI_DEMO_MARKER("Examples/Simple overlay"); // Scroll up to the beginning of this function to see overlay flags
        ImGui::Text("シンプルなオーバーレイ\n" "(右クリックで位置を変更)");
        ImGui::Separator();
        if (ImGui::IsMousePosValid())
            ImGui::Text("マウス位置: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("マウス位置: <無効>");
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("カスタム",       NULL, location == -1)) location = -1;
            if (ImGui::MenuItem("中央",       NULL, location == -2)) location = -2;
            if (ImGui::MenuItem("左上",     NULL, location == 0)) location = 0;
            if (ImGui::MenuItem("右上",    NULL, location == 1)) location = 1;
            if (ImGui::MenuItem("左下",  NULL, location == 2)) location = 2;
            if (ImGui::MenuItem("右下", NULL, location == 3)) location = 3;
            if (p_open && ImGui::MenuItem("閉じる")) *p_open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
//-----------------------------------------------------------------------------

// Demonstrate creating a window covering the entire screen/viewport
static void ShowExampleAppFullscreen(bool* p_open)
{
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one or the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    if (ImGui::Begin("サンプル: 全画面ウィンドウ", p_open, flags))
    {
        IMGUI_DEMO_MARKER("Examples/Fullscreen window");
        ImGui::Checkbox("メイン領域ではなく作業領域を使う", &use_work_area);
        ImGui::SameLine();
        HelpMarker("メイン領域 = ビューポート全体、\n作業領域 = ビューポートからメインメニューバーやタスクバーなどが使う部分を除いた領域。\n\n違いを見るには「サンプル」メニューからメインメニューバーを有効にしてください。");

        ImGui::CheckboxFlags("ImGuiWindowFlags_NoBackground", &flags, ImGuiWindowFlags_NoBackground);
        ImGui::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &flags, ImGuiWindowFlags_NoDecoration);
        ImGui::Indent();
        ImGui::CheckboxFlags("ImGuiWindowFlags_NoTitleBar", &flags, ImGuiWindowFlags_NoTitleBar);
        ImGui::CheckboxFlags("ImGuiWindowFlags_NoCollapse", &flags, ImGuiWindowFlags_NoCollapse);
        ImGui::CheckboxFlags("ImGuiWindowFlags_NoScrollbar", &flags, ImGuiWindowFlags_NoScrollbar);
        ImGui::Unindent();

        if (p_open && ImGui::Button("このウィンドウを閉じる"))
            *p_open = false;
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate the use of "##" and "###" in identifiers to manipulate ID generation.
// This applies to all regular items as well.
// Read FAQ section "How can I have multiple widgets with the same label?" for details.
static void ShowExampleAppWindowTitles(bool*)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 base_pos = viewport->Pos;

    // By default, Windows are uniquely identified by their title.
    // You can use the "##" and "###" markers to manipulate the display/ID.

    // Using "##" to display same title but have unique identifier.
    ImGui::SetNextWindowPos(ImVec2(base_pos.x + 100, base_pos.y + 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("別のウィンドウと同じタイトル##1");
    IMGUI_DEMO_MARKER("Examples/Manipulating window titles##1");
    ImGui::Text("これはウィンドウ 1 です。\nタイトルはウィンドウ 2 と同じですが、識別子は別々です。");
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(base_pos.x + 100, base_pos.y + 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("別のウィンドウと同じタイトル##2");
    IMGUI_DEMO_MARKER("Examples/Manipulating window titles##2");
    ImGui::Text("これはウィンドウ 2 です。\nタイトルはウィンドウ 1 と同じですが、識別子は別々です。");
    ImGui::End();

    // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
    char buf[128];
    sprintf(buf, "動くタイトル %c %d###AnimatedTitle", "|/-\\"[(int)(ImGui::GetTime() / 0.25f) & 3], ImGui::GetFrameCount());
    ImGui::SetNextWindowPos(ImVec2(base_pos.x + 100, base_pos.y + 300), ImGuiCond_FirstUseEver);
    ImGui::Begin(buf);
    IMGUI_DEMO_MARKER("Examples/Manipulating window titles##3");
    ImGui::Text("このウィンドウはタイトルが変化します。");
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Add a |_| looking shape
static void PathConcaveShape(ImDrawList* draw_list, float x, float y, float sz)
{
    const ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
    for (const ImVec2& p : pos_norms)
        draw_list->PathLineTo(ImVec2(x + 0.5f + (int)(sz * p.x), y + 0.5f + (int)(sz * p.y)));
}

// Demonstrate using the low-level ImDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool* p_open)
{
    if (!ImGui::Begin("サンプル: カスタム描画", p_open))
    {
        ImGui::End();
        return;
    }
    IMGUI_DEMO_MARKER("Examples/Custom rendering");

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
    // overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
    // types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
    // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

    if (ImGui::BeginTabBar("##TabBar"))
    {
        if (ImGui::BeginTabItem("プリミティブ"))
        {
            IMGUI_DEMO_MARKER("Examples/Custom rendering/Primitives");
            ImGui::PushItemWidth(-ImGui::GetFontSize() * 15);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            // Draw gradients
            // (note that those are currently exacerbating our sRGB/Linear issues)
            // Calling ImGui::GetColorU32() multiplies the given colors by the current Style Alpha, but you may pass the IM_COL32() directly as well..
            ImGui::Text("グラデーション");
            ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
            {
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
                ImU32 col_a = ImGui::GetColorU32(IM_COL32(0, 0, 0, 255));
                ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 255, 255, 255));
                draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
                ImGui::InvisibleButton("##gradient1", gradient_size);
            }
            {
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
                ImU32 col_a = ImGui::GetColorU32(IM_COL32(0, 255, 0, 255));
                ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 0, 0, 255));
                draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
                ImGui::InvisibleButton("##gradient2", gradient_size);
            }

            // Draw a bunch of primitives
            ImGui::Text("すべてのプリミティブ");
            static float sz = 36.0f;
            static float thickness = 3.0f;
            static int ngon_sides = 6;
            static bool circle_segments_override = false;
            static int circle_segments_override_v = 12;
            static bool curve_segments_override = false;
            static int curve_segments_override_v = 8;
            static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
            ImGui::DragFloat("サイズ", &sz, 0.2f, 2.0f, 100.0f, "%.0f");
            ImGui::DragFloat("太さ", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
            ImGui::SliderInt("多角形の辺の数", &ngon_sides, 3, 12);
            ImGui::Checkbox("##circlesegmentoverride", &circle_segments_override);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            circle_segments_override |= ImGui::SliderInt("円のセグメント数を上書き", &circle_segments_override_v, 3, 40);
            ImGui::Checkbox("##curvessegmentoverride", &curve_segments_override);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            curve_segments_override |= ImGui::SliderInt("曲線のセグメント数を上書き", &curve_segments_override_v, 3, 40);
            ImGui::ColorEdit4("色", &colf.x);

            const ImVec2 p = ImGui::GetCursorScreenPos();
            const ImU32 col = ImColor(colf);
            const float spacing = 10.0f;
            const ImDrawFlags corners_tl_br = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;
            const float rounding = sz / 5.0f;
            const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
            const int curve_segments = curve_segments_override ? curve_segments_override_v : 0;
            const ImVec2 cp3[3] = { ImVec2(0.0f, sz * 0.6f), ImVec2(sz * 0.5f, -sz * 0.4f), ImVec2(sz, sz) }; // Control points for curves
            const ImVec2 cp4[4] = { ImVec2(0.0f, 0.0f), ImVec2(sz * 1.3f, sz * 0.3f), ImVec2(sz - sz * 1.3f, sz - sz * 0.3f), ImVec2(sz, sz) };

            float x = p.x + 4.0f;
            float y = p.y + 4.0f;
            for (int n = 0; n < 2; n++)
            {
                // First line uses a thickness of 1.0f, second line uses the configurable thickness
                float th = (n == 0) ? 1.0f : thickness;
                draw_list->AddNgon(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, ngon_sides, th);                 x += sz + spacing;  // N-gon
                draw_list->AddCircle(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col, circle_segments, th);          x += sz + spacing;  // Circle
                draw_list->AddEllipse(ImVec2(x + sz*0.5f, y + sz*0.5f), ImVec2(sz*0.5f, sz*0.3f), col, -0.3f, circle_segments, th); x += sz + spacing;  // Ellipse
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 0.0f, th);                            x += sz + spacing;  // Square
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, rounding, th);                        x += sz + spacing;  // Square with all rounded corners
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, rounding, th, corners_tl_br);         x += sz + spacing;  // Square with two rounded corners
                draw_list->AddTriangle(ImVec2(x+sz*0.5f,y), ImVec2(x+sz, y+sz-0.5f), ImVec2(x, y+sz-0.5f), col, th);x += sz + spacing;  // Triangle
                //draw_list->AddTriangle(ImVec2(x+sz*0.2f,y), ImVec2(x, y+sz-0.5f), ImVec2(x+sz*0.4f, y+sz-0.5f), col, th);x+= sz*0.4f + spacing; // Thin triangle
                PathConcaveShape(draw_list, x, y, sz); draw_list->PathStroke(col, th, ImDrawFlags_Closed);          x += sz + spacing;  // Concave Shape
                //draw_list->AddPolyline(concave_shape, IM_COUNTOF(concave_shape), col, ImDrawFlags_Closed, th);
                draw_list->AddLineH(x, x + sz, y, col, th);                                                         x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLineV(x, y, y + sz, col, th);                                                         x += spacing;       // Vertical line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y + sz), col, th);                                  x += sz + spacing;  // Diagonal line

                // Path
                draw_list->PathArcTo(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, 3.141592f, 3.141592f * -0.5f);
                draw_list->PathStroke(col, th);
                x += sz + spacing;

                // Quadratic Bezier Curve (3 control points)
                draw_list->AddBezierQuadratic(ImVec2(x + cp3[0].x, y + cp3[0].y), ImVec2(x + cp3[1].x, y + cp3[1].y), ImVec2(x + cp3[2].x, y + cp3[2].y), col, th, curve_segments);
                x += sz + spacing;

                // Cubic Bezier Curve (4 control points)
                draw_list->AddBezierCubic(ImVec2(x + cp4[0].x, y + cp4[0].y), ImVec2(x + cp4[1].x, y + cp4[1].y), ImVec2(x + cp4[2].x, y + cp4[2].y), ImVec2(x + cp4[3].x, y + cp4[3].y), col, th, curve_segments);

                x = p.x + 4;
                y += sz + spacing;
            }

            // Filled shapes
            draw_list->AddNgonFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides);             x += sz + spacing;  // N-gon
            draw_list->AddCircleFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, circle_segments);      x += sz + spacing;  // Circle
            draw_list->AddEllipseFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), ImVec2(sz * 0.5f, sz * 0.3f), col, -0.3f, circle_segments); x += sz + spacing;// Ellipse
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col);                                    x += sz + spacing;  // Square
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f);                             x += sz + spacing;  // Square with all rounded corners
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_tl_br);              x += sz + spacing;  // Square with two rounded corners
            draw_list->AddTriangleFilled(ImVec2(x+sz*0.5f,y), ImVec2(x+sz, y+sz-0.5f), ImVec2(x, y+sz-0.5f), col);  x += sz + spacing;  // Triangle
            //draw_list->AddTriangleFilled(ImVec2(x+sz*0.2f,y), ImVec2(x, y+sz-0.5f), ImVec2(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin triangle
            PathConcaveShape(draw_list, x, y, sz); draw_list->PathFillConcave(col);                                 x += sz + spacing;  // Concave shape
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + thickness), col);                             x += sz + spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + thickness, y + sz), col);                             x += spacing * 2.0f;// Vertical line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), col);                                      x += sz;            // Pixel (faster than AddLine)

            // Path
            draw_list->PathArcTo(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, 3.141592f * -0.5f, 3.141592f);
            draw_list->PathFillConvex(col);
            x += sz + spacing;

            // Quadratic Bezier Curve (3 control points)
            draw_list->PathLineTo(ImVec2(x + cp3[0].x, y + cp3[0].y));
            draw_list->PathBezierQuadraticCurveTo(ImVec2(x + cp3[1].x, y + cp3[1].y), ImVec2(x + cp3[2].x, y + cp3[2].y), curve_segments);
            draw_list->PathFillConvex(col);
            x += sz + spacing;

            draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + sz, y + sz), IM_COL32(0, 0, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 0, 255));
            x += sz + spacing;

            ImGui::Dummy(ImVec2((sz + spacing) * 13.2f, (sz + spacing) * 3.0f));
            ImGui::PopItemWidth();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("キャンバス"))
        {
            IMGUI_DEMO_MARKER("Examples/Custom rendering/Canvas");
            static ImVector<ImVec2> points;
            static ImVec2 scrolling(0.0f, 0.0f);
            static bool opt_enable_grid = true;
            static bool opt_enable_context_menu = true;
            static bool adding_line = false;

            ImGui::Checkbox("グリッドを表示", &opt_enable_grid);
            ImGui::Checkbox("コンテキストメニューを有効にする", &opt_enable_context_menu);
            ImGui::Text("マウス左ボタン: ドラッグで線を追加、\nマウス右ボタン: ドラッグでスクロール、クリックでコンテキストメニュー。");

            // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
            // Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
            // To use a child window instead we could use, e.g:
            //      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
            //      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
            //      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove);
            //      ImGui::PopStyleColor();
            //      ImGui::PopStyleVar();
            //      [...]
            //      ImGui::EndChild();

            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

            // This will catch our interactions
            ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            const bool is_hovered = ImGui::IsItemHovered(); // Hovered
            const bool is_active = ImGui::IsItemActive();   // Held
            const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
            const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

            // Add first and second point
            if (is_hovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                points.push_back(mouse_pos_in_canvas);
                points.push_back(mouse_pos_in_canvas);
                adding_line = true;
            }
            if (adding_line)
            {
                points.back() = mouse_pos_in_canvas;
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    adding_line = false;
            }

            // Pan (we use a zero mouse threshold when there's no context menu)
            // You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
            const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
            {
                scrolling.x += io.MouseDelta.x;
                scrolling.y += io.MouseDelta.y;
            }

            // Context menu (under default mouse threshold)
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            if (opt_enable_context_menu && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
                ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
            if (ImGui::BeginPopup("context"))
            {
                if (adding_line)
                    points.resize(points.size() - 2);
                adding_line = false;
                if (ImGui::MenuItem("1つ削除", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
                if (ImGui::MenuItem("すべて削除", NULL, false, points.Size > 0)) { points.clear(); }
                ImGui::EndPopup();
            }

            // Draw grid + all lines in the canvas
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);
            if (opt_enable_grid)
            {
                const float GRID_STEP = 64.0f;
                for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
                    draw_list->AddLineV(canvas_p0.x + x, canvas_p0.y, canvas_p1.y, IM_COL32(200, 200, 200, 40));
                for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
                    draw_list->AddLineH(canvas_p0.x, canvas_p1.x, canvas_p0.y + y, IM_COL32(200, 200, 200, 40));
            }
            for (int n = 0; n < points.Size; n += 2)
                draw_list->AddLine(ImVec2(origin.x + points[n].x, origin.y + points[n].y), ImVec2(origin.x + points[n + 1].x, origin.y + points[n + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
            draw_list->PopClipRect();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("背景/前景の描画リスト"))
        {
            IMGUI_DEMO_MARKER("Examples/Custom rendering/BG & FG draw lists");
            static bool draw_bg = true;
            static bool draw_fg = true;
            ImGui::Checkbox("背景の描画リストに描く", &draw_bg);
            ImGui::SameLine(); HelpMarker("背景の描画リストは、すべての Dear ImGui ウィンドウより下に描画されます。");
            ImGui::Checkbox("前景の描画リストに描く", &draw_fg);
            ImGui::SameLine(); HelpMarker("前景の描画リストは、すべての Dear ImGui ウィンドウより上に描画されます。");
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
            if (draw_bg)
                ImGui::GetBackgroundDrawList()->AddCircle(window_center, window_size.x * 0.6f, IM_COL32(255, 0, 0, 200), 0, 10 + 4);
            if (draw_fg)
                ImGui::GetForegroundDrawList()->AddCircle(window_center, window_size.y * 0.6f, IM_COL32(0, 255, 0, 200), 0, 10);
            ImGui::EndTabItem();
        }

        // Demonstrate out-of-order rendering via channels splitting
        // We use functions in ImDrawList as each draw list contains a convenience splitter,
        // but you can also instantiate your own ImDrawListSplitter if you need to nest them.
        if (ImGui::BeginTabItem("描画チャンネル"))
        {
            IMGUI_DEMO_MARKER("Examples/Custom rendering/Draw Channels");
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            {
                ImGui::Text("青い図形を先に描画: 奥に表示されます");
                ImGui::Text("赤い図形を後に描画: 手前に表示されます");
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                draw_list->AddRectFilled(ImVec2(p0.x, p0.y), ImVec2(p0.x + 50, p0.y + 50), IM_COL32(0, 0, 255, 255)); // Blue
                draw_list->AddRectFilled(ImVec2(p0.x + 25, p0.y + 25), ImVec2(p0.x + 75, p0.y + 75), IM_COL32(255, 0, 0, 255)); // Red
                ImGui::Dummy(ImVec2(75, 75));
            }
            ImGui::Separator();
            {
                ImGui::Text("青い図形を先にチャンネル 1 へ描画: 手前に表示されます");
                ImGui::Text("赤い図形を後にチャンネル 0 へ描画: 奥に表示されます");
                ImVec2 p1 = ImGui::GetCursorScreenPos();

                // Create 2 channels and draw a Blue shape THEN a Red shape.
                // You can create any number of channels. Tables API use 1 channel per column in order to better batch draw calls.
                draw_list->ChannelsSplit(2);
                draw_list->ChannelsSetCurrent(1);
                draw_list->AddRectFilled(ImVec2(p1.x, p1.y), ImVec2(p1.x + 50, p1.y + 50), IM_COL32(0, 0, 255, 255)); // Blue
                draw_list->ChannelsSetCurrent(0);
                draw_list->AddRectFilled(ImVec2(p1.x + 25, p1.y + 25), ImVec2(p1.x + 75, p1.y + 75), IM_COL32(255, 0, 0, 255)); // Red

                // Flatten/reorder channels. Red shape is in channel 0 and it appears below the Blue shape in channel 1.
                // This works by copying draw indices only (vertices are not copied).
                draw_list->ChannelsMerge();
                ImGui::Dummy(ImVec2(75, 75));
                ImGui::Text("並べ替えたあとは、チャンネル 0 の内容がチャンネル 1 の下に表示されます。");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Docking, DockSpace / ShowExampleAppDockSpace()
//-----------------------------------------------------------------------------

struct ImGuiDemoDockspaceArgs
{
    bool                IsFullscreen = true;
    bool                KeepWindowPadding = false; // Keep WindowPadding to help understand that DockSpace() is a widget inside the window.
    ImGuiDockNodeFlags  DockSpaceFlags  = ImGuiDockNodeFlags_None;
};

// THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace().
// MOST REGULAR APPLICATIONS WANTING TO ALLOW DOCKING WINDOWS ON THE EDGE OF YOUR SCREEN CAN SIMPLY USE:
//    ImGui::NewFrame(); + ImGui::DockSpaceOverViewport();                                                   // Create a dockspace in main viewport
// OR:
//    ImGui::NewFrame(); + ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode); // Create a dockspace in main viewport, where central node is transparent.
// Demonstrate using DockSpace() to create an explicit docking node within an existing window, with various options.
// Read https://github.com/ocornut/imgui/wiki/Docking for details.
// The reasons we do not use DockSpaceOverViewport() in this demo is because:
// - (1) we allow the host window to be floating/moveable instead of filling the viewport (when args->IsFullscreen == false)
//       which is mostly to showcase the idea that DockSpace() may be submitted anywhere.
//       Also see 'Demo->Examples->Documents' for a less abstract version of this.
// - (2) we allow the host window to have padding (when args->UsePadding == true)
// - (3) we expose variety of other flags.
static void ShowExampleAppDockSpaceAdvanced(ImGuiDemoDockspaceArgs* args, bool* p_open)
{
    ImGuiDockNodeFlags dockspace_flags = args->DockSpaceFlags;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    if (args->IsFullscreen)
    {
        // Fullscreen dockspace: practically the same as calling DockSpaceOverViewport();
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;
    }
    else
    {
        // Floating dockspace
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!args->KeepWindowPadding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("ドックスペースを持つウィンドウ", p_open, window_flags);
    if (!args->KeepWindowPadding)
        ImGui::PopStyleVar();

    if (args->IsFullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace widget inside our window
    // - Note that the id here is different from the one used by DockSpaceOverViewport(), so docking state won't get transfered between "Basic" and "Advanced" demos.
    // - If we made the ShowExampleAppDockSpaceBasic() calculate its own ID and pass it to DockSpaceOverViewport() the ID could easily match.
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    ImGui::End();
}

static void ShowExampleAppDockSpaceBasic(ImGuiDockNodeFlags flags)
{
    // Basic version which you can use in many apps:
    // e.g:
    //   ImGui::DockSpaceOverViewport();
    // or:
    //   ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode); // Central node will be transparent
    // or:
    //   ImGuiViewport* viewport = ImGui::GetMainViewport();
    //   ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_None);

    ImGui::DockSpaceOverViewport(0, nullptr, flags);
}

void ShowExampleAppDockSpace(bool* p_open)
{
    static int opt_demo_mode = 0;
    static bool opt_demo_mode_changed = false;
    static ImGuiDemoDockspaceArgs args;

    if (opt_demo_mode == 0)
        ShowExampleAppDockSpaceBasic(args.DockSpaceFlags);
    else
        ShowExampleAppDockSpaceAdvanced(&args, p_open);

    // Refocus our window to minimize perceived loss of focus when changing mode (caused by the fact that each use a different window, which would not happen in a real app)
    if (opt_demo_mode_changed)
        ImGui::SetNextWindowFocus();
    ImGui::Begin("サンプル: ドックスペース", p_open, ImGuiWindowFlags_MenuBar);
    opt_demo_mode_changed = false;
    opt_demo_mode_changed |= ImGui::RadioButton("基本のデモ", &opt_demo_mode, 0);
    opt_demo_mode_changed |= ImGui::RadioButton("応用のデモ", &opt_demo_mode, 1);

    ImGui::SeparatorText("オプション");

    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0)
    {
        ShowDockingDisabledMessage();
    }
    else if (opt_demo_mode == 0)
    {
        args.DockSpaceFlags &= ImGuiDockNodeFlags_PassthruCentralNode; // Allowed flags
        ImGui::CheckboxFlags("Flag: PassthruCentralNode", &args.DockSpaceFlags, ImGuiDockNodeFlags_PassthruCentralNode);
    }
    else if (opt_demo_mode == 1)
    {
        ImGui::Checkbox("全画面", &args.IsFullscreen);
        ImGui::Checkbox("ウィンドウの余白を保つ", &args.KeepWindowPadding);
        ImGui::SameLine();
        HelpMarker("DockSpace() がウィンドウの「中」にあることを理解しやすくするために用意しています。");
        ImGui::BeginDisabled(args.IsFullscreen == false);
        ImGui::CheckboxFlags("Flag: PassthruCentralNode",      &args.DockSpaceFlags, ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::EndDisabled();
        ImGui::CheckboxFlags("Flag: NoDockingOverCentralNode", &args.DockSpaceFlags, ImGuiDockNodeFlags_NoDockingOverCentralNode);
        ImGui::CheckboxFlags("Flag: NoDockingSplit",           &args.DockSpaceFlags, ImGuiDockNodeFlags_NoDockingSplit);
        ImGui::CheckboxFlags("Flag: NoUndocking",              &args.DockSpaceFlags, ImGuiDockNodeFlags_NoUndocking);
        ImGui::CheckboxFlags("Flag: NoResize",                 &args.DockSpaceFlags, ImGuiDockNodeFlags_NoResize);
        ImGui::CheckboxFlags("Flag: AutoHideTabBar",           &args.DockSpaceFlags, ImGuiDockNodeFlags_AutoHideTabBar);
    }

    // Show demo options and help
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("ヘルプ"))
        {
            ImGui::TextUnformatted(
                "これは ImGui::DockSpace() の使用例です。別のウィンドウの「中」に\n手動でドッキングノードを作れます。\n" "\n"
                "「基本」版は ImGui::DockSpaceOverViewport() ヘルパーを使っています。たいていのアプリはこちらで十分でしょう。");
            ImGui::Separator();
            ImGui::TextUnformatted("ドッキングが有効なら、ほとんどのウィンドウを常に別のウィンドウへドッキングできます! 試してみてください!\n" "\n"
                "- ウィンドウのタイトルバーやタブからドラッグしてドッキング/解除。\n" "\n"
                "- ウィンドウメニューボタン (左上のボタン) からドラッグすると、ノード全体 (すべてのウィンドウ) を切り離せます。\n" "\n"
                "- SHIFT を押している間はドッキングが無効 (io.ConfigDockingWithShift == false の場合。既定)\n" "\n"
                "- SHIFT を押している間だけドッキングが有効 (io.ConfigDockingWithShift == true の場合)");
            ImGui::Separator();
            ImGui::TextUnformatted("詳細:"); ImGui::Bullet(); ImGui::SameLine(); ImGui::TextLinkOpenURL("ドッキングの Wiki ページ", "https://github.com/ocornut/imgui/wiki/Docking");
            ImGui::BulletText("ShowExampleAppDockSpace() のコメントを読んでください");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
//-----------------------------------------------------------------------------

// Simplified structure to mimic a Document model
struct MyDocument
{
    char        Name[32];   // Document title
    int         UID;        // Unique ID (necessary as we can change title)
    bool        Open;       // Set when open (we keep an array of all available documents to simplify demo code!)
    bool        OpenPrev;   // Copy of Open from last update.
    bool        Dirty;      // Set when the document has been modified
    ImVec4      Color;      // An arbitrary variable associated to the document

    MyDocument(int uid, const char* name, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        UID = uid;
        snprintf(Name, sizeof(Name), "%s", name);
        Open = OpenPrev = open;
        Dirty = false;
        Color = color;
    }
    void DoOpen()       { Open = true; }
    void DoForceClose() { Open = false; Dirty = false; }
    void DoSave()       { Dirty = false; }
};

struct ExampleAppDocuments
{
    ImVector<MyDocument>    Documents;
    ImVector<MyDocument*>   CloseQueue;
    MyDocument*             RenamingDoc = NULL;
    bool                    RenamingStarted = false;

    ExampleAppDocuments()
    {
        Documents.push_back(MyDocument(0, "レタス",             true,  ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument(1, "ナス",            true,  ImVec4(0.8f, 0.5f, 1.0f, 1.0f)));
        Documents.push_back(MyDocument(2, "ニンジン",              true,  ImVec4(1.0f, 0.8f, 0.5f, 1.0f)));
        Documents.push_back(MyDocument(3, "トマト",              false, ImVec4(1.0f, 0.3f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument(4, "かなり長いタイトル", false, ImVec4(0.4f, 0.8f, 0.8f, 1.0f)));
        Documents.push_back(MyDocument(5, "何かのドキュメント",       false, ImVec4(0.8f, 0.8f, 1.0f, 1.0f)));
    }

    // As we allow to change document name, we append a never-changing document ID so tabs are stable
    void GetTabName(MyDocument* doc, char* out_buf, size_t out_buf_size)
    {
        snprintf(out_buf, out_buf_size, "%s###doc%d", doc->Name, doc->UID);
    }

    // Display placeholder contents for the Document
    void DisplayDocContents(MyDocument* doc)
    {
        ImGui::PushID(doc);
        ImGui::Text("ドキュメント \"%s\"", doc->Name);
        ImGui::PushStyleColor(ImGuiCol_Text, doc->Color);
        ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        ImGui::PopStyleColor();

        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_R, ImGuiInputFlags_Tooltip);
        if (ImGui::Button("名前を変更.."))
        {
            RenamingDoc = doc;
            RenamingStarted = true;
        }
        ImGui::SameLine();

        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_M, ImGuiInputFlags_Tooltip);
        if (ImGui::Button("変更する"))
            doc->Dirty = true;

        ImGui::SameLine();
        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_Tooltip);
        if (ImGui::Button("保存"))
            doc->DoSave();

        ImGui::SameLine();
        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_W, ImGuiInputFlags_Tooltip);
        if (ImGui::Button("閉じる"))
            CloseQueue.push_back(doc);
        ImGui::ColorEdit3("色", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
        ImGui::PopID();
    }

    // Display context menu for the Document
    void DisplayDocContextMenu(MyDocument* doc)
    {
        if (!ImGui::BeginPopupContextItem())
            return;

        char buf[256];
        sprintf(buf, "%s を保存", doc->Name);
        if (ImGui::MenuItem(buf, "Ctrl+S", false, doc->Open))
            doc->DoSave();
        if (ImGui::MenuItem("名前を変更...", "Ctrl+R", false, doc->Open))
            RenamingDoc = doc;
        if (ImGui::MenuItem("閉じる", "Ctrl+W", false, doc->Open))
            CloseQueue.push_back(doc);
        ImGui::EndPopup();
    }

    // [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
    // If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo,
    // as opposed to clicking on the regular tab closing button) and stops being submitted, it will take a frame for
    // the tab bar to notice its absence. During this frame there will be a gap in the tab bar, and if the tab that has
    // disappeared was the selected one, the tab bar will report no selected tab during the frame. This will effectively
    // give the impression of a flicker for one frame.
    // We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
    // Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
    void NotifyOfDocumentsClosedElsewhere()
    {
        for (MyDocument& doc : Documents)
        {
            if (!doc.Open && doc.OpenPrev)
                ImGui::SetTabItemClosed(doc.Name);
            doc.OpenPrev = doc.Open;
        }
    }
};

void ShowExampleAppDocuments(bool* p_open)
{
    static ExampleAppDocuments app;

    // Options
    enum Target
    {
        Target_None,
        Target_Tab,                 // Create documents as local tab into a local tab bar
        Target_DockSpaceAndWindow   // Create documents as regular windows, and create an embedded dockspace
    };
    static Target opt_target = Target_Tab;
    static bool opt_reorderable = true;
    static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;

    // When (opt_target == Target_DockSpaceAndWindow) there is the possibily that one of our child Document window (e.g. "Eggplant")
    // that we emit gets docked into the same spot as the parent window ("Example: Documents").
    // This would create a problematic feedback loop because selecting the "Eggplant" tab would make the "Example: Documents" tab
    // not visible, which in turn would stop submitting the "Eggplant" window.
    // We avoid this problem by submitting our documents window even if our parent window is not currently visible.
    // Another solution may be to make the "Example: Documents" window use the ImGuiWindowFlags_NoDocking.

    bool window_contents_visible = ImGui::Begin("サンプル: ドキュメント", p_open, ImGuiWindowFlags_MenuBar);
    if (!window_contents_visible && opt_target != Target_DockSpaceAndWindow)
    {
        ImGui::End();
        return;
    }
    IMGUI_DEMO_MARKER("Examples/Documents");

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("ファイル"))
        {
            int open_count = 0;
            for (MyDocument& doc : app.Documents)
                open_count += doc.Open ? 1 : 0;

            if (ImGui::BeginMenu("開く", open_count < app.Documents.Size))
            {
                for (MyDocument& doc : app.Documents)
                    if (!doc.Open && ImGui::MenuItem(doc.Name))
                        doc.DoOpen();
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("すべてのドキュメントを閉じる", NULL, false, open_count > 0))
                for (MyDocument& doc : app.Documents)
                    app.CloseQueue.push_back(&doc);
            if (ImGui::MenuItem("終了") && p_open)
                *p_open = false;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // [Debug] List documents with one checkbox for each
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyDocument& doc = app.Documents[doc_n];
        if (doc_n > 0)
            ImGui::SameLine();
        ImGui::PushID(&doc);
        if (ImGui::Checkbox(doc.Name, &doc.Open))
            if (!doc.Open)
                doc.DoForceClose();
        ImGui::PopID();
    }
    ImGui::PushItemWidth(ImGui::GetFontSize() * 12);
    ImGui::Combo("出力先", (int*)&opt_target, "なし\0TabBar+Tabs\0DockSpace+Window\0");
    ImGui::PopItemWidth();
    bool redock_all = false;
    if (opt_target == Target_Tab)                { ImGui::SameLine(); ImGui::Checkbox("タブの並べ替えを許可", &opt_reorderable); }
    if (opt_target == Target_DockSpaceAndWindow) { ImGui::SameLine(); redock_all = ImGui::Button("すべて再ドッキング"); }

    ImGui::Separator();

    // About the ImGuiWindowFlags_UnsavedDocument / ImGuiTabItemFlags_UnsavedDocument flags.
    // They have multiple effects:
    // - Display a dot next to the title.
    // - Tab is selected when clicking the X close button.
    // - Closure is not assumed (will wait for user to stop submitting the tab).
    //   Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    //   We need to assume closure by default otherwise waiting for "lack of submission" on the next frame would leave an empty
    //   hole for one-frame, both in the tab-bar and in tab-contents when closing a tab/window.
    //   The rarely used SetTabItemClosed() function is a way to notify of programmatic closure to avoid the one-frame hole.

    // Tabs
    if (opt_target == Target_Tab)
    {
        ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
        tab_bar_flags |= ImGuiTabBarFlags_DrawSelectedOverline;
        if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
        {
            if (opt_reorderable)
                app.NotifyOfDocumentsClosedElsewhere();

            // [DEBUG] Stress tests
            //if ((ImGui::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
            //if (ImGui::GetIO().KeyCtrl) ImGui::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

            // Submit Tabs
            for (MyDocument& doc : app.Documents)
            {
                if (!doc.Open)
                    continue;

                // As we allow to change document name, we append a never-changing document id so tabs are stable
                char doc_name_buf[64];
                app.GetTabName(&doc, doc_name_buf, sizeof(doc_name_buf));
                ImGuiTabItemFlags tab_flags = (doc.Dirty ? ImGuiTabItemFlags_UnsavedDocument : 0);
                bool visible = ImGui::BeginTabItem(doc_name_buf, &doc.Open, tab_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc.Open && doc.Dirty)
                {
                    doc.Open = true;
                    app.CloseQueue.push_back(&doc);
                }

                app.DisplayDocContextMenu(&doc);
                if (visible)
                {
                    app.DisplayDocContents(&doc);
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }
    }
    else if (opt_target == Target_DockSpaceAndWindow)
    {
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            app.NotifyOfDocumentsClosedElsewhere();

            // Create a DockSpace node where any window can be docked
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id);

            // Create Windows
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
            {
                MyDocument* doc = &app.Documents[doc_n];
                if (!doc->Open)
                    continue;

                ImGui::SetNextWindowDockID(dockspace_id, redock_all ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
                ImGuiWindowFlags window_flags = (doc->Dirty ? ImGuiWindowFlags_UnsavedDocument : 0);
                bool visible = ImGui::Begin(doc->Name, &doc->Open, window_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc->Open && doc->Dirty)
                {
                    doc->Open = true;
                    app.CloseQueue.push_back(doc);
                }

                app.DisplayDocContextMenu(doc);
                if (visible)
                    app.DisplayDocContents(doc);

                ImGui::End();
            }
        }
        else
        {
            ShowDockingDisabledMessage();
        }
    }

    // Early out other contents
    if (!window_contents_visible)
    {
        ImGui::End();
        return;
    }

    // Display renaming UI
    if (app.RenamingDoc != NULL)
    {
        if (app.RenamingStarted)
            ImGui::OpenPopup("Rename");
        if (ImGui::BeginPopup("Rename"))
        {
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 30);
            if (ImGui::InputText("###Name", app.RenamingDoc->Name, IM_COUNTOF(app.RenamingDoc->Name), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                ImGui::CloseCurrentPopup();
                app.RenamingDoc = NULL;
            }
            if (app.RenamingStarted)
                ImGui::SetKeyboardFocusHere(-1);
            ImGui::EndPopup();
        }
        else
        {
            app.RenamingDoc = NULL;
        }
        app.RenamingStarted = false;
    }

    // Display closing confirmation UI
    if (!app.CloseQueue.empty())
    {
        int close_queue_unsaved_documents = 0;
        for (int n = 0; n < app.CloseQueue.Size; n++)
            if (app.CloseQueue[n]->Dirty)
                close_queue_unsaved_documents++;

        if (close_queue_unsaved_documents == 0)
        {
            // Close documents when all are unsaved
            for (int n = 0; n < app.CloseQueue.Size; n++)
                app.CloseQueue[n]->DoForceClose();
            app.CloseQueue.clear();
        }
        else
        {
            if (!ImGui::IsPopupOpen("保存しますか?"))
                ImGui::OpenPopup("保存しますか?");
            if (ImGui::BeginPopupModal("保存しますか?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("次の項目の変更を保存しますか?");
                float item_height = ImGui::GetTextLineHeightWithSpacing();
                if (ImGui::BeginChild(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height), ImGuiChildFlags_FrameStyle))
                    for (MyDocument* doc : app.CloseQueue)
                        if (doc->Dirty)
                            ImGui::Text("%s", doc->Name);
                ImGui::EndChild();

                ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
                if (ImGui::Button("はい", button_size))
                {
                    for (MyDocument* doc : app.CloseQueue)
                    {
                        if (doc->Dirty)
                            doc->DoSave();
                        doc->DoForceClose();
                    }
                    app.CloseQueue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("いいえ", button_size))
                {
                    for (MyDocument* doc : app.CloseQueue)
                        doc->DoForceClose();
                    app.CloseQueue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("キャンセル", button_size))
                {
                    app.CloseQueue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Assets Browser / ShowExampleAppAssetsBrowser()
//-----------------------------------------------------------------------------

//#include "imgui_internal.h" // NavMoveRequestTryWrapping()

struct ExampleAsset
{
    ImGuiID ID;
    int     Type;

    ExampleAsset(ImGuiID id, int type) { ID = id; Type = type; }

    static const ImGuiTableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ExampleAsset* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), ExampleAsset::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const ExampleAsset* a = (const ExampleAsset*)lhs;
        const ExampleAsset* b = (const ExampleAsset*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            if (sort_spec->ColumnIndex == 0)
                delta = ((int)a->ID - (int)b->ID);
            else if (sort_spec->ColumnIndex == 1)
                delta = (a->Type - b->Type);
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }
        return (int)a->ID - (int)b->ID;
    }
};
const ImGuiTableSortSpecs* ExampleAsset::s_current_sort_specs = NULL;

struct ExampleAssetsBrowser
{
    // Options
    bool            ShowTypeOverlay = true;
    bool            AllowSorting = true;
    bool            AllowBoxSelect = true;                  // Will set ImGuiMultiSelectFlags_BoxSelect2d
    bool            AllowBoxSelectInsideSelection = false;  // Will set ImGuiMultiSelectFlags_SelectOnClickAlways
    bool            AllowDragUnselected = false;            // Will set ImGuiMultiSelectFlags_SelectOnClickRelease
    float           IconSize = 0;
    int             IconSpacing = 10;
    int             IconHitSpacing = 4;                     // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    bool            StretchSpacing = true;
    bool            UseScrollX = false;                     // Debug: submit twice the number of items per line (overflow horizontally to exercise ScrollX + box-select)

    // State
    ImVector<ExampleAsset> Items;               // Our items
    ExampleSelectionWithDeletion Selection;     // Our selection (ImGuiSelectionBasicStorage + helper funcs to handle deletion)
    ImGuiID         NextItemId = 0;             // Unique identifier when creating new items
    bool            RequestDelete = false;      // Deferred deletion request
    bool            RequestSort = false;        // Deferred sort request
    float           ZoomWheelAccum = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better

    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    ImVec2          LayoutItemSize;
    ImVec2          LayoutItemStep;             // == LayoutItemSize + LayoutItemSpacing
    float           LayoutItemSpacing = 0.0f;
    float           LayoutSelectableSpacing = 0.0f;
    float           LayoutOuterPadding = 0.0f;
    int             LayoutColumnCount = 0;
    int             LayoutLineCount = 0;

    // Functions
    ExampleAssetsBrowser()
    {
        AddItems(10000);
    }
    void AddItems(int count)
    {
        if (Items.Size == 0)
            NextItemId = 0;
        Items.reserve(Items.Size + count);
        for (int n = 0; n < count; n++, NextItemId++)
            Items.push_back(ExampleAsset(NextItemId, (NextItemId % 20) < 15 ? 0 : (NextItemId % 20) < 18 ? 1 : 2));
        RequestSort = true;
    }
    void ClearItems()
    {
        Items.clear();
        Selection.Clear();
    }

    // Logic would be written in the main code BeginChild() and outputting to local variables.
    // We extracted it into a function so we can call it easily from multiple places.
    void UpdateLayoutSizes(float avail_width)
    {
        // Layout: when not stretching: allow extending into right-most spacing.
        LayoutItemSpacing = (float)IconSpacing;
        if (StretchSpacing == false)
            avail_width += floorf(LayoutItemSpacing * 0.5f);

        // Layout: calculate number of icon per line and number of lines
        LayoutItemSize = ImVec2(floorf(IconSize), floorf(IconSize));
        LayoutColumnCount = IM_MAX((int)(avail_width / (LayoutItemSize.x + LayoutItemSpacing)), 1);

        // Layout: when stretching: allocate remaining space to more spacing. Round before division, so item_spacing may be non-integer.
        if (StretchSpacing && LayoutColumnCount > 1)
            LayoutItemSpacing = floorf(avail_width - LayoutItemSize.x * LayoutColumnCount) / LayoutColumnCount;

        if (UseScrollX)
            LayoutColumnCount *= 2;
        LayoutLineCount = (Items.Size + LayoutColumnCount - 1) / LayoutColumnCount;

        LayoutItemStep = ImVec2(LayoutItemSize.x + LayoutItemSpacing, LayoutItemSize.y + LayoutItemSpacing);
        LayoutSelectableSpacing = IM_MAX(floorf(LayoutItemSpacing) - IconHitSpacing, 0.0f);
        LayoutOuterPadding = floorf(LayoutItemSpacing * 0.5f);
    }

    void Draw(const char* title, bool* p_open)
    {
        if (IconSize <= 0.0f)
            IconSize = ImGui::CalcTextSize("99999").x;

        ImGui::SetNextWindowSize(ImVec2(IconSize * 25, IconSize * 15), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar))
        {
            ImGui::End();
            return;
        }
        IMGUI_DEMO_MARKER("Examples/Assets Browser");

        // Menu bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("ファイル"))
            {
                if (ImGui::MenuItem("10000 項目を追加"))
                    AddItems(10000);
                if (ImGui::MenuItem("項目をすべて消去"))
                    ClearItems();
                ImGui::Separator();
                if (ImGui::MenuItem("閉じる", NULL, false, p_open != NULL))
                    *p_open = false;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("編集"))
            {
                if (ImGui::MenuItem("削除", "Del", false, Selection.Size > 0))
                    RequestDelete = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("オプション"))
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * 10);

                ImGui::SeparatorText("内容");
                ImGui::Checkbox("種類のオーバーレイを表示", &ShowTypeOverlay);
                ImGui::Checkbox("並べ替えを許可", &AllowSorting);

                ImGui::SeparatorText("選択の挙動");
                ImGui::Checkbox("矩形選択を許可", &AllowBoxSelect);
                if (ImGui::Checkbox("選択済みの項目からの矩形選択を許可", &AllowBoxSelectInsideSelection) && AllowBoxSelectInsideSelection)
                    AllowDragUnselected = false;
                if (ImGui::Checkbox("未選択の項目のドラッグを許可", &AllowDragUnselected) && AllowDragUnselected)
                    AllowBoxSelectInsideSelection = false;

                ImGui::SeparatorText("レイアウト");
                ImGui::SliderFloat("アイコンのサイズ", &IconSize, 16.0f, 128.0f, "%.0f");
                ImGui::SameLine(); HelpMarker("Ctrl+ホイールで拡大縮小できます");
                ImGui::SliderInt("アイコンの間隔", &IconSpacing, 0, 32);
                ImGui::SliderInt("アイコンの当たり判定の間隔", &IconHitSpacing, 0, 32);
                ImGui::Checkbox("間隔を引き伸ばす", &StretchSpacing);
                ImGui::Checkbox("ScrollX を使う", &UseScrollX);
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Show a table with ONLY one header row to showcase the idea/possibility of using this to provide a sorting UI
        if (AllowSorting)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGuiTableFlags table_flags_for_sort_specs = ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders;
            if (ImGui::BeginTable("for_sort_specs_only", 2, table_flags_for_sort_specs, ImVec2(0.0f, ImGui::GetFrameHeight())))
            {
                ImGui::TableSetupColumn("番号");
                ImGui::TableSetupColumn("種類");
                ImGui::TableHeadersRow();
                if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                    if (sort_specs->SpecsDirty || RequestSort)
                    {
                        ExampleAsset::SortWithSortSpecs(sort_specs, Items.Data, Items.Size);
                        sort_specs->SpecsDirty = RequestSort = false;
                    }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar();
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowContentSize(ImVec2(0.0f, LayoutOuterPadding + LayoutLineCount * (LayoutItemSize.y + LayoutItemSpacing)));
        if (ImGui::BeginChild("Assets", ImVec2(0.0f, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            const float avail_width = ImGui::GetContentRegionAvail().x;
            UpdateLayoutSizes(avail_width);

            // Calculate and store start position.
            ImVec2 start_pos = ImGui::GetCursorScreenPos();
            start_pos = ImVec2(start_pos.x + LayoutOuterPadding, start_pos.y + LayoutOuterPadding);
            ImGui::SetCursorScreenPos(start_pos);

            // Multi-select
            ImGuiMultiSelectFlags ms_flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;

            // - Enable box-select (in 2D mode, so that changing box-select rectangle X1/X2 boundaries will affect clipped items)
            if (AllowBoxSelect)
                ms_flags |= ImGuiMultiSelectFlags_BoxSelect2d;

            // - Selection mode
            if (AllowDragUnselected)
                ms_flags |= ImGuiMultiSelectFlags_SelectOnClickRelease; // Rarely used: Allows dragging an unselected item without selecting it(rarely used)
            else if (AllowBoxSelectInsideSelection)
                ms_flags |= ImGuiMultiSelectFlags_SelectOnClickAlways; // Rarely used: Prevents Drag and Drop from being used on multiple-selection, but allows e.g. BoxSelect to always reselect even when clicking inside an existing selection.

            // - Enable keyboard wrapping on X axis
            // (FIXME-MULTISELECT: We haven't designed/exposed a general nav wrapping api yet, so this flag is provided as a courtesy to avoid doing:
            //    ImGui::NavMoveRequestTryWrapping(ImGui::GetCurrentWindow(), ImGuiNavMoveFlags_WrapX);
            // When we finish implementing a more general API for this, we will obsolete this flag in favor of the new system)
            ms_flags |= ImGuiMultiSelectFlags_NavWrapX;

            ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(ms_flags, Selection.Size, Items.Size);

            // Use custom selection adapter: store ID in selection (recommended)
            Selection.UserData = this;
            Selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self_, int idx) { ExampleAssetsBrowser* self = (ExampleAssetsBrowser*)self_->UserData; return self->Items[idx].ID; };
            Selection.ApplyRequests(ms_io);

            const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (Selection.Size > 0)) || RequestDelete;
            const int item_curr_idx_to_focus = want_delete ? Selection.ApplyDeletionPreLoop(ms_io, Items.Size) : -1;
            RequestDelete = false;

            // Push LayoutSelectableSpacing (which is LayoutItemSpacing minus hit-spacing, if we decide to have hit gaps between items)
            // Altering style ItemSpacing may seem unnecessary as we position every items using SetCursorScreenPos()...
            // But it is necessary for two reasons:
            // - Selectables uses it by default to visually fill the space between two items.
            // - The vertical spacing would be measured by Clipper to calculate line height if we didn't provide it explicitly (here we do).
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(LayoutSelectableSpacing, LayoutSelectableSpacing));

            // Rendering parameters
            const ImU32 icon_type_overlay_colors[3] = { 0, IM_COL32(200, 70, 70, 255), IM_COL32(70, 170, 70, 255) };
            const ImU32 icon_bg_color = ImGui::GetColorU32(IM_COL32(35, 35, 35, 220));
            const ImVec2 icon_type_overlay_size = ImVec2(4.0f, 4.0f);
            const bool display_label = (LayoutItemSize.x >= ImGui::CalcTextSize("999").x);

            const int column_count = LayoutColumnCount;
            ImGuiListClipper clipper;
            clipper.Begin(LayoutLineCount, LayoutItemStep.y);
            if (item_curr_idx_to_focus != -1)
                clipper.IncludeItemByIndex(item_curr_idx_to_focus / column_count); // Ensure focused item line is not clipped.
            if (ms_io->RangeSrcItem != -1)
                clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem / column_count); // Ensure RangeSrc item line is not clipped.
            while (clipper.Step())
            {
                for (int line_idx = clipper.DisplayStart; line_idx < clipper.DisplayEnd; line_idx++)
                {
                    const int item_min_idx_for_current_line = line_idx * column_count;
                    const int item_max_idx_for_current_line = IM_MIN((line_idx + 1) * column_count, Items.Size);
                    for (int item_idx = item_min_idx_for_current_line; item_idx < item_max_idx_for_current_line; ++item_idx)
                    {
                        ExampleAsset* item_data = &Items[item_idx];
                        ImGui::PushID((int)item_data->ID);

                        // Position item
                        ImVec2 pos = ImVec2(start_pos.x + (item_idx % column_count) * LayoutItemStep.x, start_pos.y + line_idx * LayoutItemStep.y);
                        ImGui::SetCursorScreenPos(pos);

                        ImGui::SetNextItemSelectionUserData(item_idx);
                        bool item_is_selected = Selection.Contains((ImGuiID)item_data->ID);
                        bool item_is_visible = ImGui::IsRectVisible(LayoutItemSize);
                        ImGui::Selectable("", item_is_selected, ImGuiSelectableFlags_None, LayoutItemSize);

                        // Update our selection state immediately (without waiting for EndMultiSelect() requests)
                        // because we use this to alter the color of our text/icon.
                        if (ImGui::IsItemToggledSelection())
                            item_is_selected = !item_is_selected;

                        // Focus (for after deletion)
                        if (item_curr_idx_to_focus == item_idx)
                            ImGui::SetKeyboardFocusHere(-1);

                        // Drag and drop
                        if (ImGui::BeginDragDropSource())
                        {
                            // Create payload with full selection OR single unselected item.
                            // (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
                            if (ImGui::GetDragDropPayload() == NULL)
                            {
                                ImVector<ImGuiID> payload_items;
                                void* it = NULL;
                                ImGuiID id = 0;
                                if (!item_is_selected)
                                    payload_items.push_back(item_data->ID);
                                else
                                    while (Selection.GetNextSelectedItem(&it, &id))
                                        payload_items.push_back(id);
                                ImGui::SetDragDropPayload("ASSETS_BROWSER_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
                            }

                            // Display payload content in tooltip, by extracting it from the payload data
                            // (we could read from selection, but it is more correct and reusable to read from payload)
                            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                            const int payload_count = (int)payload->DataSize / (int)sizeof(ImGuiID);
                            ImGui::Text("%d 個のアセット", payload_count);

                            ImGui::EndDragDropSource();
                        }

                        // Render icon (a real app would likely display an image/thumbnail here)
                        // Because we use ImGuiMultiSelectFlags_BoxSelect2d, clipping vertical may occasionally be larger, so we coarse-clip our rendering as well.
                        if (item_is_visible)
                        {
                            ImVec2 box_min(pos.x - 1, pos.y - 1);
                            ImVec2 box_max(box_min.x + LayoutItemSize.x + 2, box_min.y + LayoutItemSize.y + 2); // Dubious
                            draw_list->AddRectFilled(box_min, box_max, icon_bg_color); // Background color
                            if (ShowTypeOverlay && item_data->Type != 0)
                            {
                                ImU32 type_col = icon_type_overlay_colors[item_data->Type % IM_COUNTOF(icon_type_overlay_colors)];
                                draw_list->AddRectFilled(ImVec2(box_max.x - 2 - icon_type_overlay_size.x, box_min.y + 2), ImVec2(box_max.x - 2, box_min.y + 2 + icon_type_overlay_size.y), type_col);
                            }
                            if (display_label)
                            {
                                ImU32 label_col = ImGui::GetColorU32(item_is_selected ? ImGuiCol_Text : ImGuiCol_TextDisabled);
                                char label[32];
                                sprintf(label, "%d", item_data->ID);
                                draw_list->AddText(ImVec2(box_min.x, box_max.y - ImGui::GetFontSize()), label_col, label);
                            }
                        }

                        ImGui::PopID();
                    }
                }
            }
            clipper.End();
            if (Items.Size == 0)
                ImGui::Dummy(ImVec2(0, 0));
            ImGui::PopStyleVar(); // ImGuiStyleVar_ItemSpacing

            // Context menu
            if (ImGui::BeginPopupContextWindow())
            {
                ImGui::Text("選択中: %d 項目", Selection.Size);
                ImGui::Separator();
                if (ImGui::MenuItem("削除", "Del", false, Selection.Size > 0))
                    RequestDelete = true;
                ImGui::EndPopup();
            }

            ms_io = ImGui::EndMultiSelect();
            Selection.ApplyRequests(ms_io);
            if (want_delete)
                Selection.ApplyDeletionPostLoop(ms_io, Items, item_curr_idx_to_focus);

            // Zooming with Ctrl+Wheel
            if (ImGui::IsWindowAppearing())
                ZoomWheelAccum = 0.0f;
            if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsAnyItemActive() == false)
            {
                ZoomWheelAccum += io.MouseWheel;
                if (fabsf(ZoomWheelAccum) >= 1.0f)
                {
                    // Calculate hovered item index from mouse location
                    // FIXME: Locking aiming on 'hovered_item_idx' (with a cool-down timer) would ensure zoom keeps on it.
                    const float hovered_item_nx = (io.MousePos.x - start_pos.x + LayoutItemSpacing * 0.5f) / LayoutItemStep.x;
                    const float hovered_item_ny = (io.MousePos.y - start_pos.y + LayoutItemSpacing * 0.5f) / LayoutItemStep.y;
                    const int hovered_item_idx = ((int)hovered_item_ny * LayoutColumnCount) + (int)hovered_item_nx;
                    //ImGui::SetTooltip("%f,%f -> item %d", hovered_item_nx, hovered_item_ny, hovered_item_idx); // Move those 4 lines in block above for easy debugging

                    // Zoom
                    IconSize *= powf(1.1f, (float)(int)ZoomWheelAccum);
                    IconSize = IM_CLAMP(IconSize, 16.0f, 128.0f);
                    ZoomWheelAccum -= (int)ZoomWheelAccum;
                    UpdateLayoutSizes(avail_width);

                    // Manipulate scroll to that we will land at the same Y location of currently hovered item.
                    // - Calculate next frame position of item under mouse
                    // - Set new scroll position to be used in next ImGui::BeginChild() call.
                    float hovered_item_rel_pos_y = ((float)(hovered_item_idx / LayoutColumnCount) + fmodf(hovered_item_ny, 1.0f)) * LayoutItemStep.y;
                    hovered_item_rel_pos_y += ImGui::GetStyle().WindowPadding.y;
                    float mouse_local_y = io.MousePos.y - ImGui::GetWindowPos().y;
                    ImGui::SetScrollY(hovered_item_rel_pos_y - mouse_local_y);
                }
            }
        }
        ImGui::EndChild();

        ImGui::Text("選択中: %d/%d 項目", Selection.Size, Items.Size);
        ImGui::End();
    }
};

void ShowExampleAppAssetsBrowser(bool* p_open)
{
    IMGUI_DEMO_MARKER("Examples/Assets Browser");
    static ExampleAssetsBrowser assets_browser;
    assets_browser.Draw("サンプル: アセットブラウザー", p_open);
}

// End of Demo code
#else

void ImGui::ShowAboutWindow(bool*) {}
void ImGui::ShowDemoWindow(bool*) {}
void ImGui::ShowUserGuide() {}
void ImGui::ShowStyleEditor(ImGuiStyle*) {}
bool ImGui::ShowStyleSelector(const char*) { return false; }

#endif // #ifndef IMGUI_DISABLE_DEMO_WINDOWS

#endif // #ifndef IMGUI_DISABLE
