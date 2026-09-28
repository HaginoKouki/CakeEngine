#pragma once
/*====================================
 *
 * シーン内の可視な MeshRenderer をすべて描くパス。
 *
 * 実行前に Scene::UpdateTransforms() でワールド行列が確定していること
 * （SceneManager::Update が毎フレーム呼んでいる）。
 *
 * 【将来】
 * 半透明を分離する際は、このパスを不透明のみに絞り、
 * TransparentPass を後段へ追加する。
 *
 * ====================================*/
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class OpaquePass : public RenderPass {
public:
	const char* GetName() const override { return "Opaque"; }
	void Execute(RenderContext& ctx) override;
};

} // namespace Cake
