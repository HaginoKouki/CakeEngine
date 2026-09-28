#include "Sprite.h"

namespace Cake {

void Sprite::Initialize(TextureManager* textureManager, MaterialManager* materialManager, const std::string& name) {
	textureManager_ = textureManager;
	materialManager_ = materialManager;

	// テクスチャと色をスプライトごとに持たせたいので、専用マテリアルを1枚確保する.
	material_ = materialManager_->CreateMaterial(name, "Sprite");

	Material* mat = materialManager_->Resolve(material_);
	if (mat == nullptr) {
		return;
	}
	mat->SetTexture(mat->GetShader()->textures[0].name, textureManager_->GetDefaultTexture());
	mat->Apply();
}

SpriteTransform Sprite::BuildTransform(float screenWidth, float screenHeight) const {
	SpriteTransform transform{};
	transform.position = position_;
	transform.offset = offset_;
	transform.rotation = rotation_;
	transform.aspect = screenWidth / screenHeight;
	// ピクセルサイズ×倍率を画面比(0..1)へ変換する. VSはこの単位でしか矩形を作れない.
	transform.size = {
		sourceSize_.x * scale_.x / screenWidth,
		sourceSize_.y * scale_.y / screenHeight
	};
	return transform;
}

void Sprite::SetTexture(const TextureHandle& texture) {
	Material* mat = materialManager_->Resolve(material_);
	if (mat == nullptr) {
		return;
	}
	const std::string& slot = mat->GetShader()->textures[0].name;
	if (mat->GetTexture(slot) != texture) {
		mat->SetTexture(slot, texture);
	}

	// 倍率の基準になるので、テクスチャのピクセルサイズを控える.
	const TextureData* data = textureManager_->ResolveOrError(texture);
	textureSize_ = {static_cast<float>(data->textureWidth), static_cast<float>(data->textureHeight)};
	UpdateSourceSize();
}

void Sprite::SetUVRect(const Vector4& uvRect) {
	Material* mat = materialManager_->Resolve(material_);
	if (mat == nullptr) {
		return;
	}
	uvRect_ = uvRect;
	mat->SetFloat4("uvRect", uvRect);
	mat->Apply();
	UpdateSourceSize(); // 切り出すと実サイズも変わる.
}

void Sprite::SetColor(const Vector4& color) {
	Material* mat = materialManager_->Resolve(material_);
	if (mat == nullptr) {
		return;
	}
	mat->SetColor("color", color);
	mat->Apply();
}

} // namespace Cake
