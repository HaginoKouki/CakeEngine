#include "GameModule.h"

#include "Engine/Graphics/Shader/ShaderParam.h"
#include "Engine/Graphics/Shader/ShaderDefinition.h"
#include "Engine/Graphics/Shader/ShaderLibrary.h"
#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"

#include "Game/Components/RotatorComponent.h"
#include "Game/Components/Player/PlayerComponent.h"

namespace Game {

void RegisterGameComponents() {
	Cake::TypeRegistry& registry = Cake::TypeRegistry::GetInstance();
	// ユーザーのコンポーネントはここに追加する.
	registry.Register<RotatorComponent>(Cake::kDefaultUpdatePriority);
	registry.Register<PlayerComponent>(Cake::kDefaultUpdatePriority);
}
void RegisterGameShaders(Cake::ShaderLibrary& shaderLibrary) {
	/*
	* Object3D
	———————————————*/
	Cake::ShaderDefinition object3d;
	object3d.name = "Object3D";
	object3d.vsPath = L"Assets/Shaders/Object3D/Object3d.VS.hlsl";
	object3d.psPath = L"Assets/Shaders/Object3D/Object3d.PS.hlsl";
	object3d.params = {
		Cake::ShaderParamDesc{
			.name = "color",
			.type = Cake::ShaderParamType::Color,
			.defaultValue = {1, 1, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
		Cake::ShaderParamDesc{
			.name = "enableLighting",
			.type = Cake::ShaderParamType::Int,
			.defaultValue = {1, 0, 0, 0},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		}
	};
	object3d.textures = {
		Cake::TextureSlotDesc{
			.name = "albedo",
			.registerIndex = 0
		}, // t0.
	};
	shaderLibrary.Register(object3d);
	/*
	* Particle
	———————————————*/
	Cake::ShaderDefinition particle;
	particle.name = "Particle";
	particle.vsPath = L"Assets/Shaders/Object3D/Particle.VS.hlsl";
	particle.psPath = L"Assets/Shaders/Object3D/Particle.PS.hlsl";
	particle.params = {
		Cake::ShaderParamDesc{
			.name = "color",
			.type = Cake::ShaderParamType::Color,
			.defaultValue = {1, 1, 1, 1},
			.uiMin = 0.0f,
			.uiMax = 1.0f
		},
	};
	particle.textures = {
		Cake::TextureSlotDesc{
			.name = "albedo",
			.registerIndex = 0
		}, // t0.
	};
	particle.requiresInstancing = true;
	shaderLibrary.Register(particle);
}

} // namespace Game
