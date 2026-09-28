#pragma once
/*====================================
 *
 * Assetの部品をまとめて所有し、生涯を管理するクラス。
 *
 * 【依存の向き】
 * Graphicsに依存する。
 * この層より上（Render / Scene / Editor）を一切知らない。
 * 下は Graphics(RHI), Platform, Foundation。この規則が破れると層が意味を失う。
 *
 * ====================================*/

#include <string>

#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Asset/Material/MaterialManager.h"
#include "Engine/Asset/Model/ModelManager.h"

namespace Cake {

class DescriptorHeap;
class CommandManager;
class ShaderLibrary;

struct AssetInitializeDesc {
	ID3D12Device* device = nullptr;
	DescriptorHeap* srvHeap = nullptr;
	CommandManager* commandManager = nullptr;
	const ShaderLibrary* shaderLibrary = nullptr;

	// 索引を張るルートフォルダ。ここより外のファイルは GUID が振られない.
	std::string rootPath = "Assets";

	// 必須項目が埋まっているか（Initialize の先頭で検査する）.
	bool IsValid() const {
		return device != nullptr && srvHeap != nullptr && commandManager != nullptr && shaderLibrary != nullptr;
	}
};

class Asset {
	TextureManager textureManager_;
	MaterialManager materialManager_;
	ModelManager modelManager_;
	AssetDatabase assetDatabase_;

public:
	void Initialize(const AssetInitializeDesc& desc);

	TextureManager* GetTextureManager() { return &textureManager_; }
	MaterialManager* GetMaterialManager() { return &materialManager_; }
	ModelManager* GetModelManager() { return &modelManager_; }
	AssetDatabase* GetAssetDatabase() { return &assetDatabase_; }
};
} // namespace Cake
