#pragma once
/*====================================
 *
 * GPU描画命令の発行と描画状態の管理を行うクラス。
 * Model、Camera、Transformを受け取り、WVP行列計算と定数バッファ転送を行い、
 * DrawModelで実際の描画命令（頂点バッファ・インデックスバッファ・PSO設定）を発行する。
 * SceneRenderTarget（オフスクリーン描画）とBackbuffer描画両対応。
 *
 * ====================================*/
#include <cstdint>
#include <string>
#include <span>
#include <d3d12.h>

#include "Engine/Graphics/Buffer/FrameConstantBuffer.h"
#include "Engine/Graphics/Buffer/FrameVertexBuffer.h"

#include "Engine/Asset/Texture/TextureHandle.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"

#include "Engine/Render/LightManager.h"
#include "Engine/Render/CameraView.h"

namespace Cake {

class SwapChain;
class DepthBuffer;
class DescriptorHeap;
class PipelineState;
class TextureManager;
class MaterialManager;
class ModelManager;

struct RendererInitDesc {
	/* --- Graphics層から --- */
	ID3D12Device* device = nullptr;
	SwapChain* swapChain = nullptr;
	DepthBuffer* depthBuffer = nullptr;
	DescriptorHeap* srvHeap = nullptr;
	PipelineState* pipelineState = nullptr;

	/* --- Asset層から --- */
	TextureManager* textureManager = nullptr;
	MaterialManager* materialManager = nullptr; // マテリアルハンドルの解決に使う.
	ModelManager* modelManager = nullptr;       // メッシュの解決に使う.

	/* --- 基準解像度 --- */
	uint32_t referenceWidth = 0;
	uint32_t referenceHeight = 0;

	/* --- 初期サイズ --- */
	uint32_t clientWidth = 0;
	uint32_t clientHeight = 0;

	bool IsValid() const {
		return device != nullptr && swapChain != nullptr && depthBuffer != nullptr && srvHeap != nullptr && pipelineState != nullptr &&
			textureManager != nullptr && materialManager != nullptr && modelManager != nullptr &&
			referenceWidth > 0 && referenceHeight > 0;
	}
};

class CommandManager;
struct Model;
class Material;
class SceneRenderTarget;
class GizmoDrawList;
// インスタンシング描画の1個分.
struct InstanceData {
	Matrix4x4 world;
	Vector4 color = Vector4::One;
};

// スプライト1枚分の描画データ。Scene 層が詰めて DrawSprite に渡す.
struct SpriteDrawItem {
	Matrix4x4 world = Matrix4x4::Identity; // ピクセル単位の配置（Transform のワールド行列）.
	TextureHandle texture{};               // 無効なら既定テクスチャで描く.
	Vector4 color = Vector4::One;
	Vector4 uvRect{0.0f, 0.0f, 1.0f, 1.0f};
	Vector2 size{100.0f, 100.0f}; // ピクセル。useTextureSize が true なら使わない.
	Vector2 pivot{0.5f, 0.5f};
	Vector2 uvOffset{0.0f, 0.0f};
	bool flipX = false;
	bool flipY = false;
	bool useTextureSize = false;
};

class Renderer {
private:
	ID3D12Device* device_ = nullptr;
	SwapChain* swapChain_ = nullptr;
	DepthBuffer* depthBuffer_ = nullptr;
	DescriptorHeap* srvHeap_ = nullptr;
	PipelineState* pipelineState_ = nullptr;
	TextureManager* textureManager_ = nullptr;   // 非所有.
	MaterialManager* materialManager_ = nullptr; // マテリアルハンドルの解決に使う（非所有）.
	ModelManager* modelManager_ = nullptr;       // メッシュ解決に使う（非所有）.

	ID3D12GraphicsCommandList* commandList_ = nullptr;

	LightManager lightManager_;

	// WVP など、フレーム内で使い捨てる定数バッファ（モデルごとの WVP はここから確保する）.
	FrameConstantBuffer frameConstants_;
	// ギズモなど、フレーム内で使い捨てる頂点.
	FrameVertexBuffer frameVertices_;

	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};
	uint32_t referenceWidth_ = 0;
	uint32_t referenceHeight_ = 0;
	uint32_t clientWidth_ = 0;
	uint32_t clientHeight_ = 0;
	float drawWidth_ = 0.0f;
	float drawHeight_ = 0.0f;

	float clearColor_[4] = {0.1f, 0.25f, 0.5f, 1.0f};
	MaterialHandle skyboxMaterialHandle_{};

	ID3D12PipelineState* spritePSO_ = nullptr;

	/* キャッシュ用 */

	ID3D12PipelineState* lastPSO_ = nullptr;
	const Material* lastMat_ = nullptr;

public:
	void Initialize(RendererInitDesc desc);

	void BeginFrame(CommandManager* commandManager);
	void EndFrame(CommandManager* commandManager);

	// オフスクリーンのSRTに描画する.
	void BeginScene(CommandManager* commandManager, SceneRenderTarget* sceneRT);
	// RT->SRTバリア.
	void EndScene(SceneRenderTarget* sceneRT);

	// Debug用: エディタをバックバッファへ描画する準備.
	void PrepareBackbufferToEditor();
	// Release用: ゲームをバックバッファへ直接描く準備.
	// バックバッファを黒でクリアし、クライアント領域内に 16:9 を維持したビューポートを設定し、深度付きで描画を始める.
	void PrepareBackbufferToGame(uint32_t clientWidth, uint32_t clientHeight, float targetAspect);

	void Resize(uint32_t width, uint32_t height);

	// デバッグ用の線をまとめて1回で描く。空なら何もしない.
	void DrawGizmos(const GizmoDrawList& drawList, const CameraView& cameraView);

	// SkyboxをCameraに対して描画する.
	void DrawSkybox(const CameraView&);
	void DrawSprite(const SpriteDrawItem&);
	// 共有モデルを、指定の Transform と Camera で描画する.
	// materialOverride が有効なら、全サブメッシュをそのマテリアル1枚で描く
	// （モデル側の materialSlots は書き換えない。上書きは描画時だけの話）.
	void DrawModel(
		ModelHandle model,
		const Matrix4x4& world,
		const CameraView& cameraView,
		MaterialHandle materialOverride = {}
	);
	// 同じモデルを instances の数だけインスタンシングで描画する（Particle シェーダー用）.
	// インスタンシング非対応のマテリアルなら、1個ずつ DrawModel で描く（粒ごとの色は反映されない）.
	void DrawModelInstanced(
		ModelHandle model,
		std::span<const InstanceData> instances,
		const CameraView& cameraView,
		MaterialHandle material
	);

	void SetPSO(const std::string& name);
	void SetLight(const DirectionalLight& light);
	void SetCamera(const CameraView& cameraView);
	void SetReferenceResolution(uint32_t width, uint32_t height);

	MaterialHandle GetSkyboxMaterial() { return skyboxMaterialHandle_; }
	uint32_t GetClientWidth() const { return clientWidth_; }
	uint32_t GetClientHeight() const { return clientHeight_; }
	float GetDrawWidth() const { return drawWidth_; }
	float GetDrawHeight() const { return drawHeight_; }
};

} // namespace Cake
