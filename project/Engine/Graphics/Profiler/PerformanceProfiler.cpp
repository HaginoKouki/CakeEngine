#include "PerformanceProfiler.h"

#include <vector>
#include <algorithm>
#include <cstdio>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include "Engine/Foundation/Debug/DebugLog.h"

#include "Engine/Graphics/Device/CommandManager.h"

// PDH（システム GPU 使用率）用のライブラリ.
#pragma comment(lib, "pdh.lib")

namespace Cake {

namespace {
// FILETIME を 64bit 整数へ変換する.
uint64_t FileTimeToU64(const FILETIME& ft) {
	ULARGE_INTEGER u;
	u.LowPart = ft.dwLowDateTime;
	u.HighPart = ft.dwHighDateTime;
	return u.QuadPart;
}
} // namespace

PerformanceProfiler::~PerformanceProfiler() {
	if (pdhQuery_ != nullptr) {
		PdhCloseQuery(pdhQuery_);
		pdhQuery_ = nullptr;
	}
}

void PerformanceProfiler::Initialize(ID3D12Device* device, CommandManager* commandManager) {
	DebugLog::GetInstance().Log(LogLevel::Info, "Profiler", "Initializing...");

	// ===== GPU タイムスタンプ用の QueryHeap（開始・終了の2点）=====
	D3D12_QUERY_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	heapDesc.Count = 2;

	HRESULT hr = device->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&queryHeap_));
	if (FAILED(hr)) {
		DebugLog::GetInstance().Log(LogLevel::Warn, "Profiler", "QueryHeap の生成に失敗。GPU 計測を無効化します。");
		gpuValid_ = false;
	} else {
		// 結果を CPU 側へ読み戻す Readback バッファ（uint64 ×2）.
		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_READBACK;

		D3D12_RESOURCE_DESC resDesc{};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Width = sizeof(uint64_t) * 2;
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc.Count = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		hr = device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_PPV_ARGS(&readback_)
		);
		if (FAILED(hr)) {
			DebugLog::GetInstance().Log(LogLevel::Warn, "Profiler", "Readback バッファの生成に失敗。GPU 計測を無効化します。");
			gpuValid_ = false;
		} else {
			// タイムスタンプ周波数（ticks/秒）を取得しておく（この値は不変）.
			hr = commandManager->GetCommandQueue()->GetTimestampFrequency(&gpuFrequency_);
			gpuValid_ = SUCCEEDED(hr) && gpuFrequency_ != 0;
			if (!gpuValid_) {
				DebugLog::GetInstance().Log(LogLevel::Warn, "Profiler", "タイムスタンプ周波数の取得に失敗。GPU 計測を無効化します。");
			}
		}
	}

	// ===== システム GPU%（PDH）=====
	InitializePdh();

	DebugLog::GetInstance().Log(LogLevel::Info, "Profiler", "Initialized\n");
}

void PerformanceProfiler::InitializePdh() {
	PDH_STATUS status = PdhOpenQueryW(nullptr, 0, &pdhQuery_);
	if (status != ERROR_SUCCESS) {
		pdhValid_ = false;
		return;
	}

	// 全 GPU エンジン（3D / Copy / Video など）の使用率をワイルドカードで取得する.
	// 言語環境に依存しないよう English 版のカウンタ名で追加する.
	status = PdhAddEnglishCounterW(
		pdhQuery_,
		L"\\GPU Engine(*)\\Utilization Percentage",
		0,
		&pdhGpuCounter_
	);
	if (status != ERROR_SUCCESS) {
		PdhCloseQuery(pdhQuery_);
		pdhQuery_ = nullptr;
		pdhValid_ = false;
		DebugLog::GetInstance().Log(LogLevel::Warn, "Profiler", "GPU Engine カウンタの追加に失敗。GPU 使用率を N/A にします。");
		return;
	}

	// 初回サンプル（差分計算のベースライン。この直後の値は無効なことが多い）.
	PdhCollectQueryData(pdhQuery_);
	pdhValid_ = true;
}

void PerformanceProfiler::BeginFrame(CommandManager* commandManager, Time& time) {
	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList();
	auto now = std::chrono::steady_clock::now();

	// フレーム周期（前回先頭 → 今回先頭）を計測する.
	frameTimeMs_ = time.GetUnscaledDeltaTime() * 1000.0f;

	// CPU 処理時間の起点.
	frameBeginTime_ = now;

	// GPU 開始タイムスタンプを積む（TIMESTAMP クエリは EndQuery で1点を打つ仕様）.
	if (gpuValid_) {
		commandList->EndQuery(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
		gpuQueryStarted_ = true; // 開始点を打てた → このフレームは Resolve してよい.
	}
}

void PerformanceProfiler::EndFrame(CommandManager* commandManager) {
	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList();
	// CPU 処理時間を確定する（コマンド積み終わり＝ExecuteAndWait 直前）.
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float, std::milli> cpu = now - frameBeginTime_;
	cpuTimeMs_ = cpu.count();

	// GPU 終了タイムスタンプ＋結果の書き出し命令を積む.
	if (gpuValid_) {
		commandList->EndQuery(queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
		commandList->ResolveQueryData(
			queryHeap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
			0, 2, readback_.Get(), 0
		);
	}
}

void PerformanceProfiler::Resolve() {
	// GPU 時間を読み戻す（ExecuteAndWait 済みなので、今フレーム分が確実に入っている）.
	if (gpuValid_) {
		uint64_t* mapped = nullptr;
		D3D12_RANGE readRange{0, sizeof(uint64_t) * 2};
		HRESULT hr = readback_->Map(0, &readRange, reinterpret_cast<void**>(&mapped));
		if (SUCCEEDED(hr) && mapped != nullptr) {
			uint64_t begin = mapped[0];
			uint64_t end = mapped[1];

			// CPU 側から書き込んでいないので、書き戻し範囲は空にする.
			D3D12_RANGE writeRange{0, 0};
			readback_->Unmap(0, &writeRange);

			if (end > begin && gpuFrequency_ != 0) {
				gpuTimeMs_ = static_cast<float>(
					static_cast<double>(end - begin) / static_cast<double>(gpuFrequency_) * 1000.0
				);
			}
		}
	}

	// 履歴（リングバッファ）へ記録する.
	frameHistory_[historyOffset_] = frameTimeMs_;
	cpuHistory_[historyOffset_] = cpuTimeMs_;
	gpuHistory_[historyOffset_] = gpuTimeMs_;
	historyOffset_ = (historyOffset_ + 1) % kHistoryCount;

	// システム使用率（1秒間隔で更新）.
	UpdateSystemUsage();
}

void PerformanceProfiler::UpdateSystemUsage() {
	auto now = std::chrono::steady_clock::now();

	// 前回更新から1秒経っていなければ何もしない.
	if (lastUsageUpdate_.time_since_epoch().count() != 0) {
		std::chrono::duration<float> elapsed = now - lastUsageUpdate_;
		if (elapsed.count() < 1.0f) {
			return;
		}
	}
	lastUsageUpdate_ = now;

	// ===== システム CPU 使用率 =====
	// GetSystemTimes は起動からの累積時間を返すので、前回との差分から使用率を出す.
	FILETIME idleFt, kernelFt, userFt;
	if (GetSystemTimes(&idleFt, &kernelFt, &userFt)) {
		uint64_t idle = FileTimeToU64(idleFt);
		uint64_t kernel = FileTimeToU64(kernelFt); // kernel には idle 時間が含まれる.
		uint64_t user = FileTimeToU64(userFt);

		if (hasPrevCpuTime_) {
			uint64_t idleDiff = idle - prevIdleTime_;
			uint64_t total = (kernel - prevKernelTime_) + (user - prevUserTime_);
			if (total > 0) {
				uint64_t busy = total - idleDiff;
				cpuUsagePercent_ = static_cast<float>(busy) / static_cast<float>(total) * 100.0f;
			}
		}
		prevIdleTime_ = idle;
		prevKernelTime_ = kernel;
		prevUserTime_ = user;
		hasPrevCpuTime_ = true;
	}

	// ===== システム GPU 使用率（PDH：全エンジンの最大値）=====
	// タスクマネージャの「GPU 使用率」に近づけるため、各エンジンの最大値を採用する
	// （全エンジンを合算すると 100% を超えてしまうため）.
	if (pdhValid_ && PdhCollectQueryData(pdhQuery_) == ERROR_SUCCESS) {
		DWORD bufferSize = 0;
		DWORD itemCount = 0;

		// 1回目：必要なバッファサイズを問い合わせる.
		PDH_STATUS st = PdhGetFormattedCounterArrayW(
			pdhGpuCounter_, PDH_FMT_DOUBLE, &bufferSize, &itemCount, nullptr
		);
		if (st == PDH_MORE_DATA && bufferSize > 0) {
			std::vector<BYTE> buffer(bufferSize);
			auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());

			// 2回目：実データを取得する.
			st = PdhGetFormattedCounterArrayW(
				pdhGpuCounter_, PDH_FMT_DOUBLE, &bufferSize, &itemCount, items
			);
			if (st == ERROR_SUCCESS) {
				double maxUsage = 0.0;
				for (DWORD i = 0; i < itemCount; ++i) {
					if (items[i].FmtValue.CStatus == ERROR_SUCCESS) {
						maxUsage = (std::max)(maxUsage, items[i].FmtValue.doubleValue);
					}
				}
				gpuUsagePercent_ = static_cast<float>(maxUsage);
			}
		}
	}
}

#ifdef USE_IMGUI

namespace {
// 負荷 t（0=軽い, 1=重い）に応じて 緑→黄→赤 のグラデーション色を返す.
ImVec4 LoadColor(float t) {
	t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
	float r = (t < 0.5f) ? (t * 2.0f) : 1.0f;
	float g = (t < 0.5f) ? 1.0f : (1.0f - (t - 0.5f) * 2.0f);
	return ImVec4(r, g, 0.15f, 1.0f);
}

// 「ラベル ＋ 数値テキスト ＋ 色付きバー」を1行で描く.
// value/max が 1.0 でバー満タン＆赤に近づく.
void DrawColoredBar(const char* text, float value, float max) {
	ImGui::TextUnformatted(text);
	ImGui::SameLine(190.0f);
	float ratio = (max > 0.0f) ? (value / max) : 0.0f;
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, LoadColor(ratio));
	ImGui::ProgressBar(ratio, ImVec2(-1.0f, 0.0f), "");
	ImGui::PopStyleColor();
}
} // namespace

void PerformanceProfiler::DrawHud() {
	ImGui::Begin("Performance");

	// ---- FPS を大きく表示 ----
	ImGui::SetWindowFontScale(2.0f);
	ImGui::Text("%.1f FPS", GetFps());
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SameLine();
	ImGui::Text("(%.2f ms / frame)", frameTimeMs_);

	ImGui::Separator();

	// ---- 処理時間(ms) ----
	// 60FPS の予算 16.6ms を基準（=1.0）にしてバーを塗る.
	const float budget = 16.6f;
	ImGui::Text("Frame Time");

	char buf[64];
	std::snprintf(buf, sizeof(buf), "CPU  %6.3f ms", cpuTimeMs_);
	DrawColoredBar(buf, cpuTimeMs_, budget);

	if (gpuValid_) {
		std::snprintf(buf, sizeof(buf), "GPU  %6.3f ms", gpuTimeMs_);
		DrawColoredBar(buf, gpuTimeMs_, budget);
	} else {
		ImGui::TextDisabled("GPU  (計測不可)");
	}

	ImGui::Separator();

	// ---- 履歴グラフ ----
	// リングバッファを「古い→新しい」の並びに直してから渡す.
	auto plotRing = [&](const char* label, const std::array<float, kHistoryCount>& ring, float scaleMax) {
		std::array<float, kHistoryCount> ordered{};
		for (int i = 0; i < kHistoryCount; ++i) {
			ordered[i] = ring[(historyOffset_ + i) % kHistoryCount];
		}
		ImGui::PlotLines(label, ordered.data(), kHistoryCount, 0, nullptr, 0.0f, scaleMax, ImVec2(0.0f, 60.0f));
	};
	plotRing("Frame(ms)", frameHistory_, 33.3f);
	plotRing("CPU(ms)", cpuHistory_, 33.3f);
	if (gpuValid_) {
		plotRing("GPU(ms)", gpuHistory_, 33.3f);
	}

	ImGui::Separator();

	// ---- システム使用率(%) ----
	ImGui::Text("System Usage");

	std::snprintf(buf, sizeof(buf), "CPU  %5.1f %%", cpuUsagePercent_);
	DrawColoredBar(buf, cpuUsagePercent_, 100.0f);

	if (gpuUsagePercent_ >= 0.0f) {
		std::snprintf(buf, sizeof(buf), "GPU  %5.1f %%", gpuUsagePercent_);
		DrawColoredBar(buf, gpuUsagePercent_, 100.0f);
	} else {
		ImGui::TextDisabled("GPU  N/A (PDH カウンタ取得不可)");
	}

	ImGui::End();
}

#endif // USE_IMGUI

} // namespace Cake
