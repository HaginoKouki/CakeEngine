#pragma once
/*====================================
 *
 * シーン内の SpriteRenderer を集めて SpriteDrawList へ積む System。
 *
 * 描画そのものは行わない（積むだけ）。並べ替えと GPU への描画は SpritePass の仕事。
 * 表示中のもの（visible が true で、GameObject がアクティブ）だけを積む。
 *
 * 実行前に Scene::UpdateTransforms() でワールド行列が確定していること。
 *
 * ====================================*/

namespace Cake {
class Scene;
class SpriteDrawList;

// 表示中の SpriteRenderer を、sortingOrder と組で out へ積む.
void CollectScreenSprites(Scene& scene, SpriteDrawList& out);

} // namespace Cake
