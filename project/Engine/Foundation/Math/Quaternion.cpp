#include "Quaternion.h"

#include <cmath>
#include <numbers>

#include "Vector.h"
#include "Math.h"

namespace Cake {

Quaternion Quaternion::Multiply(Quaternion q1, Quaternion q2) {
	Vector3 v = Math::CrossProduct(q1.v(), q2.v()) + (q1.v() * q2.w) + (q2.v() * q1.w);
	return Quaternion{ v.x, v.y, v.z, (q1.w * q2.w) - Math::DotProduct(q1.v(), q2.v()) };
}

Vector3 Quaternion::v() const {
	return Vector3{ x, y, z };
}

Quaternion Quaternion::FromEuler(Vector3 euler) {
	// 各軸の半角の sin / cos
	float cr = cos(euler.x * 0.5f);
	float sr = sin(euler.x * 0.5f);
	float cp = cos(euler.y * 0.5f);
	float sp = sin(euler.y * 0.5f);
	float cy = cos(euler.z * 0.5f);
	float sy = sin(euler.z * 0.5f);

	Quaternion q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

Quaternion Quaternion::DirectionToDirection(Vector3 from, Vector3 to) {
	const float tolerance = 1.0e-6f;
	float dot = Math::DotProduct(from, to);
	// to = -from のとき.
	if (dot <= -1.0f + tolerance) {
		// from と平行でない補助ベクトル（from.x がX軸に近いならY軸、そうでないならX軸）との外積を取り、from に垂直な回転軸 axis を求める.
		Vector3 axis = Math::Normalize(Math::CrossProduct(from, (std::fabs(from.x) < 0.9f ? Vector3{ 1,0,0 } : Vector3{ 0,1,0 })));
		return Quaternion{ axis.x, axis.y, axis.z, 0.0f };
	}
	Vector3 cross = Math::CrossProduct(from, to);
	Quaternion rotation{ cross.x, cross.y, cross.z, 1.0f + dot };
	return Math::Normalize(rotation);
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(Vector3 axis, float angleRad) {
	// 軸は必ず正規化する.
	Vector3 n = Math::Normalize(axis);

	float s = std::sin(angleRad * 0.5f);
	float c = std::cos(angleRad * 0.5f);

	return Quaternion(
		n.x * s,
		n.y * s,
		n.z * s,
		c
	);
}

} // namespace Cake
