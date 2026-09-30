#include "ComponentRegistration.h"

#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"

#include "Engine/Scene/Component/LightComponent.h"
#include "Engine/Scene/Component/CameraComponent.h"
#include "Engine/Scene/Component/MeshRendererComponent.h"
#include "Engine/Scene/Component/SphereColliderComponent.h"
#include "Engine/Scene/Component/BoxColliderComponent.h"
#include "Engine/Scene/Component/ParticleSystemComponent.h"
#include "Engine/Scene/Component/ParticleRendererComponent.h"

namespace Cake {

void RegisterAllComponents() {
	TypeRegistry& registry = TypeRegistry::GetInstance();

	// コンポーネントを追加したらここに1行足す.
	// 第1引数は Update の実行順（小さいほど先）。Update を持たない型では意味を持たない.
	registry.Register<LightComponent>();
	registry.Register<CameraComponent>();
	registry.Register<MeshRendererComponent>();
	registry.Register<SphereColliderComponent>();
	registry.Register<BoxColliderComponent>();
	registry.Register<ParticleSystemComponent>();
	registry.Register<ParticleRendererComponent>();

	// TransformComponent は GameObject の組み込みフィールドでプールに入らないため、ここでは登録しない.
	// インスペクタとシリアライザが個別に扱う.

	// 依存宣言の検査はここでは行わない。ゲーム側の登録が終わった後に、
	// Application が TypeRegistry::FinalizeRegistration で行う.
}

} // namespace Cake
