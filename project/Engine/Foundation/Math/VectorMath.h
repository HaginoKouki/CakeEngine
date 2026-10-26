#pragma once
#include <algorithm>
#include "Vector.h"

namespace Cake {

struct Matrix3x3;
struct Matrix4x4;
struct Quaternion;

namespace Math {

/// <returns>ベクトルの長さ</returns>
float Length(Vector2 v);
/// <returns>ベクトルの長さ</returns>
float Length(Vector3 v);
/// <returns>ベクトルの長さ</returns>
float Length(Vector4 v);

/// <returns>v1, v2間の距離</returns>
float Distance(Vector2 v1, Vector2 v2);
/// <returns>v1, v2間の距離</returns>
float Distance(Vector3 v1, Vector3 v2);
/// <returns>v1, v2間の距離</returns>
float Distance(Vector4 v1, Vector4 v2);

/// <returns>正規化済みベクトル</returns>
Vector2 Normalize(Vector2 target);
/// <returns>正規化済みベクトル</returns>
Vector3 Normalize(Vector3 target);
/// <returns>正規化済みベクトル</returns>
Vector4 Normalize(Vector4 target);

/// <returns>value を min と max の範囲に収めたベクトル</returns>
constexpr Vector2 Clamp(Vector2 value, Vector2 min, Vector2 max) {
	assert(min.x <= max.x && "Math::Clamp error: min.x is greater than max.x");
	assert(min.y <= max.y && "Math::Clamp error: min.y is greater than max.y");
	return {
		std::clamp(value.x, min.x, max.x),
		std::clamp(value.y, min.y, max.y)
	};
}
/// <returns>value を min と max の範囲に収めたベクトル</returns>
constexpr Vector3 Clamp(Vector3 value, Vector3 min, Vector3 max) {
	assert(min.x <= max.x && "Math::Clamp error: min.x is greater than max.x");
	assert(min.y <= max.y && "Math::Clamp error: min.y is greater than max.y");
	assert(min.z <= max.z && "Math::Clamp error: min.z is greater than max.z");
	return {
		std::clamp(value.x, min.x, max.x),
		std::clamp(value.y, min.y, max.y),
		std::clamp(value.z, min.z, max.z)
	};
}
/// <returns>value を min と max の範囲に収めたベクトル</returns>
constexpr Vector4 Clamp(Vector4 value, Vector4 min, Vector4 max) {
	assert(min.x <= max.x && "Math::Clamp error: min.x is greater than max.x");
	assert(min.y <= max.y && "Math::Clamp error: min.y is greater than max.y");
	assert(min.z <= max.z && "Math::Clamp error: min.z is greater than max.z");
	assert(min.w <= max.w && "Math::Clamp error: min.w is greater than max.w");
	return {
		std::clamp(value.x, min.x, max.x),
		std::clamp(value.y, min.y, max.y),
		std::clamp(value.z, min.z, max.z),
		std::clamp(value.w, min.w, max.w)
	};
}

/// <returns>v1, v2の内積</returns>
constexpr float DotProduct(Vector2 v1, Vector2 v2) {
	return (v1.x * v2.x) + (v1.y * v2.y);
}
/// <returns>v1, v2の内積</returns>
constexpr float DotProduct(Vector3 v1, Vector3 v2) {
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}
/// <returns>v1, v2の内積</returns>
constexpr float DotProduct(Vector4 v1, Vector4 v2) {
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}

/// <returns>v1, v2の外積</returns>
constexpr float CrossProduct(Vector2 v1, Vector2 v2) {
	return (v1.x * v2.y) - (v1.y * v2.x);
}
/// <returns>v1, v2の外積</returns>
constexpr Vector3 CrossProduct(Vector3 v1, Vector3 v2) {
	return Vector3(
		(v1.y * v2.z) - (v1.z * v2.y),
		(v1.z * v2.x) - (v1.x * v2.z),
		(v1.x * v2.y) - (v1.y * v2.x)
	);
}

/// <returns>角度(度数法)</returns>
float ToDegree(Vector2 target);
/// <returns>ラジアン</returns>
float ToRadian(Vector2 target);
/// <returns>from から to への符号付き角度差（ラジアン）</returns>
float SignedAngleDifference(Vector2 from, Vector2 to);

/// <param name="v">回転させるベクトル</param>
/// <param name="q">正規化済みの Quaternion</param>
Vector3 Rotate(Vector3 v, const Quaternion& q);

Vector2 TransformCoord(Vector2 v, const Matrix3x3& m);
Vector3 TransformCoord(Vector3 v, const Matrix4x4& m);
/// 拡縮、回転を適用。移動は適用しない.
Vector3 TransformNormal(Vector3 v, const Matrix4x4& m);

} // namespace Math
} // namespace Cake
