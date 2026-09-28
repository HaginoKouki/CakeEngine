#pragma once

#include <wrl.h>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <d3d12shader.h>
#include <dxcapi.h>
#include <string>
#include <unordered_map>

namespace Cake {

class ShaderCompiler; // 前方宣言

enum class BlendMode {
	kBlendModeNone,     //!< ブレンドなし.
	kBlendModeNormal,   //!< アルファブレンド。Src * SrcA + Dest * (1 - SrcA).
	kBlendModeAdd,      //!< 加算。Src * SrcA + Dest * 1.
	kBlendModeSubtract, //!< 減算。Dest * 1 - Src * SrcA.
	kBlendModeMultiply, //!< 乗算。Src * 0 + Dest * Src.
	kBlendModeScreen,   //!< スクリーン。Src * (1 - Dest) + Dest * 1.
	kCountOfBlendMode   //!< 種類数カウント用。利用してはいけない.
};
enum class VertexLayoutType {
	None, // 頂点バッファなし（SV_VertexID で組み立てる描画）.
	Mesh, // POSITION / TEXCOORD / NORMAL。通常のモデル用.
	Line, // POSITION / COLOR。ギズモなどのデバッグ線用.
};
// PSO 1つ分の設定。項目が増え続けるので構造体で受け取る.
struct PSODesc {
	std::wstring vsPath;
	std::wstring psPath;

	VertexLayoutType vertexLayout = VertexLayoutType::Mesh;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	BlendMode blendMode = BlendMode::kBlendModeNone;
	bool depthTest = true; // false で深度比較そのものを切る（常に手前に描かれる）.
	bool depthWrite = true;
	D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
	D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID;
};

class PipelineState {
public:
	static constexpr uint32_t kMaxTextureSlots = 4; // t0..t3.
	static constexpr uint32_t kTextureRootParamStart = 3;

private:
	ID3D12Device* device_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoMap_;

private:
	void BuildRootSignature(ID3D12Device* device);

	D3D12_BLEND_DESC GetBlendDesc(BlendMode) const;

public:
	void Initialize(ID3D12Device* device);

	void AddPSO(
		const std::string& name,
		ShaderCompiler* shaderCompiler,
		const PSODesc& desc,
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection>* outPSReflection = nullptr
	);

	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
	ID3D12PipelineState* GetPSO(const std::string& name) const;
};

} // namespace Cake
