#pragma once
/*====================================
 *
 * モデルを描画するコンポーネント。
 * データのみを持ち、実際の描画は外側（RenderSystem）が全 MeshRenderer を走査して行う。
 *
 * model は AssetRef で参照する。保存されるのは GUID だけで、handle は
 * シーン読み込み後に Resolve() で埋める。
 * 内蔵の基本図形（Builtin/Cube など）も固定GUIDのアセットとして索引に載っているので、
 * OBJ と同じ枠でそのまま選べる（PrimitiveShape.h を参照）。
 *
 * material は「このオブジェクトだけの上書き」。未設定ならモデル側の既定
 * マテリアルで描く。設定するとモデル内の全サブメッシュがそれ1枚で描かれる。
 * モデルアセットそのもののマテリアル割り当てを変えたい場合は、
 * こちらではなく ModelEditor（MaterialSlot）を使うこと。あちらは共有アセットを
 * 書き換えるので、同じモデルを使う全オブジェクトに影響する。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct MeshRendererComponent {
	AssetRef<ModelHandle> model;

	// 上書き用マテリアル。未設定ならモデル既定のマテリアルを使う.
	// 【制限】スロット単位ではなくモデル全体に1枚。可変長配列は
	// リフレクションが未対応のため、必要になったら PropertyType の拡張が先.
	AssetRef<MaterialHandle> material;

	bool visible = true;
};

CAKE_REFLECT(MeshRendererComponent)
CAKE_PROPERTY(model, "Model")
CAKE_PROPERTY(material, "Material")
CAKE_PROPERTY(visible, "Visible")
CAKE_REFLECT_END()

} // namespace Cake
