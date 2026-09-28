#include "Asset.h"

namespace Cake {

void Asset::Initialize(const AssetInitializeDesc& desc) {
	// 抜けがあると nullptr のまま各マネージャへ流れ、原因の遠い場所で落ちる.
	assert(desc.IsValid() && "AssetInitDesc に未設定の項目があります");

	textureManager_.Initialize(desc.device, desc.srvHeap, desc.commandManager);
	materialManager_.Initialize(desc.device, &textureManager_, desc.shaderLibrary);
	modelManager_.Initialize(desc.device, &textureManager_, &materialManager_);

	// AssetDatabase（rootPath 以下をスキャンして索引を作る）.
	assetDatabase_.Initialize(desc.rootPath, &textureManager_, &materialManager_, &modelManager_);
}

} // namespace Cake
