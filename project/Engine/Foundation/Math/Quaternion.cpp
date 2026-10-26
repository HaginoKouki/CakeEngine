#include "Quaternion.h"

#include <cmath>
#include <numbers>

#include "Vector.h"

namespace Cake {
Quaternion Quaternion::operator+(const Quaternion& other) const {
	return Quaternion(
		x + other.x,
		y + other.y,
		z + other.z,
		w + other.w
	);
}
Quaternion Quaternion::operator-(const Quaternion& other) const {
	return Quaternion(
		x - other.x,
		y - other.y,
		z - other.z,
		w - other.w
	);
}
Quaternion Quaternion::Multiply(const Quaternion& q1, const Quaternion& q2) {
	Vector3 v = Vector3::CrossProduct(q1.v(), q2.v()) + (q1.v() * q2.w) + (q2.v() * q1.w);
	return Quaternion{ v.x, v.y, v.z, (q1.w * q2.w) - Vector3::DotProduct(q1.v(), q2.v())};
}

bool Quaternion::operator==(const Quaternion& other) const {
	return (x == other.x && y == other.y && z == other.z && w == other.w);
}

Vector3 Quaternion::v() const {
	return Vector3{ x, y, z };
};


Vector3 Quaternion::ToEuler(const Quaternion& q) {
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
Quaternion Quaternion::FromEuler(const Vector3& euler) {
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

float Quaternion::Norm(const Quaternion& q) {
	return std::sqrt((q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w));
}
Quaternion Quaternion::Normalize(const Quaternion& q) {
	float norm = Quaternion::Norm(q);
	if (norm < 1e-6f) {
		return Identity;
	}
	return Quaternion(q.x / norm, q.y / norm, q.z / norm, q.w / norm);
}
Quaternion Quaternion::Conjugate(const Quaternion& q) {
	return Quaternion{ -q.x, -q.y, -q.z, q.w };
}
Quaternion Quaternion::Inverse(const Quaternion& q) {
	float norm = Quaternion::Norm(q);
	float norm2 = norm * norm;

	if (norm < 1e-6f) {
		return Identity;
	}

	return Quaternion(-q.x / norm2, -q.y / norm2, -q.z / norm2, q.w / norm2);
}

float Quaternion::DotProduct(const Quaternion& a, const Quaternion& b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

Quaternion Quaternion::DirectionToDirection(const Vector3& from, const Vector3& to) {
	const float tolerance = 1.0e-6f;
	float dot = Vector3::DotProduct(from, to);
	// to = -from のとき.
	if (dot <= -1.0f + tolerance) {
		// from と平行でない補助ベクトル（from.x がX軸に近いならY軸、そうでないならX軸）との外積を取り、from に垂直な回転軸 axis を求める.
		Vector3 axis = Vector3::Normalize(Vector3::CrossProduct(from, (std::fabs(from.x) < 0.9f ? Vector3{ 1,0,0 } : Vector3{ 0,1,0 })));
		return Quaternion{ axis.x, axis.y, axis.z, 0.0f };
	}
	Vector3 cross = Vector3::CrossProduct(from, to);
	Quaternion rotation{ cross.x, cross.y, cross.z, 1.0f + dot };
	return Normalize(rotation);
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(const Vector3& axis, float angleRad) {
	// 軸は必ず正規化する.
	Vector3 n = Vector3::Normalize(axis);

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
