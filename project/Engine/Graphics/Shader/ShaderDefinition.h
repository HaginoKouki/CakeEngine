#pragma once
/*====================================
 *
 * シェーダーの定義情報を記述する構造体。
 * ファイルパス（.hlsl）、シェーダーパラメータの記述（ShaderParamDesc）、
 * テクスチャスロット情報（TextureSlotDesc）を保持する。
 * ShaderLibraryに登録され、Material生成時に参照される。
 *
 * ====================================*/
#include <string>
#include <vector>
#include <d3d12.h>

#include "Engine/Graphics/Shader/ShaderParam.h"
#include "Engine/Graphics/Pipeline/PipelineState.h"	// BlendModeを使う.

namespace Cake {

// 1つのシェーダーの定義。VS/PSのパス + パラメータスキーマ + PSO設定をまとめたもの.
struct ShaderDefinition {
	std::string name;		// "Standard"など。PSO名も兼ねる.
	std::wstring vsPath;
	std::wstring psPath;

	// Register時に解決される。PipelineStateが所有するPSOを指す（非所有）.
	ID3D12PipelineState* pso = nullptr;

	std::vector<ShaderParamDesc> params;	// cbufferに並ぶパラメータ.
	std::vector<TextureSlotDesc> textures;	// 必要なテクスチャスロット.

	VertexLayoutType vertexLayout = VertexLayoutType::Mesh;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	BlendMode blendMode = BlendMode::kBlendModeNone;
	bool depthTest = true;
	bool depthWrite = true;
	D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
	D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID;

	// エンジン内部専用のシェーダー。マテリアルの選択肢に出さない.
	bool hiddenInEditor = false;

	// ComputeLayoutで埋まる。cbuffer全体のサイズ(16バイト切り上げ後).
	uint32_t cbufferSize = 0;

	// paramsを走査し、各paramのoffsetとcbufferSizeをHLSLのcbufferパッキング規則で計算する.
	void ComputeLayout();
};

}	// namespace Cake
