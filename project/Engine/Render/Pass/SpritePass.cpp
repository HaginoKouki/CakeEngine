#include "SpritePass.h"

#include "Engine/Render/Renderer.h"
#include "Engine/Scene/System/Render/SpriteSystem.h"

namespace Cake {

void SpritePass::Execute(RenderContext& ctx) {
	if (ctx.options != nullptr && ctx.options->drawScreenSprites == false) {
		return;
	}

	drawList_.Clear();
	CollectScreenSprites(*ctx.scene, drawList_);
	drawList_.SortByOrder();

	for (auto& entry : drawList_.GetEntries()) {
		ctx.renderer->DrawSprite(entry.item);
	}
}

} // namespace Cake
