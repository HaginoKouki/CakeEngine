#include "Graphics.h"

namespace Cake {

void Graphics::Initialize(HWND hwnd, uint32_t width, uint32_t height) {
	// 1. Device.
	device_.Initialize();

	// 2. CommandManager.
	commandManager_.Initialize(device_.GetD3DDevice());

	// 3. DescriptorHeap ×3.
	rtvHeap_.Initialize(device_.GetD3DDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4, false);
	srvHeap_.Initialize(device_.GetD3DDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvHeap_.Initialize(device_.GetD3DDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// 4. SwapChain.
	swapChain_.Initialize(
		device_.GetFactory(),
		device_.GetD3DDevice(),
		commandManager_.GetCommandQueue(),
		hwnd,
		&rtvHeap_,
		width, height
	);

	// 5. ShaderCompiler.
	shaderCompiler_.Initialize();

	// 6. PipelineState（Standard PSO を固定登録）.
	pipelineState_.Initialize(device_.GetD3DDevice());

	// 6.5 ShaderLibrary（シェーダー定義を登録し、PSO生成まで行う）.
	shaderLibrary_.Initialize(&pipelineState_, &shaderCompiler_);

	// 7. DepthBuffer.
	depthBuffer_.Initialize(device_.GetD3DDevice(), &dsvHeap_, width, height);

	profiler_.Initialize(device_.GetD3DDevice(), &commandManager_);
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
