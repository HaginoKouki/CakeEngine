#include "OpaquePass.h"

#include "Engine/Render/Renderer.h"
#include "Engine/Scene/System/Render/RenderSystem.h"

namespace Cake {

void OpaquePass::Execute(RenderContext& ctx) {
	DrawMeshRenderers(*ctx.scene, *ctx.renderer, *ctx.view);
}

} // namespace Cake
