#include "FileDialog.h"

#include <Windows.h>
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")

#include <filesystem>
#include <vector>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Utility/Convert.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "FileDialog";

// パス長。MAX_PATH(260) では足りない環境があるため広めに取る.
constexpr size_t kPathBufferSize = 1024;

// Win32 のフィルタ形式（"説明\0パターン\0\0"）を組み立てる。
// 途中に \0 を含むため std::wstring のまま扱い、data() を渡す.
std::vector<wchar_t> BuildFilter(const char* filterName, const char* filterPattern) {
	const std::wstring name = ConvertString(filterName);
	const std::wstring pattern = ConvertString(filterPattern);

	std::vector<wchar_t> buffer;
	buffer.insert(buffer.end(), name.begin(), name.end());
	buffer.push_back(L'\0');
	buffer.insert(buffer.end(), pattern.begin(), pattern.end());
	buffer.push_back(L'\0');
	buffer.push_back(L'\0'); // 終端はヌル2つ.
	return buffer;
}

// initialDir を絶対パスへ直す。存在しなければ空を返す（ダイアログ側が既定を使う）.
std::wstring ResolveInitialDir(const std::string& initialDir) {
	if (initialDir.empty()) {
		return {};
	}
	std::error_code ec;
	const std::filesystem::path absolute = std::filesystem::absolute(initialDir, ec);
	if (ec || !std::filesystem::is_directory(absolute, ec)) {
		return {};
	}
	return absolute.wstring();
}

// ダイアログが返した絶対パスを、実行ディレクトリからの相対パスへ直す.
std::string ToRelativePath(const std::wstring& absolute) {
	std::error_code ec;
	const std::filesystem::path current = std::filesystem::current_path(ec);
	if (ec) {
		return ConvertString(absolute);
	}

	std::filesystem::path relative = std::filesystem::relative(absolute, current, ec);
	if (ec || relative.empty()) {
		// 別ドライブなどで相対化できない場合は絶対パスのまま返す.
		DebugLog::GetInstance().Log(
			LogLevel::Warn, kLogCategory,
			"プロジェクト外のパスが選ばれました。絶対パスのまま扱います"
		);
		return ConvertString(absolute);
	}

	std::string result = relative.generic_string(); // 区切りを '/' に揃える.
	if (result.rfind("..", 0) == 0) {
		DebugLog::GetInstance().Log(
			LogLevel::Warn, kLogCategory,
			"プロジェクト外のパスが選ばれました: " + result
		);
	}
	return result;
}

// OPENFILENAMEW の共通部分を埋める.
void FillCommon(
	OPENFILENAMEW& ofn,
	std::vector<wchar_t>& filter,
	std::vector<wchar_t>& pathBuffer,
	const std::wstring& initialDir,
	const std::wstring& title
) {
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFilter = filter.data();
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = pathBuffer.data();
	ofn.nMaxFile = static_cast<DWORD>(pathBuffer.size());
	ofn.lpstrTitle = title.c_str();
	if (!initialDir.empty()) {
		ofn.lpstrInitialDir = initialDir.c_str();
	}
	// OFN_NOCHANGEDIR は必須。
	// これが無いとダイアログがプロセスのカレントを移動させ、
	// 以降 "Assets/..." 形式の相対パスが全て解決できなくなる.
	ofn.Flags = OFN_NOCHANGEDIR | OFN_EXPLORER;
}

// キャンセルとエラーを区別してログを出す.
void LogIfError() {
	const DWORD error = CommDlgExtendedError();
	if (error != 0) { // 0 はユーザーによるキャンセル.
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			"ダイアログの表示に失敗しました (code: " + std::to_string(error) + ")"
		);
	}
}

} // namespace

bool OpenFileDialog(
	const char* title,
	const char* filterName,
	const char* filterPattern,
	const std::string& initialDir,
	std::string& out
) {
	std::vector<wchar_t> filter = BuildFilter(filterName, filterPattern);
	std::vector<wchar_t> pathBuffer(kPathBufferSize, L'\0');
	const std::wstring dir = ResolveInitialDir(initialDir);
	const std::wstring titleW = ConvertString(title);

	OPENFILENAMEW ofn{};
	FillCommon(ofn, filter, pathBuffer, dir, titleW);
	ofn.Flags |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (!GetOpenFileNameW(&ofn)) {
		LogIfError();
		return false; // キャンセル時は out を変更しない.
	}

	out = ToRelativePath(pathBuffer.data());
	return true;
}

bool SaveFileDialog(
	const char* title,
	const char* filterName,
	const char* filterPattern,
	const std::string& initialDir,
	const char* defaultExt,
	std::string& out
) {
	std::vector<wchar_t> filter = BuildFilter(filterName, filterPattern);
	std::vector<wchar_t> pathBuffer(kPathBufferSize, L'\0');
	const std::wstring dir = ResolveInitialDir(initialDir);
	const std::wstring titleW = ConvertString(title);
	const std::wstring extW = ConvertString(defaultExt);

	OPENFILENAMEW ofn{};
	FillCommon(ofn, filter, pathBuffer, dir, titleW);
	ofn.Flags |= OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = extW.c_str(); // 拡張子を省略されたら補う.

	if (!GetSaveFileNameW(&ofn)) {
		LogIfError();
		return false;
	}

	out = ToRelativePath(pathBuffer.data());
	return true;
}

} // namespace Cake
