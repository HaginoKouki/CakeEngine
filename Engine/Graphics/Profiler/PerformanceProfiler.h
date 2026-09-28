#pragma once
/*====================================
 *
 * エンジンのパフォーマンス計測・可視化を行うプロファイラ。
 * フレームタイム、CPU/GPU使用率、メモリ使用量等の統計情報を取得し、
 * ImGui上にグラフやテキストで表示してボトルネック分析を支援する。
 *
 * ====================================*/
#include <cstdint>
#include <array>
#include <chrono>

// pdh.h は Windows.h に依存するので、先に Windows.h を include すること.
// PDH_MORE_DATA などのエラーコードは pdhmsg.h 側に定義されている.
#include <Windows.h>
#include <pdh.h>
#include <pdhmsg.h>

#include <d3d12.h>
#include <wrl.h>

#include "Engine/Platform/Time/Time.h"

namespace Cake {

class CommandManager;

// CPU/GPU のフレーム処理時間(ms)と、システム全体の CPU/GPU 使用率(%)を計測し、
// ImGui で発表向けに可視化するプロファイラ.
//
// このエンジンは Renderer::EndFrame で毎フレーム ExecuteAndWait して GPU を待ち切る
// 同期設計なので、GPU タイムスタンプの結果を「同じフレーム内で」確実に読み戻せる.
// そのため多重バッファリングが不要で、実装がそのぶんシンプルになっている.
class PerformanceProfiler {
public:
	// 履歴として保持するフレーム数（折れ線グラフの横幅）.
	static constexpr int kHistoryCount = 120;

private:
	// ===== GPU タイムスタンプ =====
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> queryHeap_; // 開始・終了の2点.
	Microsoft::WRL::ComPtr<ID3D12Resource> readback_;   // 結果読み戻し用（uint64 ×2）.
	uint64_t gpuFrequency_ = 0;                         // タイムスタンプ周波数（ticks/秒）.
	bool gpuValid_ = false;                             // GPU 計測が使えるか.
	bool gpuQueryStarted_ = false;                      // 今フレームで開始タイムスタンプ(スロット0)を打てたか.
	bool gpuResolvePending_ = false;                    // Resolve 命令を積んだので、次の Resolve() で読み戻すべきか.

	// ===== CPU 時間計測 =====
	std::chrono::steady_clock::time_point frameBeginTime_{}; // 今フレーム先頭.

	// ===== 計測結果（ms） =====
	float frameTimeMs_ = 0.0f; // ループ1周（= 1000 / FPS）.
	float cpuTimeMs_ = 0.0f;   // CPU のコマンド積み時間.
	float gpuTimeMs_ = 0.0f;   // GPU 実行時間（タイムスタンプ実測）.

	// ===== システム使用率(%) =====
	float cpuUsagePercent_ = 0.0f;
	float gpuUsagePercent_ = -1.0f; // -1 は取得不可(N/A).

	// CPU% 計算用の前回スナップショット.
	uint64_t prevIdleTime_ = 0;
	uint64_t prevKernelTime_ = 0;
	uint64_t prevUserTime_ = 0;
	bool hasPrevCpuTime_ = false;

	// PDH（システム GPU%）.
	PDH_HQUERY pdhQuery_ = nullptr;
	PDH_HCOUNTER pdhGpuCounter_ = nullptr;
	bool pdhValid_ = false;

	// システム使用率は1秒ごとに更新する（毎フレームは重い＆意味が薄い）.
	std::chrono::steady_clock::time_point lastUsageUpdate_{};

	// ===== 履歴（リングバッファ） =====
	std::array<float, kHistoryCount> frameHistory_{};
	std::array<float, kHistoryCount> cpuHistory_{};
	std::array<float, kHistoryCount> gpuHistory_{};
	int historyOffset_ = 0;

public:
	PerformanceProfiler() = default;
	~PerformanceProfiler();

	PerformanceProfiler(const PerformanceProfiler&) = delete;
	PerformanceProfiler& operator=(const PerformanceProfiler&) = delete;

	// GPU クエリ用リソースと PDH を初期化する.
	void Initialize(ID3D12Device* device, CommandManager* commandManager);

	// フレーム先頭で呼ぶ（renderer.BeginFrame の後）.
	// GPU 開始タイムスタンプを積み、CPU タイマーを開始する.
	void BeginFrame(CommandManager* commandManager, Time& time);

	// フレーム末尾で呼ぶ（ExecuteAndWait の前）.
	// GPU 終了タイムスタンプ＋結果書き出し命令を積み、CPU 時間を確定する.
	void EndFrame(CommandManager* commandManager);

	// ExecuteAndWait の後で呼ぶ.
	// GPU 時間を読み戻し、履歴とシステム使用率を更新する.
	void Resolve();

#ifdef USE_IMGUI
	// 計測結果を ImGui ウィンドウに描画する.
	void DrawHud();
#endif

	// --- getter（数値を他所で使いたい場合）---
	float GetFrameTimeMs() const { return frameTimeMs_; }
	float GetCpuTimeMs() const { return cpuTimeMs_; }
	float GetGpuTimeMs() const { return gpuTimeMs_; }
	float GetCpuUsagePercent() const { return cpuUsagePercent_; }
	float GetGpuUsagePercent() const { return gpuUsagePercent_; }
	float GetFps() const { return frameTimeMs_ > 0.0f ? 1000.0f / frameTimeMs_ : 0.0f; }

private:
	void InitializePdh();     // PDH（システム GPU%）の初期化.
	void UpdateSystemUsage(); // CPU%/GPU% を更新（1秒間隔で間引く）.
};

} // namespace Cake
