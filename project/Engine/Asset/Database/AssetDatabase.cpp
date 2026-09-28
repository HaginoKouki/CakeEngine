#include "AssetDatabase.h"

#include <algorithm>
#include <filesystem>
#include <span>
#include <unordered_set>

#include "Engine/Foundation/Debug/DebugLog.h"

#include "Engine/Asset/Database/AssetMeta.h"
#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialManager.h"
#include "Engine/Asset/Model/Mesh.h"
#include "Engine/Asset/Model/PrimitiveShape.h"
#include "Engine/Asset/Model/ModelManager.h"
#include "Engine/Asset/Texture/TextureManager.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "AssetDatabase";

const char* ToString(AssetType type) {
	switch (type) {
		case AssetType::Model: return "Model";
		case AssetType::Texture: return "Texture";
		case AssetType::Material: return "Material";
		default: return "Unknown";
	}
}
// 1-A の暫定変換。フェーズ5の切り替えで AssetDatabase と一緒に削除する（仕様書 12章）.
ImporterType ToImporterType(AssetType type) {
	switch (type) {
		case AssetType::Model:
			return ImporterType::Model;
		case AssetType::Texture:
			return ImporterType::Texture;
		case AssetType::Material:
			return ImporterType::Material;
		case AssetType::Scene:
			return ImporterType::Scene;
		default:
			return ImporterType::Default;
	}
}
} // namespace

std::string AssetDatabase::NormalizePath(const std::string& path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '\\', '/');
	return result;
}

AssetType AssetDatabase::DetectType(const std::string& path) {
	std::filesystem::path fsPath(path);
	std::string ext = fsPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});

	if (ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".fbx") {
		return AssetType::Model;
	}
	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".dds" || ext == ".tga" || ext == ".bmp") {
		return AssetType::Texture;
	}
	if (ext == ".mat") {
		return AssetType::Material;
	}
	// .mtl はOBJの付属物で、単独では参照されない（中のマテリアルはモデルの
	// サブアセットとして LocalId で指す）ため、意図的に対象外とする.
	return AssetType::Unknown;
}

#pragma region 索引

bool AssetDatabase::RegisterAsset(const std::string& path, AssetType type) {
	AssetMeta meta;
	if (!LoadOrCreateAssetMeta(path, ToImporterType(type), meta)) {
		return false; // 詳細は AssetMeta 側でログ済み.
	}

	// 同じGUIDが二重登録された場合。アセットを .meta ごとコピーすると起きる.
	if (auto it = byGuid_.find(meta.guid); it != byGuid_.end()) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			"GUIDが重複しています: " + path + " / " + it->second.path
			+ " (片方の .meta を削除して再スキャンしてください)"
		);
		return false;
	}

	AssetEntry entry;
	entry.guid = meta.guid;
	entry.path = path;
	entry.type = type;

	byPath_[path] = meta.guid;
	byGuid_[meta.guid] = std::move(entry);
	return true;
}

void AssetDatabase::RegisterBuiltinAssets() {
	// GUID もパスも固定なので、.meta の読み書きは通さない.
	for (size_t i = 0; i < kPrimitiveShapeCount; ++i) {
		AssetEntry entry;
		entry.guid = kPrimitiveShapeGuids[i];
		entry.path = kPrimitiveShapeKeys[i];
		entry.type = AssetType::Model;
		entry.builtin = true;

		byPath_[entry.path] = entry.guid;
		byGuid_[entry.guid] = std::move(entry);
	}
}

void AssetDatabase::Initialize(
	const std::string& rootPath,
	TextureManager* textureManager,
	MaterialManager* materialManager,
	ModelManager* modelManager
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	rootPath_ = NormalizePath(rootPath);
	textureManager_ = textureManager;
	materialManager_ = materialManager;
	modelManager_ = modelManager;

	Rescan();

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void AssetDatabase::Rescan() {
	byGuid_.clear();
	byPath_.clear();

	RegisterBuiltinAssets();

	std::error_code ec;
	if (!std::filesystem::exists(rootPath_, ec)) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "ルートフォルダがありません: " + rootPath_);
		return;
	}

	size_t skipped = 0;

	// ec を渡す版を使い、権限エラー等で例外を投げさせない.
	std::filesystem::recursive_directory_iterator it(rootPath_, ec);
	const std::filesystem::recursive_directory_iterator end;
	for (; it != end; it.increment(ec)) {
		if (ec) {
			DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "走査に失敗したフォルダをスキップ: " + ec.message());
			ec.clear();
			continue;
		}
		if (!it->is_regular_file(ec)) {
			continue;
		}

		// generic_string はプラットフォームによらずスラッシュ区切りを返す.
		const std::string path = it->path().generic_string();

		// .meta 自体は索引の対象にしない.
		if (it->path().extension() == ".meta") {
			continue;
		}

		const AssetType type = DetectType(path);
		if (type == AssetType::Unknown) {
			continue; // 対象外の拡張子には .meta を作らない.
		}

		if (!RegisterAsset(path, type)) {
			++skipped;
		}
	}

	// 実体が消えた .meta（孤児）はここでは削除しない。復元できる可能性があるため.
	std::string message = "スキャン完了: " + std::to_string(byGuid_.size()) + " 件";
	if (skipped > 0) {
		message += " / 失敗 " + std::to_string(skipped) + " 件";
	}
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, message);
}

const AssetEntry* AssetDatabase::Find(const Guid& guid) const {
	auto it = byGuid_.find(guid);
	if (it == byGuid_.end()) {
		return nullptr;
	}
	return &it->second;
}

const AssetEntry* AssetDatabase::FindByPath(const std::string& path) const {
	auto it = byPath_.find(NormalizePath(path));
	if (it == byPath_.end()) {
		return nullptr;
	}
	return Find(it->second);
}

Guid AssetDatabase::GetGuid(const std::string& path) const {
	auto it = byPath_.find(NormalizePath(path));
	if (it == byPath_.end()) {
		return Guid::Invalid();
	}
	return it->second;
}

std::vector<const AssetEntry*> AssetDatabase::GetEntriesOfType(AssetType type) const {
	std::vector<const AssetEntry*> entries;
	for (const auto& [guid, entry] : byGuid_) {
		if (entry.type == type) {
			entries.push_back(&entry);
		}
	}
	// unordered_map の走査順は不定なので、一覧の並びをパスで安定させる.
	std::sort(entries.begin(), entries.end(), [](const AssetEntry* a, const AssetEntry* b) {
		return a->path < b->path;
	});
	return entries;
}

std::vector<MaterialRefEntry> AssetDatabase::GetSelectableMaterials() {
	std::vector<MaterialRefEntry> entries;

	// 1) .mat ファイル本体.
	for (const AssetEntry* asset : GetEntriesOfType(AssetType::Material)) {
		MaterialRefEntry entry;
		entry.guid = asset->guid;
		entry.localId = kSelfLocalId;
		entry.displayName = asset->path;
		entries.push_back(std::move(entry));
	}

	// 2) 読み込み済みモデルに埋め込まれたマテリアル.
	// 未読み込みのモデルは中身が分からないため列挙できない（既知の制限）.
	if (modelManager_ != nullptr && materialManager_ != nullptr) {
		for (const ModelData& model : modelManager_->GetPool()) {
			const Guid modelGuid = GetGuid(model.filePath);
			if (!modelGuid.IsValid()) {
				continue;
			}

			// 同じマテリアルが複数スロットから参照されるので重複を除く.
			std::unordered_set<LocalId> seen;
			for (const Mesh& mesh : model.meshes) {
				for (const MaterialHandle& slot : mesh.materialSlots) {
					const Material* material = materialManager_->Resolve(slot);
					if (material == nullptr) {
						continue;
					}
					const LocalId localId = MakeLocalId(material->GetName());
					if (!seen.insert(localId).second) {
						continue;
					}

					MaterialRefEntry entry;
					entry.guid = modelGuid;
					entry.localId = localId;
					entry.displayName = model.name + " / " + material->GetName();
					entries.push_back(std::move(entry));
				}
			}
		}
	}

	return entries;
}

#pragma endregion

#pragma region ロード窓口

const AssetEntry* AssetDatabase::FindForLoad(const Guid& guid, AssetType expected) const {
	const AssetEntry* entry = Find(guid);
	if (entry == nullptr) {
		DebugLog::GetInstance().Log(
			LogLevel::Warn, kLogCategory,
			"未登録のGUIDです: " + guid.ToString() + " (アセットが削除されたか、再スキャンが必要です)"
		);
		return nullptr;
	}
	if (entry->type != expected) {
		DebugLog::GetInstance().Log(
			LogLevel::Warn, kLogCategory,
			std::string("アセットの種別が一致しません: ") + entry->path
			+ " (要求: " + ToString(expected) + " / 実際: " + ToString(entry->type) + ")"
		);
		return nullptr;
	}
	return entry;
}

Guid AssetDatabase::ImportAsset(const std::string& rawPath) {
	const std::string path = NormalizePath(rawPath);

	if (const Guid existing = GetGuid(path); existing.IsValid()) {
		return existing;
	}
	const AssetType type = DetectType(path);
	if (type == AssetType::Unknown) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "対象外の拡張子です: " + path);
		return Guid::Invalid();
	}
	if (!RegisterAsset(path, type)) {
		return Guid::Invalid();
	}
	return GetGuid(path);
}

ModelHandle AssetDatabase::LoadModel(const Guid& guid, LocalId localId) {
	if (localId != kSelfLocalId) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "モデルはサブアセット参照に未対応です");
		return {};
	}
	const AssetEntry* entry = FindForLoad(guid, AssetType::Model);
	if (entry == nullptr || modelManager_ == nullptr) {
		return {};
	}

	// 内蔵モデルはファイルが無い。ModelManager が起動時に作った実体をキーで引く.
	if (entry->builtin) {
		const ModelHandle handle = modelManager_->FindHandle(entry->path);
		if (!handle.IsValid()) {
			DebugLog::GetInstance().Log(
				LogLevel::Error, kLogCategory,
				"内蔵モデルが未生成です: " + entry->path + " (ModelManager::Initialize より前に解決していないか確認してください)"
			);
		}
		return handle;
	}
	// インポート設定は将来 .meta から読んで desc として渡す（今は既定値）.
	return modelManager_->Load(entry->path);
}

TextureHandle AssetDatabase::LoadTexture(const Guid& guid, LocalId localId) {
	if (localId != kSelfLocalId) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "テクスチャはサブアセット参照に未対応です");
		return {};
	}
	const AssetEntry* entry = FindForLoad(guid, AssetType::Texture);
	if (entry == nullptr || textureManager_ == nullptr) {
		return {};
	}
	return textureManager_->Load(entry->path);
}

MaterialHandle AssetDatabase::LoadMaterial(const Guid& guid, LocalId localId) {
	if (materialManager_ == nullptr) {
		return {};
	}

	// 本体（.mat ファイル）.
	if (localId == kSelfLocalId) {
		const AssetEntry* entry = FindForLoad(guid, AssetType::Material);
		if (entry == nullptr) {
			return {};
		}
		return materialManager_->LoadFromFile(entry->path);
	}

	// サブアセット（モデルに埋め込まれたマテリアル）.
	// GUID はモデルを指し、localId がその中のどれかを指す.
	const AssetEntry* entry = FindForLoad(guid, AssetType::Model);
	if (entry == nullptr || modelManager_ == nullptr) {
		return {};
	}

	const ModelHandle model = modelManager_->Load(entry->path);
	const MaterialHandle material = FindEmbeddedMaterial(model, localId);
	if (!material.IsValid()) {
		DebugLog::GetInstance().Log(
			LogLevel::Warn, kLogCategory,
			"埋め込みマテリアルが見つかりません: " + entry->path
			+ " (マテリアル名が変わった可能性があります)"
		);
	}
	return material;
}

MaterialHandle AssetDatabase::FindEmbeddedMaterial(ModelHandle model, LocalId localId) {
	if (modelManager_ == nullptr || materialManager_ == nullptr) {
		return {};
	}

	// LocalId はマテリアル名から導出されるので、名前が一致するものを探す.
	// スロット数は多くないため線形探索で十分.
	for (const Mesh& mesh : modelManager_->ResolveMeshes(model)) {
		for (const MaterialHandle& slot : mesh.materialSlots) {
			const Material* material = materialManager_->Resolve(slot);
			if (material == nullptr) {
				continue;
			}
			if (MakeLocalId(material->GetName()) == localId) {
				return slot;
			}
		}
	}
	return {};
}

#pragma endregion

} // namespace Cake
