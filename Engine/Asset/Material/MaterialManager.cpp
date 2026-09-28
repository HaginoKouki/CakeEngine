#include "MaterialManager.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <format>
#include <iostream>
#include <filesystem>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Shader/ShaderLibrary.h"
#include "Engine/Graphics/Shader/ShaderDefinition.h"
#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Asset/Material/MaterialSerializer.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "MaterialManager";
}

void MaterialManager::Initialize(ID3D12Device* device, TextureManager* textureManager, const ShaderLibrary* shaderLibrary) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	textureManager_ = textureManager;
	shaderLibrary_ = shaderLibrary;

	// pool_ の途中再確保でハンドルは無効化されないが、初期確保しておくと再確保頻度が減る.
	pool_.reserve(64);
	generations_.reserve(64);

	errorMaterial_ = CreateMaterial("__error_material__", shaderLibrary->errorShader_->name);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

MaterialHandle MaterialManager::CreateMaterial(const std::string& materialName, const std::string& shaderName, MaterialOrigin origin) {
	std::string key = GetNewKeyName(materialName);

	const ShaderDefinition* shader = shaderLibrary_->Find(shaderName);
	assert(shader != nullptr && "指定シェーダーが見つかりません");

	Material material;
	material.SetName(key);
	material.SetOrigin(origin);
	material.Initialize(device_, shader);
	for (const TextureSlotDesc& slot : shader->textures) {
		material.SetTexture(slot.name, textureManager_->defaultTexture_);
	}

	MaterialHandle handle = Emplace(std::move(material));
	byName_[key] = handle;
	return handle;
}
MaterialHandle MaterialManager::LoadFromFile(const std::string& filePath) {
	if (auto it = byPath_.find(filePath); it != byPath_.end()) {
		return it->second;
	}

	std::string entryName;
	std::string shaderName;
	if (!PeekMaterialInfo(filePath, entryName, shaderName)) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, ".mat を読めません: " + filePath);
		return {};
	}

	MaterialHandle handle = CreateMaterial(entryName, shaderName, MaterialOrigin::Asset);
	Material* material = Resolve(handle);
	if (material == nullptr) {
		return {};
	}
	material->SetAssetPath(filePath); // ここが Ctrl+S の書き戻し先になる.

	if (!LoadMaterials(*material, device_, shaderLibrary_, textureManager_, filePath, entryName)) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "内容の適用に失敗: " + filePath);
	}
	material->Apply();
	material->ClearDirty(); // 読み込み直後はファイルと一致している.

	byPath_[filePath] = handle;
	return handle;
}

MaterialHandle MaterialManager::Emplace(Material&& material) {
	// 末尾に追加する。index は追加位置、generation は 1 始まり（0 は無効ハンドルの初期値と区別）.
	const uint32_t index = static_cast<uint32_t>(pool_.size());
	pool_.push_back(std::move(material));
	generations_.push_back(1);

	MaterialHandle handle;
	handle.index = index;
	handle.generation = generations_[index];
	return handle;
}

bool MaterialManager::IsAlive(MaterialHandle handle) const {
	if (!handle.IsValid() || handle.index >= pool_.size()) {
		return false;
	}
	return generations_[handle.index] == handle.generation;
}

Material* MaterialManager::Resolve(MaterialHandle handle) {
	if (!IsAlive(handle)) {
		return nullptr;
	}
	return &pool_[handle.index];
}

const Material* MaterialManager::Resolve(MaterialHandle handle) const {
	if (!IsAlive(handle)) {
		return nullptr;
	}
	return &pool_[handle.index];
}
Material* MaterialManager::ResolveOrError(MaterialHandle handle) {
	if (Material* m = Resolve(handle)) {
		return m;
	}
	return Resolve(errorMaterial_); // これは必ず有効.
}
const Material* MaterialManager::ResolveOrError(MaterialHandle handle) const {
	if (const Material* m = Resolve(handle)) {
		return m;
	}
	return Resolve(errorMaterial_); // これは必ず有効.
}

MaterialHandle MaterialManager::FindHandle(const std::string& materialName) const {
	auto it = byName_.find(materialName);
	if (it == byName_.end()) {
		return {}; // 無効ハンドル.
	}
	return it->second;
}

size_t MaterialManager::SaveDirtyMaterials() {
	size_t saved = 0;
	for (Material& material : pool_) {
		if (!material.IsDirty()) {
			continue;
		}
		if (!material.HasAssetPath()) {
			// コード生成のマテリアルは書き戻し先が無い。毎フレーム出ないよう dirty は落とす.
			DebugLog::GetInstance().Log(
				LogLevel::Warn, kLogCategory,
				"保存先が無いので変更を破棄します: " + material.GetName() + " (.mat として切り出すには Create Material Asset を使ってください)"
			);
			material.ClearDirty();
			continue;
		}
		if (SaveMaterials(material, textureManager_, material.GetAssetPath())) {
			material.ClearDirty();
			++saved;
		}
	}
	if (saved > 0) {
		DebugLog::GetInstance().Log(
			LogLevel::Info, kLogCategory, "マテリアルを保存: " + std::to_string(saved) + " 件"
		);
	}
	return saved;
}

bool MaterialManager::HasDirtyMaterials() const {
	return std::any_of(pool_.begin(), pool_.end(), [](const Material& m) { return m.IsDirty(); });
}

MaterialHandle MaterialManager::CreateAsset(
	const std::string& materialName, const std::string& shaderName, const std::string& path
) {
	if (auto it = byPath_.find(path); it != byPath_.end()) {
		return it->second; // 同じパスは作り直さない.
	}

	MaterialHandle handle = CreateMaterial(materialName, shaderName, MaterialOrigin::Asset);
	Material* material = Resolve(handle);
	if (material == nullptr) {
		return {};
	}
	material->SetAssetPath(path);
	material->Apply();

	std::error_code ec;
	std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
	if (!SaveMaterials(*material, textureManager_, path)) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "書き出しに失敗: " + path);
		return {};
	}
	material->ClearDirty();

	byPath_[path] = handle;
	return handle;
}

MaterialHandle MaterialManager::CreateAssetFrom(MaterialHandle source, const std::string& path) {
	const Material* src = Resolve(source);
	if (src == nullptr || src->GetShader() == nullptr) {
		return {};
	}
	// 【重要】この後の CreateAsset が pool_ を push_back するため、src はここで無効になる。
	// 必要な情報は値としてコピーしておくこと.
	const std::string shaderName = src->GetShader()->name;
	const std::string stem = std::filesystem::path(path).stem().string();

	MaterialHandle handle = CreateAsset(stem, shaderName, path);

	Material* dst = Resolve(handle);
	const Material* srcAgain = Resolve(source); // 再確保に備えて取り直す.
	if (dst == nullptr || srcAgain == nullptr) {
		return handle;
	}

	dst->CopyValuesFrom(*srcAgain);
	dst->Apply();
	if (SaveMaterials(*dst, textureManager_, path)) { // 値を上書きしたので書き直す.
		dst->ClearDirty();
	}
	return handle;
}

std::vector<std::string> MaterialManager::GetAllNames() const {
	std::vector<std::string> names;
	names.reserve(byName_.size());
	for (const auto& [key, handle] : byName_) {
		names.push_back(key);
	}
	std::sort(names.begin(), names.end()); // unordered_map は順序不定なのでソートしないとUIが毎フレーム踊る.
	return names;
}

std::string MaterialManager::GetNewKeyName(const std::string& cacheKey) const {
	std::string returnKey = cacheKey;
	auto it = byName_.find(returnKey);
	if (it != byName_.end()) {
		size_t dotPos = returnKey.rfind('.');
		std::string last = (dotPos != std::string::npos) ? returnKey.substr(dotPos + 1) : std::string();
		bool is_number = !last.empty() && std::all_of(last.begin(), last.end(), [](unsigned char c) { return std::isdigit(c); });

		if (is_number && dotPos != std::string::npos) {
			int value = std::stoi(last);
			std::string base = returnKey.substr(0, dotPos);
			return GetNewKeyName(std::format("{}.{}", base, value + 1));
		} else {
			return GetNewKeyName(std::format("{}.1", returnKey));
		}
	}
	return returnKey;
}

} // namespace Cake
