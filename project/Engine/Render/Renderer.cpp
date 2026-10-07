#include "Renderer.h"

#include <vector>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Math/Transform.h"

#include "Engine/Graphics/Device/CommandManager.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/pipeline/PipelineState.h"
#include "Engine/Graphics/RenderTarget/SwapChain.h"
#include "Engine/Graphics/RenderTarget/DepthBuffer.h"
#include "Engine/Graphics/RenderTarget/SceneRenderTarget.h"
#include "Engine/Graphics/Shader/ShaderDefinition.h"

#include "Engine/Render/Gizmo/GizmoDrawList.h"

#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Asset/Sprite/Sprite.h"
#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialManager.h"
#include "Engine/Asset/Model/Mesh.h"
#include "Engine/Asset/Model/ModelManager.h"


namespace Cake {
namespace {
constexpr const char* kLogCategory = "Renderer";
}

// Particle.VS.hlsl の ParticleForGPU と同じ並びにすること.
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

void Renderer::Initialize(RendererInitDesc desc) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = desc.device;
	swapChain_ = desc.swapChain;
	depthBuffer_ = desc.depthBuffer;
	srvHeap_ = desc.srvHeap;
	pipelineState_ = desc.pipelineState;
	materialManager_ = desc.materialManager;
	modelManager_ = desc.modelManager;

	// ライトマネージャ.
	lightManager_.Initialize(device_);

	// フレーム頂点.
	frameVertices_.Initialize(device_, 4 * 1024 * 1024);
	// フレーム定数バッファ（WVP 置き場）. 256B/モデル換算で 1MB ≒ 4096 モデル分.
	frameConstants_.Initialize(device_, 1 * 1024 * 1024);
	// ビューポート.
	viewport_.Width = static_cast<float>(desc.clientWidth);
	viewport_.Height = static_cast<float>(desc.clientHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	// シザー矩形.
	scissorRect_.left = 0;
	scissorRect_.right = desc.clientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = desc.clientHeight;

	clientWidth_ = desc.clientWidth;
	clientHeight_ = desc.clientHeight;
	drawWidth_ = static_cast<float>(clientWidth_);
	drawHeight_ = static_cast<float>(clientHeight_);

	skyboxMaterialHandle_ = materialManager_->CreateMaterial("__Skybox", "Skybox/Panoramic");
	{
		Material& mat = *materialManager_->Resolve(skyboxMaterialHandle_);
		mat.SetTexture(mat.GetShader()->textures[0].name, desc.textureManager->Load("EngineResources/textures/Skybox.png"));
		mat.Apply();
	}

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void Renderer::BeginFrame(CommandManager* commandManager) {
	frameConstants_.Reset();
	frameVertices_.Reset();
	commandList_ = commandManager->GetCommandList();
	swapChain_->BeginFrame(commandList_); // PRESENT → RENDER_TARGET.
}

void Renderer::EndFrame(CommandManager* commandManager) {
	swapChain_->EndFrame(commandManager->GetCommandList());
	commandManager->ExecuteAndWait();
	swapChain_->Present();
	commandManager->ResetForNextFrame();
}

void Renderer::BeginScene(CommandManager* commandManager, SceneRenderTarget* sceneRT) {
	commandList_ = commandManager->GetCommandList();

	// オフスクリーンRTを描画先へ遷移.
	sceneRT->TransitionToRenderTarget(commandList_);

	auto rtvHandle = sceneRT->GetRTVHandle();
	auto dsvHandle = depthBuffer_->GetDSVHandle(); // 深度は共有バッファ（サイズはシーンRTに合わせる）.

	commandList_->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	commandList_->ClearRenderTargetView(rtvHandle, sceneRT->GetClearColor(), 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// DescriptorHeap.
	ID3D12DescriptorHeap* heaps[] = {srvHeap_->GetHeap()};
	commandList_->SetDescriptorHeaps(1, heaps);

	// ビューポート/シザーはシーンRTのサイズに合わせる.
	D3D12_VIEWPORT vp{0.0f, 0.0f, (float)sceneRT->GetWidth(), (float)sceneRT->GetHeight(), 0.0f, 1.0f};
	D3D12_RECT rect{0, 0, (LONG)sceneRT->GetWidth(), (LONG)sceneRT->GetHeight()};
	commandList_->RSSetViewports(1, &vp);
	commandList_->RSSetScissorRects(1, &rect);

	// RootSignature / PSO / PrimitiveTopology.
	commandList_->SetGraphicsRootSignature(pipelineState_->GetRootSignature());
	commandList_->SetPipelineState(pipelineState_->GetPSO("Standard"));
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::EndScene(SceneRenderTarget* sceneRT) {
	// ここから先、ImGuiがテクスチャとして読めるように遷移.
	sceneRT->TransitionToShaderResource(commandList_);
}

void Renderer::PrepareBackbufferToEditor() {
	auto rtvHandle = swapChain_->GetCurrentRTV();
	// ImGuiのみ描くので深度は不要.
	commandList_->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	float backClear[4] = {0.05f, 0.05f, 0.05f, 1.0f}; // ドッキング背景.
	commandList_->ClearRenderTargetView(rtvHandle, backClear, 0, nullptr);

	// ImGuiがsrvHeap（フォント・シーンRTのSRV）を使うので必ずバインド.
	ID3D12DescriptorHeap* heaps[] = {srvHeap_->GetHeap()};
	commandList_->SetDescriptorHeaps(1, heaps);
}
void Renderer::PrepareBackbufferToGame(uint32_t clientWidth, uint32_t clientHeight, float gameAspect) {
	auto rtvHandle = swapChain_->GetCurrentRTV();
	auto dsvHandle = depthBuffer_->GetDSVHandle();

	commandList_->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	// 全面を黒でクリア（レターボックスの黒帯になる）.
	float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	commandList_->ClearRenderTargetView(rtvHandle, black, 0, nullptr);
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// DescriptorHeap.
	ID3D12DescriptorHeap* heaps[] = {srvHeap_->GetHeap()};
	commandList_->SetDescriptorHeaps(1, heaps);

	// クライアント領域内にアスペクト比を保つ最大矩形を求めてビューポートにする.
	float cw = (float)clientWidth;
	float ch = (float)clientHeight;

	drawWidth_ = cw;
	drawHeight_ = cw / gameAspect;
	if (drawHeight_ > ch) { // 縦がはみ出すなら縦基準.
		drawHeight_ = ch;
		drawWidth_ = ch * gameAspect;
	}
	float offsetX = (cw - drawWidth_) * 0.5f;
	float offsetY = (ch - drawHeight_) * 0.5f;

	D3D12_VIEWPORT vp{offsetX, offsetY, drawWidth_, drawHeight_, 0.0f, 1.0f};
	D3D12_RECT rect{(LONG)offsetX, (LONG)offsetY, (LONG)(offsetX + drawWidth_), (LONG)(offsetY + drawHeight_)};
	commandList_->RSSetViewports(1, &vp);
	commandList_->RSSetScissorRects(1, &rect);

	// RootSignature / PSO / PrimitiveTopology.
	commandList_->SetGraphicsRootSignature(pipelineState_->GetRootSignature());
	commandList_->SetPipelineState(pipelineState_->GetPSO("Standard"));
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::Resize(uint32_t width, uint32_t height) {
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);

	scissorRect_.right = width;
	scissorRect_.bottom = height;

	clientWidth_ = width;
	clientHeight_ = height;
}

void Renderer::DrawGizmos(const GizmoDrawList& drawList, const CameraView& cameraView) {
	const std::span<const GizmoVertex> vertices = drawList.GetVertices();
	if (vertices.empty()) {
		return;
	}

	const D3D12_VERTEX_BUFFER_VIEW vbv = frameVertices_.Allocate(
		vertices.data(), vertices.size_bytes(), sizeof(GizmoVertex)
	);
	if (vbv.SizeInBytes == 0) { // 枯渇。このフレームは諦める.
		return;
	}

	// 頂点は既にワールド空間なので、送るのは viewProjection だけ.
	const Matrix4x4 viewProjection = cameraView.viewMatrix * cameraView.projectionMatrix;

	commandList_->SetPipelineState(pipelineState_->GetPSO("Debug/Gizmo"));
	commandList_->SetGraphicsRootConstantBufferView(1, frameConstants_.Allocate(&viewProjection, sizeof(viewProjection)));
	commandList_->IASetVertexBuffers(0, 1, &vbv);
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList_->DrawInstanced(static_cast<UINT>(vertices.size()), 1, 0, 0);

	// 以降のパスが三角形前提なので必ず戻す。PSOキャッシュも無効化しておく.
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	lastPSO_ = nullptr;
}
void Renderer::DrawSkybox(const CameraView& cameraView) {
	const Material* mat = materialManager_->ResolveOrError(skyboxMaterialHandle_);
	const ShaderDefinition* shader = mat->GetShader();
	if (shader == nullptr || shader->pso == nullptr) {
		return;
	}

	// 平行移動を除いたビュー射影の逆行列。PSでレイ方向の再構築に使う.
	Matrix4x4 rotOnlyView = Matrix4x4::Inverse(cameraView.rotationMatrix);
	Matrix4x4 invVP = Matrix4x4::Inverse(rotOnlyView * cameraView.projectionMatrix);

	commandList_->SetPipelineState(shader->pso);

	// b0(ルートパラメータ0)に行列を流す。ライト枠(パラメータ2)には触れない.
	commandList_->SetGraphicsRootConstantBufferView(0, frameConstants_.Allocate(&invVP, sizeof(invVP)));

	// テクスチャ(パノラマ).
	for (const TextureSlotDesc& slot : shader->textures) {
		commandList_->SetGraphicsRootDescriptorTable(
			PipelineState::kTextureRootParamStart + slot.registerIndex,
			mat->GetTexture(slot.name).gpuHandle
		);
	}

	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->DrawInstanced(3, 1, 0, 0);
}
void Renderer::DrawSprite(const Sprite& sprite) {
	const Material* mat = materialManager_->ResolveOrError(sprite.GetMaterial());
	const ShaderDefinition* shader = mat->GetShader();
	if (shader == nullptr || shader->pso == nullptr) {
		return;
	}

	SpriteTransform transform = sprite.BuildTransform(static_cast<float>(clientWidth_), static_cast<float>(clientHeight_));

	commandList_->SetPipelineState(shader->pso);
	commandList_->SetGraphicsRootConstantBufferView(0, mat->GetCB()->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, frameConstants_.Allocate(&transform, sizeof(transform)));

	for (const TextureSlotDesc& slot : shader->textures) {
		commandList_->SetGraphicsRootDescriptorTable(
			PipelineState::kTextureRootParamStart + slot.registerIndex,
			mat->GetTexture(slot.name).gpuHandle
		);
	}

	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->DrawInstanced(6, 1, 0, 0);
}
void Renderer::DrawModel(
	ModelHandle model,
	const Matrix4x4& world,
	const CameraView& cameraView,
	MaterialHandle materialOverride
) {
	std::span<const Mesh> meshes = modelManager_->ResolveMeshes(model);
	if (meshes.empty()) {
		return;
	}

	Matrix4x4 viewProj = cameraView.viewMatrix * cameraView.projectionMatrix;
	TransformationMatrix wvp{world * viewProj, world};
	commandList_->SetGraphicsRootConstantBufferView(1, frameConstants_.Allocate(&wvp, sizeof(wvp)));

	for (const Mesh& mesh : meshes) {
		for (const SubMesh& sub : mesh.subMeshes) {
			// スロット番号 → マテリアルハンドル → 実体（無効ならエラーマテリアルで描画）.
			// 上書きが指定されていればスロットを無視する.
			const MaterialHandle matHandle = materialOverride.IsValid() ? materialOverride : mesh.materialSlots[sub.materialSlot];
			// ResolveOrError は必ず non-null を返す（エラーマテリアル＝マゼンタ）.
			const Material* mat = materialManager_->ResolveOrError(matHandle);
			// インスタンシング専用シェーダーは、この1個ずつ描く経路では描けない.
			// 描くと VS が未設定のルートSRV（t0）を読みに行きデバイス削除になるため、エラーマテリアルに差し替える.
			if (mat->GetShader() != nullptr && mat->GetShader()->requiresInstancing) {
				mat = materialManager_->ResolveOrError(MaterialHandle{});
			}

			const ShaderDefinition* shader = mat->GetShader();
			if (shader != nullptr && shader->pso != nullptr) {
				commandList_->SetPipelineState(shader->pso);
				for (const TextureSlotDesc& slot : shader->textures) {
					commandList_->SetGraphicsRootDescriptorTable(
						PipelineState::kTextureRootParamStart + slot.registerIndex,
						mat->GetTexture(slot.name).gpuHandle
					);
				}
			}
			commandList_->SetGraphicsRootConstantBufferView(0, mat->GetCB()->GetGPUVirtualAddress());
			commandList_->IASetVertexBuffers(0, 1, &sub.vbv);
			commandList_->DrawInstanced(sub.vertexCount, 1, 0, 0);
		}
	}
}
void Renderer::DrawModelInstanced(
	ModelHandle model,
	std::span<const InstanceData> instances,
	const CameraView& cameraView,
	MaterialHandle material
) {
	std::span<const Mesh> meshes = modelManager_->ResolveMeshes(model);
	if (meshes.empty() || instances.empty()) {
		return;
	}

	const Material* mat = materialManager_->ResolveOrError(material);
	const ShaderDefinition* shader = mat->GetShader();
	if (shader == nullptr || shader->pso == nullptr) {
		return;
	}

	// インスタンシング非対応のシェーダーは VS の b0（CBV）から行列を読むため、
	// ここで描くと未設定の CBV を読んでしまう。1個ずつ通常の経路で描く（粒ごとの色は反映されない）.
	if (!shader->requiresInstancing) {
		for (const InstanceData& instance : instances) {
			DrawModel(model, instance.world, cameraView, material);
		}
		return;
	}

	// 粒ごとの WVP / World / 色を詰めて、フレーム内バッファへ書き込む.
	const Matrix4x4 viewProj = cameraView.viewMatrix * cameraView.projectionMatrix;
	std::vector<ParticleForGPU> gpuInstances;
	gpuInstances.reserve(instances.size());
	for (const InstanceData& instance : instances) {
		gpuInstances.push_back(ParticleForGPU{instance.world * viewProj, instance.world, instance.color});
	}
	const D3D12_GPU_VIRTUAL_ADDRESS instancesAddress = frameConstants_.Allocate(gpuInstances.data(), sizeof(ParticleForGPU) * gpuInstances.size());

	commandList_->SetPipelineState(shader->pso);
	commandList_->SetGraphicsRootConstantBufferView(0, mat->GetCB()->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootShaderResourceView(PipelineState::kInstancingRootParam, instancesAddress);
	for (const TextureSlotDesc& slot : shader->textures) {
		commandList_->SetGraphicsRootDescriptorTable(
			PipelineState::kTextureRootParamStart + slot.registerIndex,
			mat->GetTexture(slot.name).gpuHandle
		);
	}

	const UINT instanceCount = static_cast<UINT>(instances.size());
	for (const Mesh& mesh : meshes) {
		for (const SubMesh& sub : mesh.subMeshes) {
			commandList_->IASetVertexBuffers(0, 1, &sub.vbv);
			commandList_->DrawInstanced(sub.vertexCount, instanceCount, 0, 0);
		}
	}
}


void Renderer::SetPSO(const std::string& name) {
	commandList_->SetPipelineState(pipelineState_->GetPSO(name));
}

void Renderer::SetLight(const DirectionalLight& light) {
	lightManager_.SetDirectional(light);
	commandList_->SetGraphicsRootConstantBufferView(2, lightManager_.GetDirectionalResource()->GetGPUVirtualAddress());
}

} // namespace Cake
