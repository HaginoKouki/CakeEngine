#include "GizmoPass.h"

#include "Engine/Render/Renderer.h"
#include "Engine/Scene/System/GizmoSystem.h"

namespace Cake {

void GizmoPass::Execute(RenderContext& ctx) {
	if (ctx.options == nullptr || ctx.options->gizmos == nullptr) {
		return;
	}

	drawList_.Clear();
	CollectSceneGizmos(*ctx.scene, *ctx.options->gizmos, drawList_);
	ctx.renderer->DrawGizmos(drawList_, *ctx.view);
}

} // namespace Cake
