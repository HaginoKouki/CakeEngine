#include "QuaternionMath.h"

#include <cmath>
#include <numbers>

#include "Vector.h"

namespace Cake {
namespace Math {

float Norm(Quaternion q) {
	return std::sqrt((q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w));
}
Quaternion Normalize(Quaternion q) {
	float norm = Math::Norm(q);
	if (norm < 1e-6f) {
		return Quaternion::Identity;
	}
	return Quaternion(q.x / norm, q.y / norm, q.z / norm, q.w / norm);
}

Quaternion Inverse(Quaternion q) {
	float norm = Math::Norm(q);
	float norm2 = norm * norm;

	if (norm < 1e-6f) {
		return Quaternion::Identity;
	}

	return Quaternion(-q.x / norm2, -q.y / norm2, -q.z / norm2, q.w / norm2);
}

Vector3 ToEuler(Quaternion q) {
	Vector3 angles;

	// roll (X軸まわりの回転)
	float sinXCosp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosXCosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	angles.x = atan2(sinXCosp, cosXCosp);

	// pitch (Y軸まわりの回転)
	float sinY = 2.0f * (q.w * q.y - q.z * q.x);
	if (fabs(sinY) >= 1.0f)
		angles.y = static_cast<float>(copysign(std::numbers::pi / 2.0f, sinY)); // ジンバルロック時は±90°に固定
	else
		angles.y = asin(sinY);

	// yaw (Z軸まわりの回転)
	float sinZCosp = 2.0f * (q.w * q.z + q.x * q.y);
	float cosZCosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	angles.z = atan2(sinZCosp, cosZCosp);

	return angles;
}

} // namespace Math
} // namespace Cake
