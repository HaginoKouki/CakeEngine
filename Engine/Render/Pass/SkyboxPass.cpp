#include "SkyboxPass.h"

#include "Engine/Render/Renderer.h"

namespace Cake {

void SkyboxPass::Execute(RenderContext& ctx) {
	ctx.renderer->DrawSkybox(*ctx.view);
}

} // namespace Cake
