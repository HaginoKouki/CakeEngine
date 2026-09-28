#pragma once
/*====================================
 *
 * Graphics 層（RHI）の部品をまとめて所有し、生涯を管理するクラス。
 * DirectX12 のデバイス・コマンド・ヒープ・スワップチェイン・シェーダを抱える。
 *
 * 【依存の向き】
 * この層より上（Asset / Render / Scene / Editor）を一切知らない。
 * 下は Platform と Foundation のみ。この規則が破れると層が意味を失う。
 *
 * DX12 の型が外へ漏れる唯一の場所でもある。将来 RHI として抽象化する際は、
 * ここの getter を差し替えることで上位層を守る。
 *
 * ====================================*/

#include "Engine/Graphics/Device/DirectXDevice.h"
#include "Engine/Graphics/Device/CommandManager.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/RenderTarget/DepthBuffer.h"
#include "Engine/Graphics/RenderTarget/SwapChain.h"
#include "Engine/Graphics/Pipeline/PipelineState.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"
#include "Engine/Graphics/Shader/ShaderLibrary.h"
#include "Engine/Graphics/Profiler/PerformanceProfiler.h"

namespace Cake {

class Graphics {
	DirectXDevice device_;
	CommandManager commandManager_;
	DescriptorHeap rtvHeap_, srvHeap_, dsvHeap_;
	SwapChain swapChain_;
	ShaderCompiler shaderCompiler_;
	PipelineState pipelineState_;
	ShaderLibrary shaderLibrary_;
	DepthBuffer depthBuffer_;
	PerformanceProfiler profiler_;

public:
	void Initialize(HWND hwnd, uint32_t width, uint32_t height);

	// Renderer::Resize も併せて呼ぶこと（ビューポート／シザーの更新はそちらの担当）
	void Resize(uint32_t width, uint32_t height);
	void ResizeDepthBuffer(uint32_t width, uint32_t height);

	DirectXDevice* GetDevice() { return &device_; }
	CommandManager* GetCommandManager() { return &commandManager_; }
	DescriptorHeap* GetRtvHeap() { return &rtvHeap_; }
	DescriptorHeap* GetSrvHeap() { return &srvHeap_; }
	DescriptorHeap* GetDsvHeap() { return &dsvHeap_; }
	SwapChain* GetSwapChain() { return &swapChain_; }
	ShaderCompiler* GetShaderCompiler() { return &shaderCompiler_; }
	PipelineState* GetPipelineState() { return &pipelineState_; }
	ShaderLibrary* GetShaderLibrary() { return &shaderLibrary_; }
	DepthBuffer* GetDepthBuffer() { return &depthBuffer_; }
	PerformanceProfiler* GetPerformanceProfiler() { return &profiler_; }
};
} // namespace Cake
