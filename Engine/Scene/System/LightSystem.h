#pragma once
/*====================================
 *
 * シーン内の LightComponent を走査し、GPU へ渡す DirectionalLight を組み立てる System。
 *
 * 照らす向きは各ライトの Transform（world 行列のローカル +Z 軸）から取る。
 * したがって Scene::UpdateTransforms() が済んでいることが前提。
 *
 * 【今は1灯のみ】
 * 有効なライトのうち priority が最小のものを1つだけ採用する。
 * 多灯対応するときは、ここを配列へ返す形に変えて Renderer 側を追随させる。
 *
 * ====================================*/
namespace Cake {

class Scene;
struct DirectionalLight;

// 採用するライトが見つかれば out へ書いて true。1つも無ければ out を触らず false.
bool CollectDirectionalLight(Scene& scene, DirectionalLight& out);

} // namespace Cake
