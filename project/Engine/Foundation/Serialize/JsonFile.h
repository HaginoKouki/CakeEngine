#pragma once
/*====================================
 *
 * JSON ファイルの読み書きだけを担う薄いヘルパ。
 *
 * ProjectSettings / EditorPreferences / EditorCache の3つが、いずれも
 * 「開く → parse → 例外を握り潰してログ」という同じ定型を必要とするため、
 * ここへ1本化する。設定を1つ増やすたびに try/catch を書き写さずに済む。
 *
 * 【書き込み時は親フォルダを作る】
 * UserSettings/ はリポジトリに存在しない（gitignore対象）ため、
 * 初回起動では必ずフォルダごと無い。ここで作らないと毎回書き込みに失敗する。
 *
 * 【失敗しても落とさない】
 * 設定ファイルが壊れていてもエンジンは既定値で起動できるべきなので、
 * ログを出して false を返すだけにする。assert も throw もしない。
 *
 * ====================================*/
#include <string>

#include "externals/nlohmann/json.hpp"

namespace Cake {

// 読み込む。ファイルが無い・壊れている場合は false（out は変更しない）.
bool ReadJsonFile(const std::string& path, nlohmann::json& out, const char* logCategory);

// 書き込む。親フォルダが無ければ作る.
bool WriteJsonFile(const std::string& path, const nlohmann::json& root, const char* logCategory);

} // namespace Cake
