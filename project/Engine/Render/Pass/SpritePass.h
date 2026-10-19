#pragma once
/*====================================
 *
 * 画面座標で描くこと。
 * 画面に重ねるので、パス列の最後に置くこと。
 * drawScreenSpritesがfalseのときは何もしないこと。
 *
 * ====================================*/
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class SpritePass : public RenderPass {
public:
	const char* GetName() const override { return "Sprite"; }
	void Execute(RenderContext& ctx) override;
};

} // namespace Cake
