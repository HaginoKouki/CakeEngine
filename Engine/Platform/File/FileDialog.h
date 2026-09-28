#pragma once
/*====================================
 *
 * OS のファイル選択ダイアログを開くためのラッパー。
 * Editor がファイルパスをユーザーに決めさせる唯一の手段。
 *
 * 【必ず相対パスを返す】
 * ダイアログは絶対パスを返すが、そのまま .scene などへ保存すると
 * 別のPCで開けなくなる。実行ディレクトリからの相対パスへ変換して返す。
 * 区切りは '/' に揃える（プロジェクト内の他のパス表記と合わせるため）。
 *
 * 【カレントディレクトリを変えない】
 * 実装では OFN_NOCHANGEDIR を必ず指定している。これが無いと、
 * ユーザーが別フォルダを開いた瞬間にプロセスのカレントが移動し、
 * 以降 "Assets/..." 形式の相対パスが全て解決できなくなる。
 *
 * 【フィルタは分けて渡す】
 * Win32 のフィルタは二重ヌル終端という特殊な形式のため、
 * 説明とパターンを別々に受け取って内部で組み立てる。
 * 呼び出し側が "\0" を書く必要はない。
 *
 * 【戻り値】
 * キャンセル時は false を返し、out は変更しない。必ず戻り値を見ること。
 *
 * Windows 専用。移植の際はこのファイルを差し替える.
 *
 * ====================================*/
#include <string>

namespace Cake {

// 既存のファイルを選ばせる。
//   filterName    : "Scene Files" のような表示名.
//   filterPattern : "*.scene" のようなパターン.
//   initialDir    : 最初に開くフォルダ（相対可）。存在しなければ無視される.
bool OpenFileDialog(
	const char* title,
	const char* filterName,
	const char* filterPattern,
	const std::string& initialDir,
	std::string& out
);

// 保存先を選ばせる。既存ファイルなら上書き確認が出る。
//   defaultExt : 拡張子を省略された場合に補う（"scene" のようにドット無しで渡す）.
bool SaveFileDialog(
	const char* title,
	const char* filterName,
	const char* filterPattern,
	const std::string& initialDir,
	const char* defaultExt,
	std::string& out
);

} // namespace Cake
