#include "VectorMath.h"

#include <numbers>
#include <cmath>

#include "Matrix.h"
#include "Quaternion.h"
#include "QuaternionMath.h"

namespace Cake {
namespace Math {

float Length(Vector2 v) {
	return sqrtf(Math::DotProduct(v, v));
}
float Length(Vector3 v) {
	return sqrtf(Math::DotProduct(v, v));
}
float Length(Vector4 v) {
	return sqrtf(Math::DotProduct(v, v));
}

float Distance(Vector2 v1, Vector2 v2) {
	Vector2 local = v2 - v1;
	return sqrtf(Math::DotProduct(local, local));
}
float Distance(Vector3 v1, Vector3 v2) {
	Vector3 local = v2 - v1;
	return sqrtf(Math::DotProduct(local, local));
}
float Distance(Vector4 v1, Vector4 v2) {
	Vector4 local = v2 - v1;
	return sqrtf(Math::DotProduct(local, local));
}

Vector2 Normalize(Vector2 target) {
	float length = Math::Length(target);
	return length == 0.0f ? Vector2::Zero : target / length;
}
Vector3 Normalize(Vector3 target) {
	float length = Math::Length(target);
	return length == 0.0f ? Vector3::Zero : target / length;
}
Vector4 Normalize(Vector4 target) {
	float length = Math::Length(target);
	return length == 0.0f ? Vector4::Zero : target / length;
}

float ToDegree(Vector2 target) {
	return atan2f(target.y, target.x) * 180.0f / std::numbers::pi_v<float>;
}

float ToRadian(Vector2 target) {
	return atan2f(target.y, target.x);
}

float SignedAngleDifference(Vector2 from, Vector2 to) {
	double angleA = static_cast<double>(Math::ToRadian(from));
	double angleB = static_cast<double>(Math::ToRadian(to));
	// 浮動小数点の剰余演算を使用して-π〜πの範囲に収める.
	return static_cast<float>(std::remainder(angleB - angleA, 2.0f * std::numbers::pi_v<float>));
}

Vector3 Rotate(Vector3 v, const Quaternion& q) {
	return Quaternion::Multiply(Quaternion::Multiply(q, Quaternion{v.x, v.y, v.z, 0}), Math::Conjugate(q)).v();
}

Vector2 TransformCoord(Vector2 vector, const Matrix3x3& matrix) {
	float w = (vector.x * matrix.m[0][2]) + (vector.y * matrix.m[1][2]) + (1.0f * matrix.m[2][2]);
	assert(w != 0.0f);
	Vector2 result;
	result.x = ((vector.x * matrix.m[0][0]) + (vector.y * matrix.m[1][0]) + (1.0f * matrix.m[2][0])) / w;
	result.y = ((vector.x * matrix.m[0][1]) + (vector.y * matrix.m[1][1]) + (1.0f * matrix.m[2][1])) / w;
	return result;
}
Vector3 TransformCoord(Vector3 v, const Matrix4x4& m) {
	float w = (v.x * m.m[0][3]) + (v.y * m.m[1][3]) + (v.z * m.m[2][3]) + (1.0f * m.m[3][3]);
	assert(w != 0.0f);
	Vector3 result;
	result.x = ((v.x * m.m[0][0]) + (v.y * m.m[1][0]) + (v.z * m.m[2][0]) + (1.0f * m.m[3][0])) / w;
	result.y = ((v.x * m.m[0][1]) + (v.y * m.m[1][1]) + (v.z * m.m[2][1]) + (1.0f * m.m[3][1])) / w;
	result.z = ((v.x * m.m[0][2]) + (v.y * m.m[1][2]) + (v.z * m.m[2][2]) + (1.0f * m.m[3][2])) / w;
	return result;
}

Vector3 TransformNormal(Vector3 v, const Matrix4x4& m) {
	Vector3 result;
	result.x = (v.x * m.m[0][0]) + (v.y * m.m[1][0]) + (v.z * m.m[2][0]);
	result.y = (v.x * m.m[0][1]) + (v.y * m.m[1][1]) + (v.z * m.m[2][1]);
	result.z = (v.x * m.m[0][2]) + (v.y * m.m[1][2]) + (v.z * m.m[2][2]);
	return result;
}

} // namespace Math
} // namespace Cake
