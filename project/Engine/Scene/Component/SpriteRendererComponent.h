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
 * 【描く順】
 * sortingOrder の小さい順に描くので、大きいほど手前に出る。
 * 同じ値どうしはプールの並び順になり、作った順になるとは限らない。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct SpriteRendererComponent {
	bool visible = true;
	int sortingOrder = 0;
};

CAKE_REFLECT(SpriteRendererComponent)
CAKE_PROPERTY(visible, "Visible")
CAKE_PROPERTY(sortingOrder, "Sorting Order")
CAKE_REFLECT_END()

} // namespace Cake
