#pragma once
/*====================================
 *
 * パーティクルの描き方を持つコンポーネント（Unity の ParticleSystemRenderer に相当）。
 * ParticleSystemComponent と組で使う。
 *
 * mesh は粒1つ分の形、material は粒の描き方（Particle シェーダーのマテリアルを想定）。
 * どちらも AssetRef で参照し、保存されるのは GUID だけ（MeshRendererComponent と同じ）。
 *
 * データのみを持つ。描画は描画パス側で行う方針。
 *
 * 構造体の直後に反映ブロックを置いてある。メンバを増やしたら CAKE_PROPERTY も足すこと。
 *
 * ====================================*/
#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct ParticleRendererComponent {
	// 粒1つ分の形.
	AssetRef<ModelHandle> mesh;

	// 粒の描き方。Particle シェーダーのマテリアルを想定.
	AssetRef<MaterialHandle> material;
};

CAKE_REFLECT(ParticleRendererComponent)
CAKE_PROPERTY(mesh, "Mesh")
CAKE_PROPERTY(material, "Material")
CAKE_REFLECT_END()

} // namespace Cake
