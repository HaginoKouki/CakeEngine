#include "PipelineState.h"
#include <cassert>
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"

namespace Cake {

namespace {
constexpr const char* kLogCategory = "PipelineState";

// レイアウト種別ごとの入力要素。PSO生成の間だけ参照されるので static で持って良い.
const D3D12_INPUT_ELEMENT_DESC kMeshLayout[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
const D3D12_INPUT_ELEMENT_DESC kLineLayout[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
} // namespace

void PipelineState::Initialize(ID3D12Device* device) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	BuildRootSignature(device);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void PipelineState::AddPSO(
	const std::string& name,
	ShaderCompiler* shaderCompiler,
	const PSODesc& desc,
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection>* outPSReflection
) {
	auto vertexShaderBlob = shaderCompiler->Compile(desc.vsPath, L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	auto pixelShaderBlob = shaderCompiler->Compile(desc.psPath, L"ps_6_0", outPSReflection);
	assert(pixelShaderBlob != nullptr);

	// RasterizerState.
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = desc.cullMode;
	rasterizerDesc.FillMode = desc.fillMode;

	// DepthStencilState.
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = desc.depthTest;
	depthStencilDesc.DepthWriteMask = desc.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// PSO.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature_.Get();
	switch (desc.vertexLayout) {
		case VertexLayoutType::Mesh:
			psoDesc.InputLayout = {kMeshLayout, _countof(kMeshLayout)};
			break;
		case VertexLayoutType::Line:
			psoDesc.InputLayout = {kLineLayout, _countof(kLineLayout)};
			break;
		default:
			psoDesc.InputLayout = {nullptr, 0};
			break;
	}
	psoDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
	psoDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};
	psoDesc.BlendState = GetBlendDesc(desc.blendMode);
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.PrimitiveTopologyType = desc.topologyType;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	AssertHRESULT(hr, "GraphicsPipelineStateの生成: " + name);

	psoMap_[name] = std::move(pso);
}

ID3D12PipelineState* PipelineState::GetPSO(const std::string& name) const {
	auto it = psoMap_.find(name);
	assert(it != psoMap_.end() && "指定されたPSOが見つかりません");
	return it->second.Get();
}

void PipelineState::BuildRootSignature(ID3D12Device* device) {
	// RootParameter.
	D3D12_ROOT_PARAMETER rootParameters[kInstancingRootParam + 1] = {};

	// 0: material CBV (b0, PIXEL).
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// 1: wvp CBV (b0, VERTEX).
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// 2: light CBV (b1, PIXEL).
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].Descriptor.ShaderRegister = 1;

	// 3..: テクスチャスロット（1枚ごとに独立テーブル t0..t3）.
	D3D12_DESCRIPTOR_RANGE texRanges[kMaxTextureSlots] = {};
	for (uint32_t i = 0; i < kMaxTextureSlots; ++i) {
		texRanges[i].BaseShaderRegister = i; // t_i.
		texRanges[i].NumDescriptors = 1;
		texRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		texRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		uint32_t p = kTextureRootParamStart + i; // p = 3, 4, 5, 6.
		// テクスチャ t_i 用テーブル (PIXEL).
		rootParameters[p].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[p].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters[p].DescriptorTable.pDescriptorRanges = &texRanges[i];
		rootParameters[p].DescriptorTable.NumDescriptorRanges = 1;
	}

	// 7: インスタンシング用 StructuredBuffer (t0, VERTEX).
	// ルートSRVなので、ディスクリプタを作らずGPUアドレスを直接渡せる.
	rootParameters[kInstancingRootParam].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[kInstancingRootParam].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[kInstancingRootParam].Descriptor.ShaderRegister = 0; // t0.
	rootParameters[kInstancingRootParam].Descriptor.RegisterSpace = 0;

	// StaticSampler.
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	desc.pParameters = rootParameters;
	desc.NumParameters = _countof(rootParameters);
	desc.pStaticSamplers = staticSamplers;
	desc.NumStaticSamplers = _countof(staticSamplers);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugLog::GetInstance().Log(
			LogLevel::Error,
			kLogCategory,
			reinterpret_cast<char*>(errorBlob->GetBufferPointer())
		);
		assert(false);
	}
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	AssertHRESULT(hr, "RootSignatureの生成");
}

D3D12_BLEND_DESC PipelineState::GetBlendDesc(BlendMode blendMode) const {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	// 透明度の列は、全モード共通で「重ね塗り」にする.
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	switch (blendMode) {
		case BlendMode::kBlendModeNone:
			blendDesc.RenderTarget[0].BlendEnable = FALSE;
			break;
		case BlendMode::kBlendModeAdd:
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;
		case BlendMode::kBlendModeSubtract:
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;
		case BlendMode::kBlendModeMultiply:
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		case BlendMode::kBlendModeScreen:
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;
		default:
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			break;
	}
	return blendDesc;
}

} // namespace Cake
