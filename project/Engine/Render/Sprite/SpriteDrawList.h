#pragma once
/*====================================
 *
 * 画面に描くスプライトを、描く順に並べるための入れ物。
 * SpriteSystem（Scene 層）が積み、SpritePass が並べ替えて Renderer に渡す。
 *
 * 【DrawList をメンバで持つ理由】
 * GizmoDrawList と同じく、パスが保持して毎フレーム Clear() で使い回す。
 * 2フレーム目以降は vector の確保が起きない。
 *
 * ====================================*/
#include <span>
#include <vector>

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Asset/Texture/TextureHandle.h"

namespace Cake {

// スプライト1枚分の描画データ。Scene 層の System が SpriteDrawList に積み、SpritePass が DrawSprite に渡す.
struct SpriteDrawItem {
	Matrix4x4 world = Matrix4x4::Identity; // ピクセル単位の配置（Transform のワールド行列）.
	TextureHandle texture{};               // 無効なら既定テクスチャで描く.
	bool isTextureMissing = false;         // 参照先を設定したのに見つからない。true ならエラー用テクスチャで描く.
	Vector4 color = Vector4::One;
	Vector4 uvRect{0.0f, 0.0f, 1.0f, 1.0f};
	Vector2 size{100.0f, 100.0f}; // ピクセル。useTextureSize が true でテクスチャがあるときは使わない.
	Vector2 pivot{0.5f, 0.5f};
	Vector2 uvOffset{0.0f, 0.0f};
	bool flipX = false;
	bool flipY = false;
	bool useTextureSize = false;
};

class SpriteDrawList {
public:
	// 並べ替えのキーと描画データの組。sortingOrder は Renderer には渡さない.
	struct Entry {
		int sortingOrder = 0;
		SpriteDrawItem item;
	};

private:
	std::vector<Entry> entries_;

public:
	void Clear() { entries_.clear(); }
	void Add(const SpriteDrawItem& item, int sortingOrder);
	// sortingOrder の小さい順に並べる。同じ値どうしの並びは保つ（std::stable_sort）.
	void SortByOrder();

	std::span<const Entry> GetEntries() const { return entries_; }
};

} // namespace Cake
