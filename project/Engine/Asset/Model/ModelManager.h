#pragma once
/*====================================
 *
 * 3Dモデル（OBJファイル等）の読み込みとキャッシュ管理を行うマネージャ。
 *
 * 【ハンドル方式（P-full）】
 * ModelData 実体は pool_（std::vector）に連続配置し、外へは ModelHandle を払い出す。
 * 呼び出し側は ModelData*・生 span・Mesh* を直接持たず、ハンドルを持ち回って
 * Resolve 系メソッドで実体にアクセスする。
 *   - index 参照：pool_ が再確保で引っ越してもハンドルは無効化されない.
 *   - generation：将来アンロードでスロットを再利用したとき、古いハンドルを検出する（予約）.
 *
 * パス→ハンドルの索引（byPath_）は「同一モデルの重複ロード検出・パス引き」用に残す。
 *
 * Material 実体は MaterialManager が所有し、Mesh は MaterialHandle で参照する。
 *
 * 【内蔵の基本図形】
 * Initialize で三角形・矩形・立方体・球を生成し、PrimitiveShape で引けるようにする。
 * キャッシュキーは PrimitiveShape.h の kPrimitiveShapeKeys（AssetDatabase が
 * 同じ文字列を索引上のパスとして使うので、勝手に変えないこと）。
 *
 * ====================================*/

#include <string>
#include <vector>
#include <span>
#include <unordered_map>

#include <d3d12.h>

#include "Engine/Foundation/Math/Geometry.h"
#include "Engine/Graphics/Shader/ShaderDefinition.h"
#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/Mesh.h"
#include "Engine/Asset/Model/ModelHandle.h"
#include "Engine/Asset/Model/PrimitiveShape.h"

namespace Cake {

class ShaderLibrary;
class MaterialManager;

// Manager がキャッシュする「モデルデータ」.
struct ModelData {
	std::string name;     // モデル名（ファイル名）.
	std::string filePath; // モデルファイルのフルパス.

	std::vector<Mesh> meshes; // Mesh 実体（所有）.

	// ロード時に払い出された MaterialManager のキー（リネーム後の実キー）.
	// 描画には使わない。シリアライズ・リセット・ホットリロード用の記録.
	std::vector<std::string> defaultMaterialKeys;
};

// モデル読み込み時の既定設定（ローダーを特定シェーダーから疎結合にする）.
struct ModelLoadDesc {
	std::string defaultShaderName = "Standard"; // 各マテリアルに割り当てるシェーダー名.
	bool flipHandedness = true;                 // 左手座標系へ変換＋巻き順反転（OBJ用）.
};

class ModelManager {
private:
	ID3D12Device* device_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	MaterialManager* materialManager_ = nullptr;

	// モデル実体を連続配置で所有する（ModelData はムーブ可）.
	std::vector<ModelData> pool_;
	// pool_ と同じ添字。スロットの世代（将来アンロードでインクリメント）.
	std::vector<uint32_t> generations_;
	// パス → ハンドル。重複ロード検出・パス引き用の索引.
	std::unordered_map<std::string, ModelHandle> byPath_;

	ModelHandle primitives_[kPrimitiveShapeCount]{};

	void LoadObjFile(ModelData& out, const std::string& directoryPath, const std::string& filename, const ModelLoadDesc& desc);
	// .mtl を読み、MaterialManager にマテリアルを生成させる。OBJ内の名前 → MaterialHandle を返す.
	std::unordered_map<std::string, MaterialHandle> LoadMtlFile(ModelData& out, const std::string& directoryPath, const std::string& mtlFilename, const ModelLoadDesc& desc);

	// pool_ に追加してハンドルを作る共通処理.
	ModelHandle Emplace(ModelData&& data);
	// ハンドルが今有効か（index 範囲内 かつ generation 一致）.
	bool IsAlive(ModelHandle handle) const;

public:
	void Initialize(ID3D12Device* device, TextureManager* textureManager, MaterialManager* materialManager);

	// 読み込み済みならキャッシュのハンドルを返し、未読なら読み込んでキャッシュする（Mesh は複製しない）.
	// ロードに失敗した場合も pool_ には空の ModelData が積まれ、有効なハンドルが返る（メッシュ0個）.
	ModelHandle Load(const std::string& filePath, const ModelLoadDesc* desc = nullptr);

	// --- ハンドル解決 ---
	// モデルの Mesh 群を取得する。無効ハンドルなら空 span.
	std::span<const Mesh> ResolveMeshes(ModelHandle handle) const;
	std::span<Mesh> ResolveMeshesMutable(ModelHandle handle);
	// 単一 Mesh を取得する。無効なら nullptr.
	const Mesh* ResolveMesh(MeshHandle handle) const;
	Mesh* ResolveMeshMutable(MeshHandle handle);
	// モデルのメタ情報を取得する。無効なら nullptr.
	const ModelData* Resolve(ModelHandle handle) const;

	// パスからハンドルを引く（見つからなければ無効ハンドル）.
	ModelHandle FindHandle(const std::string& filePath) const;

	// エディタ用：全モデルを走査する（読み取り）.
	const std::vector<ModelData>& GetPool() const { return pool_; }
	// エディタ用：pool_ の添字からハンドルを組む（generation を補う）.
	ModelHandle HandleFromIndex(uint32_t index) const;

	// 内蔵の基本図形モデルを取得する.
	ModelHandle GetPrimitiveModel(PrimitiveShape shape) const {
		return primitives_[static_cast<size_t>(shape)];
	}

private:
	ModelHandle CreateTriangleModel(const std::string& cacheKey, const Cake::Triangle& triangle);
	ModelHandle CreateQuadModel(const std::string& cacheKey, const Cake::Quad& quad);
	ModelHandle CreateCubeModel(const std::string& cacheKey, float sideLength);
	ModelHandle CreateSphereModel(const std::string& cacheKey, float radius, int32_t subdivision);
	ModelHandle CreateTetrahedronModel(const std::string& cacheKey, float circumRadius);
};

} // namespace Cake
