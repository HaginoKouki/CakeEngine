#pragma once
/*====================================
 *
 * シーン内の平行光源を収集し、Renderer へバインドするパス。
 *
 * 自身は何も描かないが、後続のパスが正しいライトで描くための前提を作る。
 * そのため必ずパス列の先頭に置くこと。順序を誤ると1フレーム前のライトで描かれる。
 *
 * ====================================*/
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class LightSetupPass : public RenderPass {
public:
	const char* GetName() const override { return "LightSetup"; }
	void Execute(RenderContext& ctx) override;
};

} // namespace Cake
