#pragma once
/*====================================
 *
 * 背景の Skybox を描くパス。
 *
 * 最背面を埋めるため、不透明描画（OpaquePass）より前に置く。
 * 使用するマテリアルは Renderer が初期化時に生成した専用のものを使う。
 *
 * ====================================*/
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class SkyboxPass : public RenderPass {
public:
	const char* GetName() const override { return "Skybox"; }
	void Execute(RenderContext& ctx) override;
};

} // namespace Cake
