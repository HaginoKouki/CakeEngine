#pragma once
#include "Matrix.h"

namespace Cake {
namespace Math {

/// <returns>逆行列</returns>
Matrix2x2 Inverse(const Matrix2x2& m);
/// <returns>逆行列</returns>
Matrix3x3 Inverse(const Matrix3x3& m);
/// <returns>逆行列。行列式が0のときは Matrix4x4::Zero</returns>
Matrix4x4 Inverse(const Matrix4x4& m);

/// <returns>転置行列</returns>
constexpr Matrix2x2 Transpose(const Matrix2x2& m) {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m.m[x][y];
		}
	}
	return result;
}
/// <returns>転置行列</returns>
constexpr Matrix3x3 Transpose(const Matrix3x3& m) {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m.m[x][y];
		}
	}
	return result;
}
/// <returns>転置行列</returns>
constexpr Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m.m[x][y];
		}
	}
	return result;
}

} // namespace Math
} // namespace Cake
