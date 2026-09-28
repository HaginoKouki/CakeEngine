#pragma once
/*====================================
 *
 * ビルド構成フラグの一元管理。
 *
 * 2つの独立した軸を扱う:
 *   USE_IMGUI     : ImGui ライブラリを使うか（include・リンク・ImGui API 呼び出しの可否）。
 *   ENABLE_EDITOR : エディタ機能を有効にするか（Editorクラス、ビューポート表示、
 *                   インスペクタ、ESCカメラ切り替え等）。
 *
 * プロジェクト設定でのプリプロセッサ定義:
 *   Debug   : _DEBUG / _WINDOWS / USE_IMGUI / ENABLE_EDITOR
 *   Release : NDEBUG / _WINDOWS
 *
 * 使い分け:
 *   - 純粋に ImGui API を叩くだけの箇所        → #ifdef USE_IMGUI
 *   - エディタ機能そのもの（Editorクラス等）    → #ifdef ENABLE_EDITOR
 *
 * ====================================*/

// エディタ機能は現状すべて ImGui 実装に依存している。
// よって ENABLE_EDITOR が立つときは USE_IMGUI も必ず立っていなければならない。
// この包含関係が崩れた場合はコンパイル時に検出する。
#ifdef ENABLE_EDITOR
	#ifndef USE_IMGUI
		#error "ENABLE_EDITOR requires USE_IMGUI. Define USE_IMGUI in the same build configuration."
	#endif
#endif
