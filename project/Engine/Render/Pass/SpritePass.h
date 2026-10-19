#pragma once
/*====================================
 *
 * 画面に描くスプライトを、基準解像度のピクセル座標で描くパス。
 * 画面に重ねるので、パス列の最後に置く。
 * options->drawScreenSprites が false のとき（Scene ビュー）は何もしない。
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
