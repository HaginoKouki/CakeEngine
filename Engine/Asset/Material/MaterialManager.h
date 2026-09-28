#pragma once
/*====================================
 *
 * マテリアル実体の生成・キャッシュ・所有を行うマネージャ。
 *
 * 【ハンドル方式】
 * 実体は pool_（std::vector）に連続配置し、外へは MaterialHandle（index+generation）
 * を払い出す。呼び出し側は Material* を直接持たず、ハンドルを持ち回って
 * Resolve(handle) 経由で実体にアクセスする。
 *   - 連続配置：描画ループ等の走査がキャッシュに優しい.
 *   - index 参照：pool_ が再確保で引っ越してもハンドルは無効化されない.
 *   - generation：将来アンロードでスロットを再利用したとき、古いハンドルを検出する（予約）.
 *
 * 名前→ハンドルの索引（byName_）は「同名マテリアルの重複生成検出・名前引き」用に残す。
 *
 * ====================================*/

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <d3d12.h>

#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialHandle.h"

namespace Cake {

class ShaderLibrary;
class TextureManager;

class MaterialManager {
private:
	ID3D12Device* device_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	const ShaderLibrary* shaderLibrary_ = nullptr;

	// マテリアル実体を連続配置で所有する.
	// Material はムーブ可（cb_/mappedCB_ はムーブしても Map は生き続ける）なので vector に入れられる.
	std::vector<Material> pool_;
	// pool_ と同じ添字。スロットの世代（将来アンロードでインクリメント）.
	std::vector<uint32_t> generations_;
	// 名前 → ハンドル。重複生成検出・名前引き用の索引.
	std::unordered_map<std::string, MaterialHandle> byName_;
	// .mat のパス → ハンドル。ファイル由来マテリアルの重複ロード検出用.
	std::unordered_map<std::string, MaterialHandle> byPath_;

	MaterialHandle errorMaterial_;

public:
	void Initialize(ID3D12Device* device, TextureManager* textureManager, const ShaderLibrary* shaderLibrary);

	// マテリアルを生成し、ハンドルを払い出す。名前が衝突する場合は "Name.1" のようにリネームされる.
	// origin は既定で Runtime。モデルローダーは Embedded を明示して呼ぶこと.
	MaterialHandle CreateMaterial(const std::string& materialName, const std::string& shaderName,  MaterialOrigin origin = MaterialOrigin::Runtime);
	// .mat ファイルからマテリアルを生成し、ハンドルを払い出す。
	// 読み込み済みならキャッシュを返す。失敗時は無効ハンドル.
	MaterialHandle LoadFromFile(const std::string& filePath);

	// --- ハンドル解決（正）---
	// 有効なら実体を返す。無効・破棄済みハンドルなら nullptr.
	Material* Resolve(MaterialHandle handle);
	const Material* Resolve(MaterialHandle handle) const;
	Material* ResolveOrError(MaterialHandle handle);
	const Material* ResolveOrError(MaterialHandle handle) const;

	// 名前からハンドルを引く（見つからなければ無効ハンドル）.
	MaterialHandle FindHandle(const std::string& materialName) const;

	/* --- 保存・読み込み ---*/
	// 未保存の変更を持つ .mat 由来マテリアルをすべて上書き保存する。保存した件数を返す.
	// Ctrl+S から呼ぶ想定。保存先を持たないマテリアルは対象外.
	size_t SaveDirtyMaterials();
	// 未保存の変更が1つでもあるか（メニューバーの "*" 表示用）.
	bool HasDirtyMaterials() const;

		// 新しい .mat アセットを作り、ファイルへ書き出してハンドルを返す。
	// path は AssetDatabase のルート（Assets）配下であること。索引に載らないと参照できない.
	// 【注意】索引への登録は行わない。呼び出し側で AssetDatabase::ImportAsset すること
	// （AssetDatabase → MaterialManager の依存があるため、逆向きに参照できない）.
	MaterialHandle CreateAsset(const std::string& materialName, const std::string& shaderName, const std::string& path);
	// source の内容を新しい .mat として複製する。モデル既定（Embedded）を編集したいときの逃げ道.
	MaterialHandle CreateAssetFrom(MaterialHandle source, const std::string& path);

	// 登録済みマテリアル名の一覧（UI用、ソート済み）.
	std::vector<std::string> GetAllNames() const;

	// エディタ等がプール全体を走査するための読み取りアクセス.
	const std::vector<Material>& GetPool() const { return pool_; }

	MaterialHandle GetErrorMaterial() { return errorMaterial_; }

private:
	// pool_ に追加してハンドルを作る共通処理.
	MaterialHandle Emplace(Material&& material);

	// 名前の衝突を避けた新しいキーを作る.
	// （"Mat" → "Mat.1" → "Mat.2" ...）.
	std::string GetNewKeyName(const std::string& cacheKey) const;

	// ハンドルが今有効か（index 範囲内 かつ generation 一致）.
	bool IsAlive(MaterialHandle handle) const;
};

}	// namespace Cake
