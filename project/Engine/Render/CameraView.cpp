#include "CameraView.h"

#include <numbers>

namespace Cake {

CameraView MakeCameraView(
	const Matrix4x4& world,
	float fovYDegree,
	float near,
	float far,
	uint32_t targetWidth,
	uint32_t targetHeight
) {
	float nearClip = near > 1e-05f ? near : 0.1f;
	float farClip = far > 1e-05f ? far : 0.1f;
	if (farClip - nearClip < 1e-05f) {
		farClip = nearClip + 0.1f;
	}

	// world から回転と位置を取り出す。スケールを打ち消すため各軸を正規化する.
	const Vector3 right = Vector3::Normalize({world.m[0][0], world.m[0][1], world.m[0][2]});
	const Vector3 up = Vector3::Normalize({world.m[1][0], world.m[1][1], world.m[1][2]});
	const Vector3 forward = Vector3::Normalize({world.m[2][0], world.m[2][1], world.m[2][2]});
	const Vector3 position = {world.m[3][0], world.m[3][1], world.m[3][2]};

	Matrix4x4 rotation = Matrix4x4::Identity;
	rotation.m[0][0] = right.x;
	rotation.m[0][1] = right.y;
	rotation.m[0][2] = right.z;
	rotation.m[1][0] = up.x;
	rotation.m[1][1] = up.y;
	rotation.m[1][2] = up.z;
	rotation.m[2][0] = forward.x;
	rotation.m[2][1] = forward.y;
	rotation.m[2][2] = forward.z;

	// 既存の Camera::Update と同じ合成順にする（結果を変えないため）.
	CameraView out;
	out.rotationMatrix = rotation;
	out.position = position;
	out.viewMatrix = Matrix4x4::Inverse(rotation * Matrix4x4::MakeTranslateMatrix(position));

	// アスペクト比は出力先から。ゼロ割りだけ避ける.
	const float aspect = (targetHeight >= 1e-05f)
	                         ? static_cast<float>(targetWidth) / static_cast<float>(targetHeight)
	                         : 1.0f;
	out.projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(
		fovYDegree * std::numbers::pi_v<float> / 180.0f, aspect, nearClip, farClip
	);
	return out;
}

} // namespace Cake
