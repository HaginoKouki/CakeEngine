#pragma once
/*====================================
 *
 * 画面にスプライトを描くためのコンポーネント。
 * データのみを持ち、実際の描画は SpritePass が SpriteSystem で集めて行う。
 *
 * 【位置・回転・拡大率は Transform から取る】
 * 単位は基準解像度（ProjectSettings の referenceWidth / referenceHeight）のピクセルで、
 * 左上が原点、y は下向き。使うのは translate.xy、rotate.z、scale.xy だけ。
 * 3D のオブジェクトの子にすると、親のワールド行列（メートル単位）が掛かって位置が崩れる。
 *
 * 【テクスチャ】
 * texture が未設定なら白い四角を描く（color で色を付けて使える）。
 * 参照先を設定したのに見つからないとき（ファイルを消したなど）は、
 * エラー用のチェッカー柄で描いて、欠けていることが見えるようにする。
 *
 * 【色と表示サイズ】
 * color はテクスチャの色に掛ける色。w（不透明度）を下げると半透明になる。
 * useTextureSize が true でテクスチャがあるときは、テクスチャのピクセルサイズ × uvRect の幅と高さで描く。
 * それ以外（false のとき、テクスチャが未設定か見つからないとき）は size（ピクセル）で描く。
 * どちらにも Transform の scale.xy がさらに掛かる（100 ピクセルで scale が 2 なら、200 ピクセル）。
 * useTextureSize を false にしても、テクスチャのサイズを size へ写すことはしない。
 *
 * 【基準点・反転・切り出し】
 * pivot は授業資料のアンカーポイントに当たり、画像のどこを Transform の位置に合わせるかを 0〜1 で表す。
 * (0, 0) が左上で、回転と拡大縮小もこの点を中心に行う。
 * uvRect は切り出す範囲で、xy が左上、zw が幅と高さ（どれも UV の 0〜1）。
 * flipX と flipY は UV を入れ替えて反転するので、切り出した範囲の中で反転する。
 * uvOffset は UV に足すずらし量。サンプラーが WRAP なので、はみ出した分は反対側から出る。
 * 切り出したまま流すと、画像全体の中で範囲がずれるので、隣のコマが見える。
 *
 * 【描く順】
 * sortingOrder の小さい順に描くので、大きいほど手前に出る。
 * 同じ値どうしはプールの並び順になり、作った順になるとは限らない。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Texture/TextureHandle.h"

namespace Cake {

struct SpriteRendererComponent {
	bool visible = true;

	AssetRef<TextureHandle> texture; // 未設定なら白い四角で描く.
	Vector4 color = Vector4::One;    // テクスチャの色に掛ける色.

	Vector2 pivot{0.5f, 0.5f}; // 基準点（アンカーポイント）。0〜1 で、(0, 0) が左上.

	bool useTextureSize = true;             // true でテクスチャがあれば、テクスチャのサイズで描く.
	Vector2 size{100.0f, 100.0f};           // 表示サイズ（ピクセル）。テクスチャのサイズを使わないときの大きさ.
	Vector4 uvRect{0.0f, 0.0f, 1.0f, 1.0f}; // 切り出す範囲（UV）。xy が左上、zw が幅と高さ.
	Vector2 uvOffset{0.0f, 0.0f};           // UV のずらし量。WRAP で繰り返す.

	bool flipX = false; // 左右反転（切り出した範囲の中で反転する）.
	bool flipY = false; // 上下反転（切り出した範囲の中で反転する）.

	int sortingOrder = 0; // 大きいほど手前に描く.
};

CAKE_REFLECT(SpriteRendererComponent)
CAKE_PROPERTY(visible, "Visible")
CAKE_PROPERTY(texture, "Texture")
CAKE_PROPERTY_COLOR(color, "Color")
CAKE_PROPERTY_CLAMP(pivot, "Pivot", 0.0f, 1.0f)
CAKE_PROPERTY(useTextureSize, "Use Texture Size")
CAKE_PROPERTY_MIN(size, "Size", 0.0f)
CAKE_PROPERTY_CLAMP(uvRect, "UV Rect", 0.0f, 1.0f)
CAKE_PROPERTY_CLAMP(uvOffset, "UV Offset", 0.0f, 1.0f)
CAKE_PROPERTY(flipX, "Flip X")
CAKE_PROPERTY(flipY, "Flip Y")
CAKE_PROPERTY(sortingOrder, "Sorting Order")
CAKE_REFLECT_END()

} // namespace Cake
