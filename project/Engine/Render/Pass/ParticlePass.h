#pragma once
/*====================================
 *
 * シーン内の ParticleSystem の粒を描くパス。
 * 粒は常にカメラを向く板として描く。向きは ctx.view の rotationMatrix から作るため、視点ごとに結果が変わる。
 *
 * 粒の状態は Scene の ParticleStore から読むだけで、動かさない（更新は UpdateParticleSystems）。
 * 1フレームにゲームビューとシーンビューの2回実行されるため、ここで状態を変えてはいけない。
 *
 * 半透明にする想定なので、OpaquePass の後に置く。
 *
 * ====================================*/
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class ParticlePass : public RenderPass {
public:
	const char* GetName() const override { return "Particle"; }
	void Execute(RenderContext& ctx) override;
};

} // namespace Cake
