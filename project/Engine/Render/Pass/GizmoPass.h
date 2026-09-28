#pragma once
/*====================================
 *
 * コライダーやカメラなど、通常は絵に出ない情報を線で重ねるパス。
 *
 * 実体の上に描くため、必ず OpaquePass より後ろに置く。
 * options->gizmos が nullptr のときは何もしないので、
 * 同じパス列のままゲームビューにはギズモが出ない。
 *
 * 【DrawList をメンバで持つ理由】
 * 毎フレーム作り直すと vector の確保が繰り返される。
 * 使い回して Clear() だけすれば、2フレーム目以降は確保が起きない。
 *
 * ====================================*/
#include "Engine/Render/Gizmo/GizmoDrawList.h"
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class GizmoPass : public RenderPass {
private:
	GizmoDrawList drawList_;

public:
	const char* GetName() const override { return "Gizmo"; }
	void Execute(RenderContext& ctx) override;
};

} // namespace Cake
