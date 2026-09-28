#include "ShaderLibrary.h"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Pipeline/PipelineState.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "ShaderLibrary";
}

void ShaderLibrary::Initialize(PipelineState* pipelineState, ShaderCompiler* shaderCompiler) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	pipelineState_ = pipelineState;
	shaderCompiler_ = shaderCompiler;

	RegisterDefaultShaders();

	errorShader_ = Find("__error_shader__");
	defaultShader_ = Find("Standard");

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}
void Cake::ShaderLibrary::RegisterDefaultShaders() {
	Cake::ShaderDefinition error;
	error.name = "__error_shader__";
	error.vsPath = L"EngineResources/shaders/Color/Color.VS.hlsl";
	error.psPath = L"EngineResources/shaders/Color/Color.PS.hlsl";
	error.params = {
		ShaderParamDesc{
			.name = "color",
			.type = ShaderParamType::Color,
			.defaultValue = {1, 0, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 0.0f
		},
	};
	error.textures = {};
	Register(error);

	Cake::ShaderDefinition standard;
	standard.name = "Standard";
	standard.vsPath = L"EngineResources/shaders/Standard/Standard.VS.hlsl";
	standard.psPath = L"EngineResources/shaders/Standard/Standard.PS.hlsl";
	standard.params = {
		ShaderParamDesc{
			.name = "color",
			.type = ShaderParamType::Color,
			.defaultValue = {1, 1, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
		ShaderParamDesc{
			.name = "tiling",
			.type = ShaderParamType::Float2,
			.defaultValue = {1, 1, 0, 0},
			.uiMin = 0.0f,
			.uiMax = 10.0f,
		},
		ShaderParamDesc{
			.name = "offset",
			.type = ShaderParamType::Float2,
			.defaultValue = {0, 0, 0, 0},
			.uiMin = 0.0f,
			.uiMax = 1.0f,
		},
		ShaderParamDesc{
			.name = "lightingType",
			.type = ShaderParamType::Int,
			.defaultValue = {1, 0, 0, 0},
			.uiMin = 0.0f,
			.uiMax = 2.0f
		}
	};
	standard.textures = {
		TextureSlotDesc{
			.name = "albedo",
			.registerIndex = 0
		}, // t0.
	};
	Register(standard);

	Cake::ShaderDefinition color;
	color.name = "Unlit/Color";
	color.vsPath = L"EngineResources/shaders/Color/Color.VS.hlsl";
	color.psPath = L"EngineResources/shaders/Color/Color.PS.hlsl";
	color.params = {
		ShaderParamDesc{
			.name = "color",
			.type = ShaderParamType::Color,
			.defaultValue = {1, 1, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
	};
	color.textures = {};
	Register(color);

	Cake::ShaderDefinition wireFrame;
	wireFrame.name = "Unlit/WireFrame";
	wireFrame.vsPath = L"EngineResources/shaders/Color/Color.VS.hlsl";
	wireFrame.psPath = L"EngineResources/shaders/Color/Color.PS.hlsl";
	wireFrame.params = {
		ShaderParamDesc{
			.name = "color",
			.type = ShaderParamType::Color,
			.defaultValue = {0, 1, 0, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
	};
	wireFrame.textures = {};
	wireFrame.fillMode = D3D12_FILL_MODE_WIREFRAME;
	Register(wireFrame);

	Cake::ShaderDefinition panoramic;
	panoramic.name = "Skybox/Panoramic";
	panoramic.vsPath = L"EngineResources/shaders/Skybox/Panoramic/Panoramic.VS.hlsl";
	panoramic.psPath = L"EngineResources/shaders/Skybox/Panoramic/Panoramic.PS.hlsl";
	panoramic.vertexLayout = VertexLayoutType::None;
	panoramic.depthWrite = false;
	panoramic.cullMode = D3D12_CULL_MODE_NONE;
	panoramic.params = {};
	panoramic.textures = {
		TextureSlotDesc{
			.name = "Spherical",
			.registerIndex = 0
		}, // t0.
	};
	Register(panoramic);

	Cake::ShaderDefinition sprite;
	sprite.name = "Sprite";
	sprite.vsPath = L"EngineResources/shaders/Sprite/Sprite.VS.hlsl";
	sprite.psPath = L"EngineResources/shaders/Sprite/Sprite.PS.hlsl";
	sprite.params = {
		ShaderParamDesc{
			.name = "color",
			.type = ShaderParamType::Color,
			.defaultValue = {1, 1, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
		ShaderParamDesc{
			.name = "uvRect",
			.type = ShaderParamType::Float4,
			.defaultValue = {1, 1, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
	};
	sprite.textures = {
		TextureSlotDesc{
			.name = "main",
			.registerIndex = 0
		}, // t0.
	};
	Register(sprite);

	Cake::ShaderDefinition gizmo;
	gizmo.name = "Debug/Gizmo";
	gizmo.vsPath = L"EngineResources/shaders/Gizmo/Gizmo.VS.hlsl";
	gizmo.psPath = L"EngineResources/shaders/Gizmo/Gizmo.PS.hlsl";
	gizmo.vertexLayout = VertexLayoutType::Line;
	gizmo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	gizmo.blendMode = BlendMode::kBlendModeNormal; // ← 既存の通常アルファの名前に合わせる.
	gizmo.depthWrite = false;                      // 線が深度を汚さないようにする.
	gizmo.depthTest = true;                        // false にすると壁の裏でも見えるようになる.
	gizmo.cullMode = D3D12_CULL_MODE_NONE;
	gizmo.hiddenInEditor = true;
	gizmo.params = {};
	gizmo.textures = {};
	Register(gizmo);
}

void ShaderLibrary::Register(ShaderDefinition definition) {
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> psReflection;

	// PSOを生成して登録する(PSO名はシェーダー名と一致させる).
	PSODesc desc{
		.vsPath = definition.vsPath,
		.psPath = definition.psPath,
		.vertexLayout = definition.vertexLayout,
		.topologyType = definition.topologyType,
		.blendMode = definition.blendMode,
		.depthTest = definition.depthTest,
		.depthWrite = definition.depthWrite,
		.cullMode = definition.cullMode,
		.fillMode = definition.fillMode,
	};
	pipelineState_->AddPSO(definition.name, shaderCompiler_, desc, &psReflection);
	// 生成済みPSOへのポインタを定義に持たせる.
	definition.pso = pipelineState_->GetPSO(definition.name);

	// リフレクションで offset / cbufferSize を埋める.
	ApplyReflection(definition, psReflection.Get());

	DebugLog::GetInstance().Log(
		LogLevel::Info,
		kLogCategory,
		"Registered shader: " + definition.name
	);

	// 定義を保持する.
	definitions_[definition.name] = std::move(definition);
}

const ShaderDefinition* ShaderLibrary::Find(const std::string& name) const {
	auto it = definitions_.find(name);
	if (it == definitions_.end()) {
		return errorShader_;
	}
	return &it->second;
}

std::vector<std::string> ShaderLibrary::GetAllNames() const {
	std::vector<std::string> names;
	names.reserve(definitions_.size());
	for (const auto& [name, def] : definitions_) {
		names.push_back(name);
	}
	return names;
}

/// <summary>
/// HLSL の構造体メンバを再帰的に降りて "メンバ名 → 絶対オフセット" を集める.
/// </summary>
/// <param name="type">シェーダーの型</param>
/// <param name="baseOffset">基準オフセット</param>
/// <param name="out">結果を格納するマップ</param>
static void CollectMembers(
	ID3D12ShaderReflectionType* type, uint32_t baseOffset,
	std::unordered_map<std::string, uint32_t>& out
) {

	D3D12_SHADER_TYPE_DESC td{};
	type->GetDesc(&td);

	// 構造体ならメンバを1段降りる.
	if (td.Class == D3D_SVC_STRUCT) {
		for (UINT i = 0; i < td.Members; ++i) {
			ID3D12ShaderReflectionType* member = type->GetMemberTypeByIndex(i);
			const char* memberName = type->GetMemberTypeName(i);
			D3D12_SHADER_TYPE_DESC md{};
			member->GetDesc(&md);
			// td.Offset基準からのメンバの相対offsetを足す.
			CollectMembers(member, baseOffset + md.Offset, out);
			if (md.Class != D3D_SVC_STRUCT && memberName != nullptr) {
				out[memberName] = baseOffset + md.Offset;
			}
		}
	}
}

void ShaderLibrary::ApplyReflection(ShaderDefinition& definition, ID3D12ShaderReflection* reflection) {
	if (reflection == nullptr) {
		// リフレクションがない場合は手計算.
		definition.ComputeLayout();
		return;
	}

	// シェーダーの全体像を確保.
	D3D12_SHADER_DESC shaderDesc{};
	reflection->GetDesc(&shaderDesc);

	// マテリアル用のcbufferを探す.
	ID3D12ShaderReflectionConstantBuffer* materialCB = nullptr;
	D3D12_SHADER_BUFFER_DESC bufferDesc{};
	for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC bind{};
		reflection->GetResourceBindingDesc(i, &bind);
		if (bind.Type == D3D_SIT_CBUFFER && bind.BindPoint == 0 && bind.Space == 0) {
			materialCB = reflection->GetConstantBufferByName(bind.Name);
			materialCB->GetDesc(&bufferDesc);
			break;
		}
	}

	// cbufferがない場合早期リターン.
	if (materialCB == nullptr) {
		definition.cbufferSize = 0;
		return;
	}
	definition.cbufferSize = bufferDesc.Size;

	// cbufferの全メンバを "名前→絶対offset" で集める（ネスト対応）.
	std::unordered_map<std::string, uint32_t> offsets;
	for (UINT i = 0; i < bufferDesc.Variables; ++i) {
		ID3D12ShaderReflectionVariable* var = materialCB->GetVariableByIndex(i);
		D3D12_SHADER_VARIABLE_DESC vd{};	// 変数名とcbuffer先頭からのoffsetを取得.
		var->GetDesc(&vd);
		ID3D12ShaderReflectionType* type = var->GetType();
		D3D12_SHADER_TYPE_DESC td{};	// 型情報などを取得.
		type->GetDesc(&td);

		if (td.Class == D3D_SVC_STRUCT) {
			// 構造体ならメンバを展開して一番子の変数まで見る.
			CollectMembers(type, vd.StartOffset, offsets);
		} else {
			// フラットな変数ならそのまま登録.
			offsets[vd.Name] = vd.StartOffset;
		}
	}

	// paramsにoffsetを割り当てる.
	for (ShaderParamDesc& p : definition.params) {
		auto it = offsets.find(p.name);
		if (it != offsets.end()) {
			p.offset = it->second;
		} else {
			// paramsに項目があるのにシェーダー側で見つからない場合.
			DebugLog::GetInstance().Log(
				LogLevel::Warn,
				kLogCategory,
				"Reflection: param '" + p.name + "' not found (shader: " + definition.name + ")"
			);
		}
	}
}

} // namespace Cake
