#pragma once
#include "Quaternion.h"

namespace Cake {

struct Vector3;

namespace Math {

/// <returns>ノルム(Quaternion の大きさ)</returns>
float Norm(Quaternion q);
/// <returns>正規化済みクォータニオン（単位 Quaternion）</returns>
Quaternion Normalize(Quaternion q);

/// <returns>共役 Quaternion (虚部を反転させた Quaternion)</returns>
constexpr Quaternion Conjugate(Quaternion q) {
	return { -q.x, -q.y, -q.z, q.w };
}
/// <returns>逆 Quaternion</returns>
Quaternion Inverse(Quaternion q);

/// <param name="q0">（単位 Quaternion）</param>
/// <param name="q1">（単位 Quaternion）</param>
/// <returns>内積</returns>
constexpr float DotProduct(Quaternion q0, Quaternion q1) {
	return (q0.x * q1.x) + (q0.y * q1.y) + (q0.z * q1.z) + (q0.w * q1.w);
}

/// <param name="q">（単位 Quaternion）</param>
/// <returns>オイラー角（ラジアン）</returns>
Vector3 ToEuler(Quaternion q);

} // namespace Math
} // namespace Cake
