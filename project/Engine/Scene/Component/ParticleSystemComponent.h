#pragma once
/*====================================
 *
 * パーティクルを出すためのコンポーネント（Unity の ParticleSystem に相当）。
 * データのみを持つ。粒の生成・更新・描画はコンポーネントに書かず、
 * System 側に置く方針（MeshRendererComponent と同じ）。
 *
 * mesh は粒1つ分の形、material は粒の描き方（Particle シェーダーのマテリアルを想定）、
 * color は全粒に掛ける色。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct ParticleSystemComponent {
	// 粒1つ分の形.
	AssetRef<ModelHandle> mesh;

	// 粒の描き方。Particle シェーダーのマテリアルを想定.
	AssetRef<MaterialHandle> material;

	// 全粒に掛ける色.
	Vector4 color = Vector4::One;
};

CAKE_REFLECT(ParticleSystemComponent)
CAKE_PROPERTY(mesh, "Mesh")
CAKE_PROPERTY(material, "Material")
CAKE_PROPERTY_COLOR(color, "Color")
CAKE_REFLECT_END()

} // namespace Cake
