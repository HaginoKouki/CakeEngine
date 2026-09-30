#include "SceneRenderer.h"

#include "Engine/Render/Renderer.h"
#include "Engine/Render/Pass/LightSetupPass.h"
#include "Engine/Render/Pass/SkyboxPass.h"
#include "Engine/Render/Pass/OpaquePass.h"
#include "Engine/Render/Pass/ParticlePass.h"
#include "Engine/Render/Pass/GizmoPass.h"
#include "Engine/Scene/System/CameraSystem.h"

namespace Cake {

void SceneRenderer::Initialize(Renderer* renderer) {
	renderer_ = renderer;

	// パス列。この並びがそのまま描画順になる.
	// LightSetup は後続が使うライトを用意するため必ず先頭。
	// Skybox は最背面を埋めるため Opaque より前に置く.
	passes_.clear();
	passes_.push_back(std::make_unique<LightSetupPass>());
	passes_.push_back(std::make_unique<SkyboxPass>());
	passes_.push_back(std::make_unique<OpaquePass>());
	passes_.push_back(std::make_unique<ParticlePass>());
	passes_.push_back(std::make_unique<GizmoPass>());
}

void SceneRenderer::Render(Scene& scene, const CameraView& view, const RenderViewOptions& options) {
	RenderContext ctx{&scene, &view, renderer_, &options};
	
	for (auto& pass : passes_) {
		pass->Execute(ctx);
	}
}

bool SceneRenderer::TryMakeMainCameraView(
	Scene& scene,
	uint32_t targetWidth,
	uint32_t targetHeight,
	CameraView& out
) {
	return CollectMainCamera(scene, targetWidth, targetHeight, out);
}

} // namespace Cake
