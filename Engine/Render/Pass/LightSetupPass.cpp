#include "LightSetupPass.h"

#include "Engine/Render/Renderer.h"
#include "Engine/Render/Light.h"
#include "Engine/Scene/System/LightSystem.h"

namespace Cake {

void LightSetupPass::Execute(RenderContext& ctx) {
	DirectionalLight light{};
	CollectDirectionalLight(*ctx.scene, light);
	ctx.renderer->SetLight(light);
}

} // namespace Cake
