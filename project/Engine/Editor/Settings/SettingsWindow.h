#pragma once
/*====================================
 *
 * Project Settings / Preferences の2つのウィンドウ。
 *
 * Editor 本体から切り離してあるのは、設定項目を増やすたびに Editor.cpp が
 * 膨らむのを避けるため。ここは値を編集するだけで、いつ保存するかは各ウィンドウが決める。
 *
 * 【保存の方針が2つで違う】
 * ProjectSettings は共有データなので Save ボタンでしか書かない。
 * Preferences は個人設定なので、変更した瞬間に書く（押し忘れて消える方が困る）。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include <Windows.h>

namespace Cake {

struct ProjectSettings;
struct EditorPreferences;
class ImGuiManager;

// hwnd はウィンドウタイトルを即時反映するために使う。nullptr 可.
void DrawProjectSettingsWindow(ProjectSettings& settings, bool* open);

// テーマ変更を即時反映するため ImGuiManager を受け取る.
void DrawPreferencesWindow(EditorPreferences& preferences, ImGuiManager& imguiManager, bool* open);

} // namespace Cake

#endif // ENABLE_EDITOR
