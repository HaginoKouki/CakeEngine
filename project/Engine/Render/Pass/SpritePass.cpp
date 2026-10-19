#include "SpritePass.h"

#include "Engine/Render/Renderer.h"
#include "Engine/Scene/System/Render/RenderSystem.h"

namespace Cake {

void SpritePass::Execute(RenderContext& ctx) {
	if (ctx.options != nullptr && ctx.options->drawScreenSprites == false) {
		return;
	}
}

} // namespace Cake
