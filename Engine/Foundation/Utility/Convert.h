#pragma once
/*====================================
 *
 * C++標準文字列型（std::string）とワイド文字列（std::wstring）の相互変換を行うユーティリティ。
 * DirectXやWindowsAPIはワイド文字列を要求することが多く、
 * エンジン内部のstd::stringとの橋渡し役として機能する。
 *
 * ====================================*/
#include <string>

// 文字列変換関数(std::string -> std::wstring).
std::wstring ConvertString(const std::string& str);
// 文字列変換関数(std::wstring -> std::string).
std::string ConvertString(const std::wstring& str);
