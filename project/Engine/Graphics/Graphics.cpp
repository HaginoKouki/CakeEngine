#include "Graphics.h"

namespace Cake {

Graphics::~Graphics() {
	// Initialize が呼ばれている場合のみ、GPU を待って解放する.
	if (device_.GetD3D12Device() != nullptr) {
		commandManager_.WaitForGPU();
		deferredReleaseQueue_.ReleaseAll();
	}
}

void Graphics::Initialize(HWND hwnd, uint32_t width, uint32_t height) {
	// 1. Device.
	device_.Initialize();

	// 2. CommandManager.
	commandManager_.Initialize(device_.GetD3D12Device());

	// 3. DescriptorHeap ×3.
	rtvHeap_.Initialize(device_.GetD3D12Device(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4, false);
	srvManager_.Initialize(device_.GetD3D12Device());
	dsvHeap_.Initialize(device_.GetD3D12Device(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// UploadQueue.
	uploadQueue_.Initialize(device_.GetD3D12Device(), &deferredReleaseQueue_);

	// 4. SwapChain.
	swapChain_.Initialize(
		device_.GetFactory(),
		device_.GetD3D12Device(),
		commandManager_.GetCommandQueue(),
		hwnd,
		&rtvHeap_,
		width, height
	);

	// 5. ShaderCompiler.
	shaderCompiler_.Initialize();

	// 6. PipelineState（Standard PSO を固定登録）.
	pipelineState_.Initialize(device_.GetD3D12Device());

	// 6.5 ShaderLibrary（シェーダー定義を登録し、PSO生成まで行う）.
	shaderLibrary_.Initialize(&pipelineState_, &shaderCompiler_);

	// 7. DepthBuffer.
	depthBuffer_.Initialize(device_.GetD3D12Device(), &dsvHeap_, width, height);

	profiler_.Initialize(device_.GetD3D12Device(), &commandManager_);
}

void Graphics::BeginFrame() {
	// GPU が使い終わったリソースを解放する.
	deferredReleaseQueue_.ReleaseCompleted(commandManager_.GetCompletedFenceValue());
	// 完了処理を呼ぶ.
	uploadQueue_.ProcessCompleted(commandManager_.GetCompletedFenceValue());
	// 予約されていたコマンドを積む.
	uploadQueue_.Record(commandManager_.GetCommandList());
}

void Graphics::EndFrame() {
	// このフレームの解放の予約を締める.
	deferredReleaseQueue_.CloseFrame(commandManager_.GetLastSignaledFenceValue());
	// 完了処理に番号を付ける.
	uploadQueue_.CloseFrame(commandManager_.GetLastSignaledFenceValue());
}

void Graphics::Resize(uint32_t width, uint32_t height) {
	commandManager_.WaitForGPU();
	swapChain_.Resize(width, height);
}
void Graphics::ResizeDepthBuffer(uint32_t width, uint32_t height) {
	commandManager_.WaitForGPU();
	depthBuffer_.Resize(width, height);
}

} // namespace Cake
