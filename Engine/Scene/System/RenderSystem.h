#pragma once
/*====================================
 *
 * シーン内の MeshRenderer をまとめて描画するSystem。
 *
 * コンポーネントは「データのみ」で自分では何もしないので、実際の処理はここが担う。
 * 型ごとのプールを順に舐めるだけなので、オブジェクトが増えても走査は連続アクセスのまま。
 *
 * 描画前に Scene::UpdateTransforms() でワールド行列が確定していること。
 * この関数は行列を計算しない（TransformComponent がキャッシュした world をそのまま使う）。
 *
 * ====================================*/
#include "Engine/Render/CameraView.h"

namespace Cake {

class Scene;
class Renderer;
class AssetDatabase;

// 可視かつ解決済みの MeshRenderer をすべて描画する.
void DrawMeshRenderers(Scene& scene, Renderer& renderer, const CameraView& cameraView);

// 未解決のアセット参照を解決する。シーン読み込み直後に1回呼ぶ.
void ResolveSceneAssets(Scene& scene, AssetDatabase& assetDatabase);

} // namespace Cake
