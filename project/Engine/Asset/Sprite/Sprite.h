#pragma once

#include <algorithm>
#include <string>

#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Asset/Texture/TextureHandle.h"
#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Material/MaterialManager.h"

namespace Cake {

// VS(b0)へ毎描画積む配置情報。HLSL側のSpriteTransformと1バイトも違ってはいけない.
struct SpriteTransform {
	Vector2 position{0.5f, 0.5f}; // 画面上の位置(0..1). 左上が(0,0).
	Vector2 size{0.2f, 0.2f};     // 画面比での大きさ.
	Vector2 offset{0.5f, 0.5f};   // 位置の基準点を画像上の0..1のどこに置くか.
	float rotation = 0.0f;        // ラジアン.
	float aspect = 1.0f;          // Rendererが描画時に埋めるので触らない.
};
static_assert(sizeof(SpriteTransform) == 32, "HLSLのcbufferパッキングと不一致");

class Sprite {
private:
	TextureManager* textureManager_ = nullptr;
	MaterialManager* materialManager_ = nullptr;

	MaterialHandle material_;

	Vector2 position_{0.5f, 0.5f}; // 画面上の位置(0..1). 左上が(0,0).
	Vector2 scale_{1.0f, 1.0f};    // 元テクスチャの何倍か. 1.0でドット等倍.
	Vector2 offset_{0.5f, 0.5f};   // 位置の基準点を画像上の0..1のどこに置くか.
	float rotation_ = 0.0f;        // ラジアン.

	Vector2 textureSize_{1.0f, 1.0f};        // 現在のテクスチャのピクセルサイズ.
	Vector4 uvRect_{0.0f, 0.0f, 1.0f, 1.0f}; // 切り出し範囲. sourceSize_の計算に使う.
	Vector2 sourceSize_{1.0f, 1.0f};         // 実際に描く領域のピクセルサイズ.

public:
	void Initialize(TextureManager* textureManager, MaterialManager* materialManager, const std::string& name);

	// 画面サイズを与えて、GPUへ送る配置情報を組み立てる.
	SpriteTransform BuildTransform(float screenWidth, float screenHeight) const;

	void SetPosition(const Vector2& p) { position_ = p; }
	// 元テクスチャに対する倍率で大きさを指定する. 負の値で反転する.
	void SetScale(const Vector2& s) { scale_ = s; }
	// ピクセル単位で大きさを指定する（内部で倍率に変換する）.
	void SetSizeInPixels(const Vector2& pixels) { scale_ = {pixels.x / sourceSize_.x, pixels.y / sourceSize_.y}; }
	void SetOffset(const Vector2& o) { offset_ = {std::clamp(o.x, 0.0f, 1.0f), std::clamp(o.y, 0.0f, 1.0f)}; }
	void SetRotation(float radian) { rotation_ = radian; }

	// --- 見た目（マテリアルを書き換える）---
	void SetTexture(const TextureHandle& texture);
	void SetColor(const Vector4& color);
	void SetUVRect(const Vector4& uvRect);

	// Getter.
	const Vector2& GetPosition() const { return position_; }
	const Vector2& GetScale() const { return scale_; }
	const Vector2& GetOffset() const { return offset_; }
	const float& GetRotation() const { return rotation_; }
	MaterialHandle GetMaterial() const { return material_; }
	const Vector2& GetSourceSize() const { return sourceSize_; }

private:
	// テクスチャサイズとUV切り出し範囲から、描画領域のピクセルサイズを更新する.
	void UpdateSourceSize() {
		sourceSize_ = {textureSize_.x * uvRect_.z, textureSize_.y * uvRect_.w};
	}
};

} // namespace Cake
