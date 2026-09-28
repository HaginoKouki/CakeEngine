#include "Material.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>

#include <fstream>
#include <sstream>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Buffer/GPUResourceUtility.h"

namespace Cake {

void Material::Initialize(ID3D12Device* device, const ShaderDefinition* shader) {
	shader_ = shader;
	assert(shader_ != nullptr && "Materialにシェーダーが割り当てられていません");
	// パラメータ無しでも最低16バイト確保する（サイズ0のCB生成を避ける）.
	uint32_t cbSize = (shader_->cbufferSize > 0) ? shader_->cbufferSize : 16;

	// CPU側の生バッファをcbufferサイズ分だけ確保（0クリア）.
	cpuParams_.assign(cbSize, 0);

	// GPU側のCBを確保してMap.
	cb_ = CreateBufferResource(device, cbSize);
	cb_->Map(0, nullptr, &mappedCB_);

	// 各パラメータをデフォルト値で初期化する.
	for (const ShaderParamDesc& p : shader_->params) {
		switch (p.type) {
			case ShaderParamType::Float:
				SetFloat(p.name, p.defaultValue.x);
				break;
			case ShaderParamType::Float2:
				SetFloat2(p.name, {p.defaultValue.x, p.defaultValue.y});
				break;
			case ShaderParamType::Float3:
				SetFloat3(p.name, {p.defaultValue.x, p.defaultValue.y, p.defaultValue.z});
				break;
			case ShaderParamType::Float4:
			case ShaderParamType::Color:
				SetFloat4(p.name, p.defaultValue);
				break;
			case ShaderParamType::Int:
				SetInt(p.name, static_cast<int32_t>(p.defaultValue.x));
				break;
		}
	}

	// デフォルト値をGPUへ反映.
	Apply();
}

const ShaderParamDesc* Material::FindParam(const std::string& name) const {
	if (shader_ == nullptr) {
		return nullptr;
	}
	for (const ShaderParamDesc& p : shader_->params) {
		if (p.name == name) {
			return &p;
		}
	}
	return nullptr;
}

void Material::SetShader(ID3D12Device* device, const ShaderDefinition* shader) {
	Initialize(device, shader); // textures_ は保持される.
}

void Material::SetFloat(const std::string& name, float v) {
	const ShaderParamDesc* p = FindParam(name);
	if (p == nullptr) {
		return;
	}
	std::memcpy(cpuParams_.data() + p->offset, &v, sizeof(float));
}
void Material::SetFloat2(const std::string& name, const Cake::Vector2& v) {
	const ShaderParamDesc* p = FindParam(name);
	if (p == nullptr) {
		return;
	}
	std::memcpy(cpuParams_.data() + p->offset, &v, sizeof(float) * 2);
}
void Material::SetFloat3(const std::string& name, const Cake::Vector3& v) {
	const ShaderParamDesc* p = FindParam(name);
	if (p == nullptr) {
		return;
	}
	std::memcpy(cpuParams_.data() + p->offset, &v, sizeof(float) * 3);
}
void Material::SetFloat4(const std::string& name, const Cake::Vector4& v) {
	const ShaderParamDesc* p = FindParam(name);
	if (p == nullptr) {
		return;
	}
	std::memcpy(cpuParams_.data() + p->offset, &v, sizeof(float) * 4);
}
void Material::SetColor(const std::string& name, const Cake::Vector4& v) {
	SetFloat4(name, v); // レイアウト上はFloat4と同じ.
}
void Material::SetInt(const std::string& name, int32_t v) {
	const ShaderParamDesc* p = FindParam(name);
	if (p == nullptr) {
		return;
	}
	std::memcpy(cpuParams_.data() + p->offset, &v, sizeof(int32_t));
}
void Material::SetTexture(const std::string& slot, const TextureHandle& handle) {
	if (shader_ != nullptr) {
		const bool exists = std::any_of(
			shader_->textures.begin(), shader_->textures.end(),
			[&](const TextureSlotDesc& s) { return s.name == slot; }
		);
		if (!exists) {
			DebugLog::GetInstance().Log(LogLevel::Warn, "Material", "未知のテクスチャスロット '" + slot + "' (shader: " + shader_->name + ")");
			return;
		}
	}
	textures_[slot] = handle;
}

TextureHandle Material::GetTexture(const std::string& slot) const {
	auto it = textures_.find(slot);
	if (it == textures_.end()) {
		return {};
	}
	return it->second;
}

void Material::Apply() {
	if (mappedCB_ != nullptr && !cpuParams_.empty()) {
		std::memcpy(mappedCB_, cpuParams_.data(), cpuParams_.size());
	}
}

void* Material::GetParamPtr(const std::string& name) {
	const ShaderParamDesc* p = FindParam(name);
	if (p == nullptr) {
		return nullptr;
	}
	return cpuParams_.data() + p->offset;
}

std::vector<Material> Material::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, TextureManager* textureManager) {
	// 中で必要となる変数の宣言.
	std::vector<Material> materials;
	std::unique_ptr<Material> material = nullptr;
	std::string line; // ファイルから読んだ1行を格納するもの.

	// Materialがないときにデフォルトで作るためのラムダ式（"newmtl"のないmtlファイル用）.
	auto CheckMaterialNullptr = [&material]() {
		if (material == nullptr) {
			material = std::make_unique<Material>();
			material->name_ = "default";
		}
	};

	// ファイルを開く.
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open()); // とりあえずファイルが開けなかったら止める.

	// 実際にファイルを読み、Materialを構築していく.
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む.

		// identifierに応じた処理.
		if (identifier == "newmtl") {
#pragma region マテリアル名の定義
			// すでにMaterialが存在している場合は、materialsに登録してから新しいMaterialを作る.
			if (material != nullptr) {
				materials.push_back(std::move(*material));
				material = nullptr;
			}
			material = std::make_unique<Material>();
			s >> material->name_;
#pragma endregion
		} else if (identifier == "map_Kd") {
#pragma region テクスチャファイルの定義
			CheckMaterialNullptr();
			std::string textureFilename;
			s >> textureFilename;
			// 読み込んだテクスチャは "albedo" スロットに格納する（Standardシェーダーのt0）.
			material->textures_["albedo"] = textureManager->Load(directoryPath + "/" + textureFilename);
#pragma endregion
		}
	}
	// 最後のMaterialをmaterialsに登録する.
	if (material != nullptr) {
		materials.push_back(std::move(*material));
	}
	// Materialのリストを返す.
	return materials;
}

Material& Material::FindMaterialByName(std::vector<Material>& materials, const std::string& name) {
	for (Material& material : materials) {
		if (material.GetName() == name) {
			return material;
		}
	}
	assert(false && "Material with the specified name not found.");
	return materials[0]; // これは実際には到達しないはずですが、コンパイルエラーを防ぐために追加しています.
}

const Material& Material::FindMaterialByName(const std::vector<Material>& materials, const std::string& name) {
	for (const Material& material : materials) {
		if (material.GetName() == name) {
			return material;
		}
	}
	assert(false && "Material with the specified name not found.");
	return materials[0]; // これは実際には到達しないはずですが、コンパイルエラーを防ぐために追加しています.
}

void Material::CopyValuesFrom(const Material& other) {
	if (shader_ != other.shader_) {
		DebugLog::GetInstance().Log(
			LogLevel::Warn, "Material",
			"シェーダーが異なるマテリアルから値を複製しようとしました"
		);
		return;
	}
	// cpuParams_ のサイズはシェーダーが同じなら必ず一致する.
	cpuParams_ = other.cpuParams_;
	textures_ = other.textures_;
	dirty_ = true; // 複製直後は未保存扱い.
}

} // namespace Cake
