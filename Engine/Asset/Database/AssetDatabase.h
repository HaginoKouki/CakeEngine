#pragma once
/*====================================
 *
 * プロジェクト内のアセットを一元管理する索引兼ロード窓口。
 *
 * 【役割1：索引】
 * 起動時に Assets/ 以下を再帰スキャンし、対象拡張子のファイルすべてに .meta を用意して
 * GUID とパスの相互索引を張る。シーンやプレハブは GUID だけを保存し、実行時にここで
 * パスへ解決する。これにより、アセットを移動・リネームしても参照が切れない。
 *
 * 【役割2：ロード窓口（ファサード）】
 * {GUID, LocalId} を渡すと、対応するマネージャへ委譲してハンドルを返す。
 * localId が非0の場合はサブアセット（モデルに埋め込まれたマテリアル等）を指す。
 *
 * 2つの役割はクラス内で明確に区切ってある（後で分割しやすくするため）。
 * パスは常にスラッシュ区切りで正規化して保持する（既存マネージャの索引と揃えるため）。
 *
 * ====================================*/
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Foundation/Identity/LocalId.h"

#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"
#include "Engine/Asset/Texture/TextureHandle.h"

namespace Cake {

class TextureManager;
class MaterialManager;
class ModelManager;

// 拡張子から判定するアセットの種類。インスペクタでの絞り込みに使う.
enum class AssetType {
	Unknown,
	Model,
	Texture,
	Material,
	Scene,
};

struct AssetEntry {
	Guid guid;
	std::string path; // スラッシュ区切りの相対パス（例: "Assets/Models/bunny.obj"）.
	AssetType type = AssetType::Unknown;

	// 実ファイルを持たない内蔵アセット（基本図形など）。
	// true のときは path がファイルパスではなくマネージャのキャッシュキーを指す.
	bool builtin = false;
};

// インスペクタのマテリアル選択に並べる候補。
// .mat ファイル本体と、モデルに埋め込まれたマテリアルの両方を表す.
struct MaterialRefEntry {
	Guid guid;
	LocalId localId = kSelfLocalId;
	std::string displayName;
};

class AssetDatabase {
private:
	std::unordered_map<Guid, AssetEntry> byGuid_;
	std::unordered_map<std::string, Guid> byPath_;

	std::string rootPath_ = "Assets";

	// ファサードの委譲先。所有はしない.
	TextureManager* textureManager_ = nullptr;
	MaterialManager* materialManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;

	// 1ファイルを索引へ登録する（.meta の読み込みまたは新規発行を伴う）.
	bool RegisterAsset(const std::string& path, AssetType type);

	// 実ファイルを持たない内蔵アセットを索引へ登録する。Rescan の先頭で呼ぶ.
	void RegisterBuiltinAssets();

	// GUID から実体を引く。未登録・種別違いならログを出して nullptr.
	const AssetEntry* FindForLoad(const Guid& guid, AssetType expected) const;

	// モデルに埋め込まれたマテリアルを LocalId で探す.
	MaterialHandle FindEmbeddedMaterial(ModelHandle model, LocalId localId);

public:
	void Initialize(
		const std::string& rootPath,
		TextureManager* textureManager,
		MaterialManager* materialManager,
		ModelManager* modelManager
	);

	// ===== 役割1：索引 =====

	// 索引を捨てて再スキャンする。エディタからの手動更新用.
	void Rescan();

	// 見つからなければ nullptr.
	const AssetEntry* Find(const Guid& guid) const;
	const AssetEntry* FindByPath(const std::string& path) const;

	// パスから GUID を引く。見つからなければ無効な Guid.
	Guid GetGuid(const std::string& path) const;

	// 指定した種類のアセット一覧（パス順にソート済み）。インスペクタの選択UI用.
	// 読み込み済みかどうかに関わらず、プロジェクト内の全アセットが並ぶ.
	std::vector<const AssetEntry*> GetEntriesOfType(AssetType type) const;

	// マテリアル参照の候補一覧。.mat ファイルに加え、読み込み済みモデルに
	// 埋め込まれたマテリアルも含む。
	// 【制限】未読み込みのモデルの中身は分からないため、そこは列挙されない.
	std::vector<MaterialRefEntry> GetSelectableMaterials();

	size_t GetCount() const { return byGuid_.size(); }

	// ===== 役割2：ロード窓口 =====
	// 未登録・種別違い・マネージャ未設定のいずれでも、無効ハンドルを返す（落とさない）.

	// 実行中に生成されたファイルを1件だけ索引へ追加する（Rescan を丸ごと回さずに済む）。
	// 既に登録済みならその GUID を返す。対象外拡張子・.meta 発行失敗なら無効な Guid.
	Guid ImportAsset(const std::string& path);

	// モデルとテクスチャはサブアセット未対応（localId が非0なら警告して無効を返す）.
	ModelHandle LoadModel(const Guid& guid, LocalId localId = kSelfLocalId);
	TextureHandle LoadTexture(const Guid& guid, LocalId localId = kSelfLocalId);

	// localId が 0 なら .mat ファイル、非0ならモデル内の埋め込みマテリアル.
	MaterialHandle LoadMaterial(const Guid& guid, LocalId localId = kSelfLocalId);

	// ===== 補助 =====

	// 拡張子からアセット種別を判定する（大文字小文字は区別しない）.
	static AssetType DetectType(const std::string& path);

	// バックスラッシュをスラッシュへ揃える.
	static std::string NormalizePath(const std::string& path);
};

} // namespace Cake
