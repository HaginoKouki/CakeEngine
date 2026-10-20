#pragma once

namespace Cake {
class Scene;
class SpriteDrawList;

// 描画はせず、積むだけ
// 実行前に、Scene::UpdateTransformsでワールド行列が確定していること.
void CollectScreenSprites(Scene& scene, SpriteDrawList& out);

} // namespace Cake
