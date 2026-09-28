#pragma once
/*====================================
 *
 * マテリアル情報をJSONファイル（.mat）に保存・読み込みするユーティリティ。
 * エディタで編集したマテリアルの設定（シェーダー、パラメータ値、テクスチャパス）を永続化し、
 * ゲーム実行時に復元する仕組みを提供する。
 *
 * ====================================*/
#include <string>
#include <vector>

#include <d3d12.h>

namespace Cake {

class Material;
class ShaderLibrary;
class TextureManager;

// マテリアル群を .mat(JSON) に保存する。成功で true.
bool SaveMaterials(
	Material& materials,
	TextureManager* textureManager,
	const std::string& path
);

// .mat(JSON) を読み込み、既存マテリアルに適用する。成功で true.
// entryName が空なら materials.GetName() をキーに使う。
// ファイルから新規生成する場合は、リネーム後の実キーとファイル内のキーが
// 食い違うため、ファイル内のキーを明示して渡すこと。
bool LoadMaterials(
	Material& materials,
	ID3D12Device* device,
	const ShaderLibrary* shaderLibrary,
	TextureManager* textureManager,
	const std::string& path,
	const std::string& entryName = ""
);

// .mat から先頭エントリの名前とシェーダー名だけを取り出す。
// マテリアルを生成する前に、必要な情報を知るための下見用。
bool PeekMaterialInfo(const std::string& path, std::string& outName, std::string& outShaderName);

}	// namespace Cake
