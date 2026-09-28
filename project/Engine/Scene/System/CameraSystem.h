#pragma once
/*====================================
 *
 * シーン内の CameraComponent を走査し、描画に使う CameraView を組み立てる System。
 *
 * 有効なカメラのうち priority が最小のものを1つだけ採用する。
 * 姿勢は Transform から取るため、Scene::UpdateTransforms() が済んでいること。
 * アスペクト比は引数で受け取った出力先の解像度から決まる。
 *
 * ====================================*/
#include <cstdint>

namespace Cake {

class Scene;
struct CameraView;

// 採用するカメラが見つかれば out へ書いて true。1つも無ければ out を触らず false.
bool CollectMainCamera(Scene& scene, uint32_t targetWidth, uint32_t targetHeight, CameraView& out);

} // namespace Cake
