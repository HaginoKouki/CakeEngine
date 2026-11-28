#include "GraphicsDebugWindow.h"

#ifdef ENABLE_EDITOR
#include <format>
#include <imgui.h>
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Platform/Job/JobSystem.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Upload/UploadBudget.h"

namespace Cake {
namespace {
// ワーカーで作るテクスチャの一辺の画素数.
constexpr uint32_t kWorkerTextureSizes[] = {256, 1024, 2048, 4096};
constexpr const char* kWorkerTextureSizeLabels[] = {"256", "1024", "2048", "4096"};
} // namespace

GraphicsDebugWindow::~GraphicsDebugWindow() {
	ReleaseTestTexture(workerTexture_);
	ReleaseTestTexture(stressTexture_);
}
void GraphicsDebugWindow::Initialize(Graphics* graphics, JobSystem* jobSystem) {
	graphics_ = graphics;
	jobSystem_ = jobSystem;
}

void GraphicsDebugWindow::Draw(const char* title) {
	// たたまれているときも Begin と End は1回ずつ呼ぶ.
	if (!ImGui::Begin(title)) {
		ImGui::End();
		return;
	}

	ImGui::SeparatorText("Frame");
	float frameTimeMs = graphics_->GetPerformanceProfiler()->GetFrameTimeMs();
	maxFrameTimeMs_ = frameTimeMs > maxFrameTimeMs_ ? frameTimeMs : maxFrameTimeMs_;
	ImGui::Text("%s", std::format("フレーム時間 {:.2f}ms / 最大フレーム時間 {:.2f}ms", frameTimeMs, maxFrameTimeMs_).c_str());
	if (ImGui::Button("最大値をリセット")) {
		maxFrameTimeMs_ = 0.0f;
	}

	// SRV の使用数.
	ImGui::SeparatorText("SRV");
	uint32_t usedCount = graphics_->GetSrvManager()->GetUsedCount();
	uint32_t capacity = graphics_->GetSrvManager()->GetCapacity();
	uint32_t peakUsedCount = graphics_->GetSrvManager()->GetPeakUsedCount();
	ImGui::Text("%s", std::format("使用 {} / {}", usedCount, capacity).c_str());
	ImGui::Text("%s", std::format("最大使用数 {}", peakUsedCount).c_str());
	ImGui::ProgressBar(static_cast<float>(usedCount) / static_cast<float>(capacity), ImVec2(-1.0f, 0.0f));

	// GPU への転送.
	ImGui::SeparatorText("Upload");
	ImGui::Text("%s", std::format("直前のフレームの転送量 {} MB", graphics_->GetUploadQueue()->GetBytesLastFrame() / (1024.0 * 1024.0)).c_str());
	ImGui::Text("%s", std::format("上限 {} MB", kUploadBytesPerFrame / (1024.0 * 1024.0)).c_str());
	ImGui::Text("%s", std::format("予約の数 {}", graphics_->GetUploadQueue()->GetPendingCount()).c_str());

	// 解放待ち.
	ImGui::SeparatorText("Deferred Release");
	ImGui::Text("%s", std::format("未解放予約の数 {}", graphics_->GetDeferredReleaseQueue()->GetPendingCount()).c_str());

	// ジョブ.
	ImGui::SeparatorText("Jobs");
	ImGui::Text("%s", std::format("未実行の処理 {}", jobSystem_->GetQueuedCount()).c_str());
	ImGui::Text("%s", std::format("完了処理待ち {}", jobSystem_->GetPendingCompletionCount()).c_str());

	// テスト.
	DrawWorkerTextureTest();
	DrawStressTest();

	ImGui::End();
}

GraphicsDebugWindow::TestTexture GraphicsDebugWindow::CreateTestTexture(uint32_t width, uint32_t height, Pixels pixels, std::function<void()> onComplete) {
	TestTexture texture;
	texture.width = width;
	texture.height = height;

	texture.srvIndex = graphics_->GetSrvManager()->Allocate();
	if (texture.srvIndex == SrvManager::kInvalidIndex) {
		return texture;
	}

	// テクスチャのリソースを作る.
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);
	resourceDesc.Height = UINT(height);
	resourceDesc.MipLevels = UINT16(1);
	resourceDesc.DepthOrArraySize = UINT16(1);
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = graphics_->GetDevice()->GetD3D12Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture.resource)
	);
	AssertHRESULT(hr, "TextureResourceの生成");

	// ▼ SRV を書く（Format は同じ、MipLevels = 1、ViewDimension = TEXTURE2D、Shader4ComponentMapping は既定）.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resourceDesc.Format; // 例: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

	// 3D描画用のSRV.
	graphics_->GetDevice()->GetD3D12Device()->CreateShaderResourceView(texture.resource.Get(), &srvDesc, graphics_->GetSrvManager()->GetCPUHandle(texture.srvIndex));

	// 転送するデータの場所を作る.
	D3D12_SUBRESOURCE_DATA subresource;
	subresource.pData = pixels->data();
	subresource.RowPitch = width * 4;
	subresource.SlicePitch = subresource.RowPitch * height;

	// ▼ UploadRequest を作って、graphics_->GetUploadQueue()->Enqueue する.
	//    owner には pixels を入れる（Record で写すまで、画素が生きているように）.
	//    onComplete には、引数の onComplete を std::move して入れる.
	UploadRequest request;
	request.destination = texture.resource;
	request.subresources = std::move(std::vector<D3D12_SUBRESOURCE_DATA>{subresource});
	request.owner = pixels;
	request.stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	request.onComplete = std::move(onComplete);
	graphics_->GetUploadQueue()->Enqueue(std::move(request));

	// ▼ texture を返す.
	return texture;
}

void GraphicsDebugWindow::ReleaseTestTexture(TestTexture& texture) {
	// srvIndex が kInvalidIndex なら（作っていない、または作るのに失敗した）、何もしない.
	if (texture.srvIndex == SrvManager::kInvalidIndex) {
		return;
	}

	graphics_->GetDeferredReleaseQueue()->Enqueue([resource = std::move(texture.resource), srvManager = graphics_->GetSrvManager(), srvIndex = texture.srvIndex]() {
		srvManager->Free(srvIndex);
	});

	// texture を空に戻す.
	texture = {};
}

void GraphicsDebugWindow::DrawWorkerTextureTest() {
	ImGui::SeparatorText("Worker Texture Test");

	// ワーカーで作っている最中は押せなくする（作業を積みすぎないため）.
	ImGui::BeginDisabled(workerJobRunning_);
	ImGui::Combo("大きさ", &workerTextureSizeIndex_, kWorkerTextureSizeLabels, 4);
	const bool pressed = ImGui::Button("Create Texture", ImVec2{-1.0f, 0.0f});
	const uint32_t size = kWorkerTextureSizes[workerTextureSizeIndex_];
	ImGui::EndDisabled();

	if (pressed) {
		workerJobRunning_ = true;

		// 押すたびに青の強さを変えて、作り直されたことが目で見てわかるようにする.
		const uint32_t seed = workerTextureSerial_ + 1;

		jobSystem_->Submit<Pixels>(
			JobPriority::Normal,

			// 作業（ワーカーで実行）：画素を作るだけ。D3D12 と ImGui には触らない.
			[seed, size]() -> Pixels {
				auto pixels = std::make_shared<std::vector<uint8_t>>(size * size * 4);
				for (uint32_t y = 0; y < size; ++y) {
					for (uint32_t x = 0; x < size; ++x) {
						uint8_t* p = pixels->data() + (y * size + x) * 4;
						p[0] = static_cast<uint8_t>(x * 255 / (size - 1)); // R：左から右へ明るく.
						p[1] = static_cast<uint8_t>(y * 255 / (size - 1)); // G：上から下へ明るく.
						p[2] = static_cast<uint8_t>((seed * 64) % 256);     // B：押すたびに変わる.
						p[3] = 255;                                         // A.
					}
				}
				return pixels;
			},

			// 完了処理（メインスレッドで実行）：ここで初めて GPU の部品に触る.
			[this, size](JobResult<Pixels> result) {
				workerJobRunning_ = false;
				if (!result.Succeeded()) {
					return;
				}

				// 前のテクスチャを捨てる。GPU が使い終わってから解放される.
				ReleaseTestTexture(workerTexture_);
				workerTextureReady_ = false;

				// 何枚目かを覚えておき、転送が終わったときに、まだ最新のテクスチャかを確かめる.
				const uint32_t serial = ++workerTextureSerial_;
				workerTexture_ = CreateTestTexture(
					size, size, std::move(*result.value),
					[this, serial]() {
						if (serial == workerTextureSerial_) {
							workerTextureReady_ = true;
						}
					}
				);
			}
		);
	}

	// 転送が終わっていれば表示する.
	if (workerTextureReady_) {
		const D3D12_GPU_DESCRIPTOR_HANDLE handle = graphics_->GetSrvManager()->GetGPUHandle(workerTexture_.srvIndex);
		ImGui::Image(
			(ImTextureID)handle.ptr,
			ImVec2(256.0f, 256.0f)
		);
	} else if (workerJobRunning_ || workerTexture_.srvIndex != SrvManager::kInvalidIndex) {
		ImGui::TextUnformatted("作成中・転送中...");
	}
}

void GraphicsDebugWindow::DrawStressTest() {
	ImGui::SeparatorText("Stress Test");

	// ▼ ボタン「作成と破棄を 3000 回」。押されたら stressRemaining_ = 3000.
	ImGui::BeginDisabled(stressRemaining_ > 0);
	const bool pressed = ImGui::Button("作成と破棄を 3000 回");
	if (pressed) {
		stressRemaining_ = 3000;
	}
	ImGui::EndDisabled();

	// ▼ stressRemaining_ が 0 より大きいあいだ、毎フレーム次のことをする.
	//    ・前のフレームの stressTexture_ を ReleaseTestTexture で捨てる.
	//    ・4 x 4 の小さな画素を、メインスレッドで作る（make_shared で std::vector<uint8_t> を作る。中身は何色でもよい）.
	//    ・CreateTestTexture で stressTexture_ を作る（onComplete は要らないので、空の std::function を渡す）.
	//    ・stressRemaining_ を 1 減らす.
	//    ・残りが 0 になったら、最後の stressTexture_ も ReleaseTestTexture で捨てる.
	if (stressRemaining_ > 0) {
		ReleaseTestTexture(stressTexture_);
		constexpr uint32_t kSize = 4;
		auto pixels = std::make_shared<std::vector<uint8_t>>(kSize * kSize * 4);

		stressTexture_ = CreateTestTexture(kSize, kSize, pixels, {});

		stressRemaining_--;

		if (stressRemaining_ == 0) {
			ReleaseTestTexture(stressTexture_);
		}
	}

	// ▼ 残りの回数を表示する.
	ImGui::Text("%s", std::format("残り {} 回", stressRemaining_).c_str());
}

} // namespace Cake
#endif
