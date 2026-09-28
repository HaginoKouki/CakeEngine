#pragma once
/*====================================
 *
 * 1回の描画パスで使う「視点」を表す値。
 *
 * Renderer はこれだけを受け取り、Camera も CameraComponent も GameObject も知らない。
 * ゲームのカメラもエディタのデバッグカメラも、最終的にこの形になって Renderer へ渡る。
 *
 * 射影行列は出力先の解像度から作るため、アスペクト比はどこにも保存しない。
 *
 * ====================================*/
#include <cstdint>

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Vector.h"

namespace Cake {

struct CameraView {
	Matrix4x4 viewMatrix = Matrix4x4::Identity;
	Matrix4x4 projectionMatrix = Matrix4x4::Identity;
	Matrix4x4 rotationMatrix = Matrix4x4::Identity; // Skybox が平行移動を抜いたビューを作るのに使う.
	Vector3 position = Vector3::Zero;               // 将来スペキュラ等で使う.
};

// world 行列と射影パラメータ、出力先の解像度から視点を組み立てる.
// world にスケールが乗っていても影響しないよう、各軸を正規化してから使う.
CameraView MakeCameraView(
	const Matrix4x4& world,
	float fovYDegree,
	float nearClip,
	float farClip,
	uint32_t targetWidth,
	uint32_t targetHeight
);

} // namespace Cake
