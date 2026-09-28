#include "MaterialSerializer.h"

#include <cstdint>
#include <fstream>

#include "externals/nlohmann/json.hpp"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Shader/ShaderLibrary.h"
#include "Engine/Graphics/Shader/ShaderDefinition.h"
#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Asset/Material/Material.h"

using json = nlohmann::json;

namespace Cake {
namespace {
constexpr const char* kLogCategory = "MaterialSerializer";
}

bool SaveMaterials(
	Material& materials,
	TextureManager* textureManager,
	const std::string& path
) {
	json root;
	json materialsJson = json::object();

	const TextureHandle white = textureManager->GetDefaultTexture();

	const ShaderDefinition* shader = materials.GetShader();
	if (shader == nullptr) {
		return false;
	}

	json m;
	m["shader"] = shader->name;

	// パラメータ（型ごとに配列で書き出す）.
	json params = json::object();
	for (const ShaderParamDesc& p : shader->params) {
		void* ptr = materials.GetParamPtr(p.name);
		if (ptr == nullptr) {
			return false;
		}
		float* f = static_cast<float*>(ptr);
		switch (p.type) {
			case ShaderParamType::Float:
				params[p.name] = {f[0]};
				break;
			case ShaderParamType::Float2:
				params[p.name] = {f[0], f[1]};
				break;
			case ShaderParamType::Float3:
				params[p.name] = {f[0], f[1], f[2]};
				break;
			case ShaderParamType::Float4:
			case ShaderParamType::Color:
				params[p.name] = {f[0], f[1], f[2], f[3]};
				break;
			case ShaderParamType::Int:
				params[p.name] = {*static_cast<int32_t*>(ptr)};
				break;
		}
	}
	m["params"] = params;

	// テクスチャ（スロット名 → ファイルパス。未設定=白はスキップ）.
	json textures = json::object();
	for (const TextureSlotDesc& slot : shader->textures) {
		TextureHandle h = materials.GetTexture(slot.name);
		if (h == white) {
			continue;
		} // 未割り当て（白ダミー）は保存しない.
		const TextureData* td = textureManager->Resolve(h);
		if (td == nullptr) {
			continue;
		} // 無効なら書き出さない.
		if (!td->filePath.empty()) {
			textures[slot.name] = td->filePath;
		}
	}
	m["textures"] = textures;

	materialsJson[materials.GetName()] = m;

	root["materials"] = materialsJson;

	std::ofstream ofs(path);
	if (!ofs.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "Failed to open for write: " + path);
		return false;
	}
	ofs << root.dump(2); // インデント2で整形.
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, "Saved: " + path);
	return true;
}

bool LoadMaterials(
	Material& materials,
	ID3D12Device* device,
	const ShaderLibrary* shaderLibrary,
	TextureManager* textureManager,
	const std::string& path,
	const std::string& entryName
) {
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "No file: " + path);
		return false;
	}

	json root;
	try {
		ifs >> root;
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::string("Parse error: ") + e.what());
		return false;
	}

	if (!root.contains("materials")) {
		return false;
	}
	const json& materialsJson = root["materials"];

	const TextureHandle white = textureManager->GetDefaultTexture();

	const std::string key = entryName.empty() ? materials.GetName() : entryName;
	auto it = materialsJson.find(key);
	if (it == materialsJson.end()) {
		return false; // ファイルに無いマテリアルはそのまま.
	}
	const json& m = *it;

	// シェーダー適用（先にやる。cbufferレイアウトが決まる）.
	if (m.contains("shader")) {
		const ShaderDefinition* shader = shaderLibrary->Find(m["shader"].get<std::string>());
		if (shader != nullptr) {
			materials.SetShader(device, shader);
		}
	}
	const ShaderDefinition* shader = materials.GetShader();
	if (shader == nullptr) {
		return false;
	}

	// パラメータ適用（型はシェーダー定義から引く）.
	if (m.contains("params")) {
		const json& params = m["params"];
		for (const ShaderParamDesc& p : shader->params) {
			auto pit = params.find(p.name);
			if (pit == params.end()) {
				continue;
			}
			const json& v = *pit;
			switch (p.type) {
				case ShaderParamType::Float:
					materials.SetFloat(p.name, v[0].get<float>());
					break;
				case ShaderParamType::Float2:
					materials.SetFloat2(p.name, {v[0].get<float>(), v[1].get<float>()});
					break;
				case ShaderParamType::Float3:
					materials.SetFloat3(p.name, {v[0].get<float>(), v[1].get<float>(), v[2].get<float>()});
					break;
				case ShaderParamType::Float4:
				case ShaderParamType::Color:
					materials.SetFloat4(p.name, {v[0].get<float>(), v[1].get<float>(), v[2].get<float>(), v[3].get<float>()});
					break;
				case ShaderParamType::Int:
					materials.SetInt(p.name, v[0].get<int32_t>());
					break;
			}
		}
	}

	// テクスチャ適用（パスからLoad。キャッシュ済みなら既存ハンドル）.
	if (m.contains("textures")) {
		const json& textures = m["textures"];
		for (auto tit = textures.begin(); tit != textures.end(); ++tit) {
			materials.SetTexture(tit.key(), textureManager->Load(tit.value().get<std::string>()));
		}
	}

	// 要求スロットの未割り当てを白で埋める（シェーダー変更でスロットが増えた場合の保険）.
	for (const TextureSlotDesc& slot : shader->textures) {
		if (materials.GetTexture(slot.name).gpuHandle.ptr == 0) {
			materials.SetTexture(slot.name, white);
		}
	}

	materials.Apply();

	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, "Loaded: " + path);
	return true;
}

bool PeekMaterialInfo(const std::string& path, std::string& outName, std::string& outShaderName) {
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "ファイルがありません: " + path);
		return false;
	}

	json root;
	try {
		ifs >> root;
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::string("解析に失敗: ") + e.what());
		return false;
	}

	if (!root.contains("materials") || !root["materials"].is_object()) {
		return false;
	}
	const json& materialsJson = root["materials"];
	if (materialsJson.empty()) {
		return false;
	}

	// SaveMaterials は1ファイルに1マテリアルしか書かないので、先頭のエントリを採用する.
	auto it = materialsJson.begin();
	if (!it->contains("shader")) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "shader が指定されていません: " + path);
		return false;
	}

	outName = it.key();
	outShaderName = it->at("shader").get<std::string>();
	return true;
}

} // namespace Cake
