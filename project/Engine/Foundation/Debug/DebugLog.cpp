#include "DebugLog.h"

#include <cassert>
#include <winerror.h>
#include <format>
#include <string>
#include <sstream>
#include <iomanip>
// ファイルやディレクトリに関する操作を行うライブラリ.
#include <filesystem>
// 時間を扱うライブラリ.
#include <chrono>

namespace Cake {

DebugLog::DebugLog() {
	// 現在時刻を取得.
	auto now = std::chrono::system_clock::now();
	auto nowSecond = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{std::chrono::current_zone(), nowSecond};

	// 日付フォルダ名（YYYYMMDD）と、ファイル名（HHMMSS）を分けて作る.
	std::string dateString = std::format("{:%Y%m%d}", localTime); // フォルダ用
	std::string timeString = std::format("{:%H%M%S}", localTime); // ファイル用

	// 日付別ディレクトリを用意.
	std::string logDir = std::format("logs/{}", dateString);
	std::filesystem::create_directories(logDir);

	// パスを作る.
	std::string logFilePath = std::format("{}/{}.log", logDir, timeString);
	logStream_.open(logFilePath);
}

DebugLog::~DebugLog() {
	if (logStream_.is_open()) {
		logStream_.close();
	}
}

std::string DebugLog::GetCurrentTimeString() const {
	// 現在時刻を取得.
	auto now = std::chrono::system_clock::now();
	// ミリ秒に丸める.
	auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	// ローカル時間に変換.
	std::chrono::zoned_time localTime{std::chrono::current_zone(), nowMs};
	return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
}

void DebugLog::Log(LogLevel level, const std::string& category, const std::string& message) {
	// キャッシュ用に要素を分解して保持しておく.
	LogEntry entry;
	entry.level = level;
	entry.timeString = GetCurrentTimeString();
	entry.category = category;
	entry.message = message;

	// ファイル / デバッグ出力用に1行へ整形.
	std::string fullMessage = std::format(
		"{} [{}] [{:<15}] {}",
		entry.timeString,
		LevelToString(level),
		category,
		message
	);
	logStream_ << fullMessage << std::endl;
	OutputDebugStringA((fullMessage + "\n").c_str());

	// ImGui表示用にキャッシュへ追加.
	logEntries_.push_back(std::move(entry));
}

// -----------------------------------------------------------------------
// GetHRESULTInfo  –  HRESULT を名前・説明の文字列ペアに変換する
// -----------------------------------------------------------------------
// 戻り値: { エラー名, 説明メッセージ }
// 未知のコードの場合は名前を "UNKNOWN_HRESULT(0x????????)" 形式で返す
// -----------------------------------------------------------------------
inline std::pair<std::string, std::string> GetHRESULTInfo(HRESULT hr) {
	struct HREntry {
		HRESULT code;
		const char* name;
		const char* message;
	};

	static const HREntry kTable[] =
		{
			// ====================================================================
	        // 汎用 (winerror.h)
	        // ====================================================================
			{S_OK, "S_OK", "操作に成功しました。"},
			{S_FALSE, "S_FALSE", "操作は成功しましたが、false が返されました。"},
			{E_ABORT, "E_ABORT", "操作が中止されました。"},
			{E_FAIL, "E_FAIL", "不特定のエラー。"},
			{E_HANDLE, "E_HANDLE", "ハンドルが無効です。"},
			{E_INVALIDARG, "E_INVALIDARG", "引数が無効です。"},
			{E_NOINTERFACE, "E_NOINTERFACE", "要求されたインターフェイスはサポートされていません。"},
			{E_NOTIMPL, "E_NOTIMPL", "要求された機能は実装されていません。"},
			{E_OUTOFMEMORY, "E_OUTOFMEMORY", "メモリの割り当てに失敗しました。"},
			{E_POINTER, "E_POINTER", "無効なポインタが渡されました。"},
			{E_UNEXPECTED, "E_UNEXPECTED", "予期しないエラーが発生しました。"},
			{E_ACCESSDENIED, "E_ACCESSDENIED", "アクセスが拒否されました。"},
			{E_PENDING, "E_PENDING", "データはまだ利用できません。"},
			{E_CHANGED_STATE, "E_CHANGED_STATE", "状態が変化したため操作を完了できませんでした。"},
			{E_ILLEGAL_STATE_CHANGE, "E_ILLEGAL_STATE_CHANGE", "無効な状態遷移が試みられました。"},
			{E_ILLEGAL_METHOD_CALL, "E_ILLEGAL_METHOD_CALL", "現在の状態ではこのメソッドを呼び出せません。"},
			{REGDB_E_CLASSNOTREG, "REGDB_E_CLASSNOTREG", "クラスがレジストリに登録されていません。"},
			{CLASS_E_NOAGGREGATION, "CLASS_E_NOAGGREGATION", "このクラスは集約をサポートしていません。"},
			{CO_E_NOTINITIALIZED, "CO_E_NOTINITIALIZED", "COM が初期化されていません。CoInitialize を先に呼び出してください。"},

			// ====================================================================
	        // DXGI (dxgi.h / dxgierr.h)
	        // ====================================================================
			{DXGI_ERROR_ACCESS_DENIED, "DXGI_ERROR_ACCESS_DENIED", "DXGI オブジェクトへのアクセスが拒否されました。"},
			{DXGI_ERROR_ACCESS_LOST, "DXGI_ERROR_ACCESS_LOST", "デスクトップ複製インターフェイスへのアクセスが失われました。"},
			{DXGI_ERROR_ALREADY_EXISTS, "DXGI_ERROR_ALREADY_EXISTS", "オブジェクトはすでに存在しています。"},
			{DXGI_ERROR_CANNOT_PROTECT_CONTENT, "DXGI_ERROR_CANNOT_PROTECT_CONTENT", "DXGI がコンテンツを保護できません。"},
			{DXGI_ERROR_DEVICE_HUNG, "DXGI_ERROR_DEVICE_HUNG", "GPU が無効なコマンドにより応答しなくなりました。"},
			{DXGI_ERROR_DEVICE_REMOVED, "DXGI_ERROR_DEVICE_REMOVED", "GPU デバイスが取り外されたか、ドライバのアップグレードが発生しました。"},
			{DXGI_ERROR_DEVICE_RESET, "DXGI_ERROR_DEVICE_RESET", "不正なコマンドのため GPU がリセットされました。"},
			{DXGI_ERROR_DRIVER_INTERNAL_ERROR, "DXGI_ERROR_DRIVER_INTERNAL_ERROR", "GPU ドライバの内部エラーが発生しました。"},
			{DXGI_ERROR_FRAME_STATISTICS_DISJOINT, "DXGI_ERROR_FRAME_STATISTICS_DISJOINT", "フレーム統計の計測が中断されました。"},
			{DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE", "ビデオプレゼントネットワークソースはすでに使用されています。"},
			{DXGI_ERROR_INVALID_CALL, "DXGI_ERROR_INVALID_CALL", "メソッドの呼び出しが無効です（引数等を確認してください）。"},
			{DXGI_ERROR_MORE_DATA, "DXGI_ERROR_MORE_DATA", "バッファが小さすぎます。"},
			{DXGI_ERROR_NAME_ALREADY_EXISTS, "DXGI_ERROR_NAME_ALREADY_EXISTS", "指定した名前はすでに存在しています。"},
			{DXGI_ERROR_NONEXCLUSIVE, "DXGI_ERROR_NONEXCLUSIVE", "グローバルカウンタのリソースが非排他的です。"},
			{DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE", "リソースまたは操作は現在利用できません。"},
			{DXGI_ERROR_NOT_FOUND, "DXGI_ERROR_NOT_FOUND", "要求されたオブジェクトが見つかりませんでした。"},
			{DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED", "リモートクライアントが切断されました。"},
			{DXGI_ERROR_REMOTE_OUTOFMEMORY, "DXGI_ERROR_REMOTE_OUTOFMEMORY", "リモートコンピュータのメモリが不足しています。"},
			{DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE, "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE", "出力制限が古くなっています。"},
			{DXGI_ERROR_SDK_COMPONENT_MISSING, "DXGI_ERROR_SDK_COMPONENT_MISSING", "必要な SDK コンポーネントが見つかりません。"},
			{DXGI_ERROR_SESSION_DISCONNECTED, "DXGI_ERROR_SESSION_DISCONNECTED", "リモートセッションが切断されたため操作を完了できませんでした。"},
			{DXGI_ERROR_UNSUPPORTED, "DXGI_ERROR_UNSUPPORTED", "この機能はハードウェアやドライバでサポートされていません。"},
			{DXGI_ERROR_WAIT_TIMEOUT, "DXGI_ERROR_WAIT_TIMEOUT", "待機がタイムアウトしました。"},
			{DXGI_ERROR_WAS_STILL_DRAWING, "DXGI_ERROR_WAS_STILL_DRAWING", "GPU がまだ描画中のため操作を完了できませんでした。"},
			{DXGI_STATUS_OCCLUDED, "DXGI_STATUS_OCCLUDED", "ウィンドウが別のウィンドウに覆われています。"},
			{DXGI_STATUS_CLIPPED, "DXGI_STATUS_CLIPPED", "プレゼントされたコンテンツがクリップされました。"},
			{DXGI_STATUS_NO_REDIRECTION, "DXGI_STATUS_NO_REDIRECTION", "リダイレクションなしでコンテンツが表示されました。"},
			{DXGI_STATUS_NO_DESKTOP_ACCESS, "DXGI_STATUS_NO_DESKTOP_ACCESS", "デスクトップへのアクセスがありません。"},
			{DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE, "DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE", "ビデオプレゼントネットワークソースが使用中です。"},
			{DXGI_STATUS_MODE_CHANGED, "DXGI_STATUS_MODE_CHANGED", "ディスプレイモードが変更されました。"},
			{DXGI_STATUS_MODE_CHANGE_IN_PROGRESS, "DXGI_STATUS_MODE_CHANGE_IN_PROGRESS", "ディスプレイモードの変更が進行中です。"},

			// ====================================================================
	        // Direct3D 11 (d3d11.h)
	        // ====================================================================
			{D3D11_ERROR_FILE_NOT_FOUND, "D3D11_ERROR_FILE_NOT_FOUND", "指定したファイルが見つかりませんでした。"},
			{D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS, "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS", "ユニークな状態オブジェクトが多すぎます。"},
			{D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS, "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS", "ユニークなビューオブジェクトが多すぎます。"},
			{D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD, "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD", "遅延コンテキストでの Map に DISCARD が指定されていません。"},

			// ====================================================================
	        // Direct3D 12 (d3d12.h)
	        // ====================================================================
			{D3D12_ERROR_ADAPTER_NOT_FOUND, "D3D12_ERROR_ADAPTER_NOT_FOUND", "指定したアダプタが見つかりませんでした。"},
			{D3D12_ERROR_DRIVER_VERSION_MISMATCH, "D3D12_ERROR_DRIVER_VERSION_MISMATCH", "ドライバのバージョンが一致しません。"},

	// ====================================================================
	// Direct3D 共通 (d3d9.h が必要)
	// ====================================================================
#ifdef D3DERR_INVALIDCALL
			{D3DERR_INVALIDCALL, "D3DERR_INVALIDCALL", "呼び出しが無効です（引数や状態を確認してください）。"},
#endif
#ifdef D3DERR_WASSTILLDRAWING
			{D3DERR_WASSTILLDRAWING, "D3DERR_WASSTILLDRAWING", "GPU がまだ描画処理中です。"},
#endif
		};

	for (const auto& entry : kTable) {
		if (entry.code == hr) {
			return {entry.name, entry.message};
		}
	}

	// 未知のコード: 16進数表記で返す
	std::ostringstream oss;
	oss << "UNKNOWN_HRESULT(0x"
		<< std::uppercase << std::setfill('0') << std::setw(8) << std::hex
		<< static_cast<unsigned long>(hr) << ")";
	return {oss.str(), "未知の HRESULT コードです。"};
}

void DebugLog::LogHRESULT(HRESULT hr) {
	auto hrInfo = GetHRESULTInfo(hr);
	LogLevel logLevel = LogLevel::Warn;
	if (SUCCEEDED(hr)) {
		if (hr == S_OK) {
			logLevel = LogLevel::Info;
		} else {
			logLevel = LogLevel::Warn;
		}
	} else {
		logLevel = LogLevel::Error;
	}

	Log(
		logLevel,
		"HRESULT",
		std::format("{:08X} {} {}\n", hr, hrInfo.first, hrInfo.second)
	);
}
void AssertHRESULT(HRESULT hr, const std::string& message) {
	if (SUCCEEDED(hr))
		return;
	auto hrInfo = GetHRESULTInfo(hr);
	DebugLog::GetInstance().Log(LogLevel::Error, "AssertHRESULT", std::format("Check {}", message));
	DebugLog::GetInstance().LogHRESULT(hr);
	assert(false);
}

} // namespace Cake
