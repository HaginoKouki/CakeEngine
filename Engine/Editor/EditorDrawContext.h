#pragma once
/*====================================
 *
 * エディタUIの描画関数が共通で必要とする参照を1つにまとめた束。
 *
 * MaterialEditor / ModelEditor / Inspector はいずれもデバイスと各マネージャを
 * 要求するため、引数が6〜7個まで増えていた。ここへ集約して、描画関数の
 * シグネチャを (描画対象, ctx) の2引数に統一する。
 *
 * 【所有しない】
 * 中身は Editor::Initialize で1度だけ埋める。寿命は Editor と同じ。
 *
 * 【allowAssetEditing】
 * Play 中はアセットの編集・保存を止めるためのフラグ。毎フレーム Editor が更新する。
 * マテリアルの変更は PlayModeController のスナップショット対象外で Stop しても
 * 巻き戻らないため、Play 中に触らせないことで整合を保つ。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include <d3d12.h>

namespace Cake {

class ShaderLibrary;
class TextureManager;
class MaterialManager;
class ModelManager;
class AssetDatabase;

// エディタUIの描画関数が共通で必要とする参照を1つにまとめた束.
struct EditorDrawContext {
	ID3D12Device* device = nullptr;
	ShaderLibrary* shaderLibrary = nullptr;
	TextureManager* textureManager = nullptr;
	MaterialManager* materialManager = nullptr;
	ModelManager* modelManager = nullptr;
	AssetDatabase* assetDatabase = nullptr;

	bool allowAssetEditing = true;
};

} // namespace Cake

#endif // ENABLE_EDITOR
