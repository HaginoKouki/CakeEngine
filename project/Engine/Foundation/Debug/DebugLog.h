#pragma once
/*====================================
 *
 * ログ出力とHRESULT（DirectX/COM関連のエラーコード）の解析・表示機能を提供するユーティリティ。
 * 日時別ディレクトリにログファイルを自動生成し、
 * エラーレベル（Info/Warn/Error）ごとに色分けして出力デバッグ情報を表示する。
 *
 * 【スレッド】
 * Log / LogInitStart / LogInitComplete / LogHRESULT / AssertHRESULT / SetCurrentThreadName は
 * どのスレッドから呼んでもよい。
 * GetLogEntry と PumpMainThread はメインスレッド専用（Debug ビルドでは assert で検査する）。
 *
 *   ファイル・デバッグ出力 … Log の中ですぐ書く（mutex で1行ずつ守る）。
 *                            落ちる直前のログも残るよう、1行ごとに flush する。
 *   画面表示用の一覧       … Log は保留キューに積むだけ。メインスレッドが毎フレーム
 *                            PumpMainThread で GetLogEntry の一覧へ移す。
 *
 * 画面表示用の一覧をメインスレッドだけが触るようにしているのは、LogWindow が
 * 一覧を参照で受け取って描画している間に、ワーカーの push_back で配列が再確保される
 * （参照が無効になる）のを防ぐため。
 *
 * 【メインスレッドの判定】
 * 最初に GetInstance を呼んだスレッドをメインスレッドとみなす。
 * Application::Initialize の先頭でログを出しているので、ワーカーを起動する前に必ず決まる。
 *
 * 【終了時】
 * このオブジェクトは関数内 static なので、main を抜けた後に破棄される。
 * それまでにワーカースレッドをすべて止めておくこと（JobSystem の終了処理で保証する）。
 *
 * ====================================*/
#include <Windows.h>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace Cake {

enum class LogLevel {
	Info,
	Warn,
	Error
};
struct LogEntry {
	LogLevel level;         // ログの重要度.
	std::string timeString; // ログが出力された時間.
	std::string threadName; // Log を呼んだスレッド（"Main"、"Worker03" など）.
	std::string category;   // ログを出力した機能（"Editor", "SceneManager"など）.
	std::string message;    // ログの内容.
};

class DebugLog {
private:
	/* 複数のスレッドが触るもの。必ず mutex_ を取ってから触る */
	// logStream_ と pending_ を、同時に1つのスレッドしか触れないようにする鍵.
	std::mutex mutex_;
	// .log ファイルへの書き込み口.
	std::ofstream logStream_;
	// 受付箱。どのスレッドからでも Log() で積まれ、メインスレッドが毎フレーム取り出して空にする.
	std::vector<LogEntry> pending_;

	/* メインスレッドだけが触るもの。鍵は要らない */
	// 画面（LogWindow）に映す用の一覧。受付箱から移したログを、出た順に全部持つ.
	std::vector<LogEntry> logEntries_;
	// PumpMainThread で pending_ と中身を入れ替える空の箱（毎回作り直さずに使い回す）.
	std::vector<LogEntry> pumpBuffer_;

	// メインスレッドの識別番号.
	std::thread::id mainThreadId_;

private:
	DebugLog();
	~DebugLog();

	std::string GetCurrentTimeString() const;
	std::string GetCurrentThreadName() const;
	bool IsMainThread() const { return std::this_thread::get_id() == mainThreadId_; }

public:
	DebugLog(const DebugLog&) = delete;
	DebugLog& operator=(const DebugLog&) = delete;

	static DebugLog& GetInstance() {
		static DebugLog instance;
		return instance;
	}

	// ログを書き出す.
	void Log(LogLevel level, const std::string& category, const std::string& message);

	// HRESULTをログに流す.
	void LogHRESULT(HRESULT hr);
	// 初期化開始のログを流す.
	void LogInitStart(const std::string& category) { Log(LogLevel::Info, category, "Initialize Start"); }
	// 初期化終了のログを流す.
	void LogInitComplete(const std::string& category) { Log(LogLevel::Info, category, "Initialize Complete"); }

	// 呼んだスレッドの、ログに出す名前を決める。ワーカースレッドの起動直後に呼ぶ.
	// 呼ばなかったスレッドは、メインスレッドなら "Main"、それ以外は "T<スレッドID>" になる.
	static void SetCurrentThreadName(std::string_view name);

	// メインスレッド専用.
	// 保留キューに溜まったログを画面表示用の一覧へ移す。毎フレーム1回呼ぶ.
	void PumpMainThread();

	// メインスレッド専用.
	// LogWindow に表示する用のログ一覧を取得する.
	const std::vector<LogEntry>& GetLogEntry() const;
};

void AssertHRESULT(HRESULT hr, const std::string& message);
inline const char* LevelToString(LogLevel level) {
	switch (level) {
		case LogLevel::Info:
			return "INFO "; // 5文字で揃える.
		case LogLevel::Warn:
			return "WARN ";
		case LogLevel::Error:
			return "ERROR";
	}
	return "?????";
}

} // namespace Cake
