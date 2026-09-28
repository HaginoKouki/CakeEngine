#pragma once
/*====================================
 *
 * モデルのインポート結果を表す中間表現。Importer の出力であり Registry の入力。
 * Mesh との違いは「GPU リソースを持たないこと」で、これが分離の理由そのもの。
 *
 *   Mesh          … vertexResource / vbv / MaterialHandle を持つ。実行時専用.
 *   MeshArtifact  … 頂点配列と LocalId しか持たない。ディスクへ落とせる.
 *
 * 【ハンドルを持たせないこと】
 * マテリアルの参照は MaterialHandle ではなく LocalId で持つ。
 * ハンドルは index + generation であり、次回起動時には全く別のものを指す。
 * ディスクに落ちるデータへ混ぜると、キャッシュが効いた瞬間に壊れる。
 * LocalId は名前から導出されるので、再インポートしても同じ値になる。
 *
 * 【マテリアルは記述だけを持つ】
 * .mtl 由来のマテリアルは、ここでは「どのシェーダーで、どのテクスチャを
 * どのスロットへ」という記述に留める。Material 実体（定数バッファを含む）を
 * 作るのは MaterialRegistry の仕事。
 * テクスチャは GUID ではなく相対パスで持つ。インポート時点では参照先の
 * GUID が発行済みとは限らないため（依存アセットが後からスキャンされうる）。
 * パス → GUID の解決は読み戻し時に AssetDatabase が行う。
 *
 * ====================================*/
#include <cstdint>
#include <string>
#include <vector>

#include "Engine/Foundation/Identity/LocalId.h"
#include "Engine/Foundation/Serialize/BinaryStream.h"
#include "Engine/Asset/Model/Mesh.h" // VertexData のみ使用.

namespace Cake {

// アーティファクトの先頭に置く識別子と版数。読み戻し時に検証する.
constexpr uint32_t kModelArtifactMagic = 0x314B4D43; // 'CMK1'.
constexpr uint32_t kModelArtifactVersion = 1;

// 埋め込みマテリアルの記述。実体ではなく「作り方」を持つ.
struct MaterialArtifact {
	std::string name;         // .mtl 内の名前。LocalId の導出元でもある.
	LocalId localId = kSelfLocalId;
	std::string shaderName;   // 割り当てるシェーダー名.

	// スロット名 → テクスチャの相対パス。読み戻し時に GUID へ解決する.
	std::vector<std::pair<std::string, std::string>> texturePaths;

	void Serialize(BinaryWriter& writer) const;
	void Deserialize(BinaryReader& reader);
};

// サブメッシュの記述。GPU バッファはまだ無い.
struct SubMeshArtifact {
	std::vector<VertexData> vertices;
	uint32_t materialSlot = 0; // 親 MeshArtifact の materialSlots への添字.

	void Serialize(BinaryWriter& writer) const;
	void Deserialize(BinaryReader& reader);
};

struct MeshArtifact {
	std::string name;
	std::vector<SubMeshArtifact> subMeshes;
	std::vector<LocalId> materialSlots; // スロット番号 → 埋め込みマテリアルの LocalId.

	void Serialize(BinaryWriter& writer) const;
	void Deserialize(BinaryReader& reader);
};

struct ModelArtifact {
	std::string name;
	std::vector<MeshArtifact> meshes;
	std::vector<MaterialArtifact> materials; // このモデルに埋め込まれたマテリアル.

	// magic と version を含めて書き出す.
	void Serialize(BinaryWriter& writer) const;

	// magic / version / 読み取り範囲のいずれかが不正なら false（中身は不定）.
	// 版数が上がった古いアーティファクトもここで弾かれ、再インポートへ回る.
	bool Deserialize(BinaryReader& reader);
};

} // namespace Cake
