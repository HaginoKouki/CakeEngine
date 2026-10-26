#include "MatrixMath.h"

namespace Cake {
namespace Math {

Matrix2x2 Inverse(const Matrix2x2& target) {
	float scalar = 1.0f / (target.m[0][0] * target.m[1][1] - target.m[0][1] * target.m[1][0]);

	Matrix2x2 result;

	result.m[0][0] = target.m[1][1] * scalar;
	result.m[0][1] = -target.m[0][1] * scalar;

	result.m[1][0] = -target.m[1][0] * scalar;
	result.m[1][1] = target.m[0][0] * scalar;

	return result;
}

Matrix3x3 Inverse(const Matrix3x3& target) {
	Matrix3x3 result;

	float a11 = target.m[0][0];
	float a12 = target.m[0][1];
	float a13 = target.m[0][2];

	float a21 = target.m[1][0];
	float a22 = target.m[1][1];
	float a23 = target.m[1][2];

	float a31 = target.m[2][0];
	float a32 = target.m[2][1];
	float a33 = target.m[2][2];

	float absolute = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32) - (a13 * a22 * a31) - (a12 * a21 * a33) - (a11 * a23 * a32);
	absolute = 1.0f / absolute;

	result.m[0][0] = (a22 * a33 - a23 * a32) * absolute;
	result.m[0][1] = -(a12 * a33 - a13 * a32) * absolute;
	result.m[0][2] = (a12 * a23 - a13 * a22) * absolute;

	result.m[1][0] = -(a21 * a33 - a23 * a31) * absolute;
	result.m[1][1] = (a11 * a33 - a13 * a31) * absolute;
	result.m[1][2] = -(a11 * a23 - a13 * a21) * absolute;

	result.m[2][0] = (a21 * a32 - a22 * a31) * absolute;
	result.m[2][1] = -(a11 * a32 - a12 * a31) * absolute;
	result.m[2][2] = (a11 * a22 - a12 * a21) * absolute;

	return result;
}

Matrix4x4 Inverse(const Matrix4x4& target) {
	float a11 = target.m[0][0];
	float a12 = target.m[0][1];
	float a13 = target.m[0][2];
	float a14 = target.m[0][3];

	float a21 = target.m[1][0];
	float a22 = target.m[1][1];
	float a23 = target.m[1][2];
	float a24 = target.m[1][3];

	float a31 = target.m[2][0];
	float a32 = target.m[2][1];
	float a33 = target.m[2][2];
	float a34 = target.m[2][3];

	float a41 = target.m[3][0];
	float a42 = target.m[3][1];
	float a43 = target.m[3][2];
	float a44 = target.m[3][3];

	float absolute = 0.0f;
	absolute += +(a11 * a22 * a33 * a44) + (a11 * a23 * a34 * a42) + (a11 * a24 * a32 * a43);
	absolute += -(a11 * a24 * a33 * a42) - (a11 * a23 * a32 * a44) - (a11 * a22 * a34 * a43);
	absolute += -(a12 * a21 * a33 * a44) - (a13 * a21 * a34 * a42) - (a14 * a21 * a32 * a43);
	absolute += +(a14 * a21 * a33 * a42) + (a13 * a21 * a32 * a44) + (a12 * a21 * a34 * a43);
	absolute += +(a12 * a23 * a31 * a44) + (a13 * a24 * a31 * a42) + (a14 * a22 * a31 * a43);
	absolute += -(a14 * a23 * a31 * a42) - (a13 * a22 * a31 * a44) - (a12 * a24 * a31 * a43);
	absolute += -(a12 * a23 * a34 * a41) - (a13 * a24 * a32 * a41) - (a14 * a22 * a33 * a41);
	absolute += +(a14 * a23 * a32 * a41) + (a13 * a22 * a34 * a41) + (a12 * a24 * a33 * a41);

	if (absolute == 0.0f) {
		return Matrix4x4::Zero;
	}

	Matrix4x4 result;

	result.m[0][0] = (a22 * a33 * a44) + (a23 * a34 * a42) + (a24 * a32 * a43) - (a24 * a33 * a42) - (a23 * a32 * a44) - (a22 * a34 * a43);
	result.m[0][1] = -(a12 * a33 * a44) - (a13 * a34 * a42) - (a14 * a32 * a43) + (a14 * a33 * a42) + (a13 * a32 * a44) + (a12 * a34 * a43);
	result.m[0][2] = (a12 * a23 * a44) + (a13 * a24 * a42) + (a14 * a22 * a43) - (a14 * a23 * a42) - (a13 * a22 * a44) - (a12 * a24 * a43);
	result.m[0][3] = -(a12 * a23 * a34) - (a13 * a24 * a32) - (a14 * a22 * a33) + (a14 * a23 * a32) + (a13 * a22 * a34) + (a12 * a24 * a33);

	result.m[1][0] = -(a21 * a33 * a44) - (a23 * a34 * a41) - (a24 * a31 * a43) + (a24 * a33 * a41) + (a23 * a31 * a44) + (a21 * a34 * a43);
	result.m[1][1] = (a11 * a33 * a44) + (a13 * a34 * a41) + (a14 * a31 * a43) - (a14 * a33 * a41) - (a13 * a31 * a44) - (a11 * a34 * a43);
	result.m[1][2] = -(a11 * a23 * a44) - (a13 * a24 * a41) - (a14 * a21 * a43) + (a14 * a23 * a41) + (a13 * a21 * a44) + (a11 * a24 * a43);
	result.m[1][3] = (a11 * a23 * a34) + (a13 * a24 * a31) + (a14 * a21 * a33) - (a14 * a23 * a31) - (a13 * a21 * a34) - (a11 * a24 * a33);

	result.m[2][0] = (a21 * a32 * a44) + (a22 * a34 * a41) + (a24 * a31 * a42) - (a24 * a32 * a41) - (a22 * a31 * a44) - (a21 * a34 * a42);
	result.m[2][1] = -(a11 * a32 * a44) - (a12 * a34 * a41) - (a14 * a31 * a42) + (a14 * a32 * a41) + (a12 * a31 * a44) + (a11 * a34 * a42);
	result.m[2][2] = (a11 * a22 * a44) + (a12 * a24 * a41) + (a14 * a21 * a42) - (a14 * a22 * a41) - (a12 * a21 * a44) - (a11 * a24 * a42);
	result.m[2][3] = -(a11 * a22 * a34) - (a12 * a24 * a31) - (a14 * a21 * a32) + (a14 * a22 * a31) + (a12 * a21 * a34) + (a11 * a24 * a32);

	result.m[3][0] = -(a21 * a32 * a43) - (a22 * a33 * a41) - (a23 * a31 * a42) + (a23 * a32 * a41) + (a22 * a31 * a43) + (a21 * a33 * a42);
	result.m[3][1] = (a11 * a32 * a43) + (a12 * a33 * a41) + (a13 * a31 * a42) - (a13 * a32 * a41) - (a12 * a31 * a43) - (a11 * a33 * a42);
	result.m[3][2] = -(a11 * a22 * a43) - (a12 * a23 * a41) - (a13 * a21 * a42) + (a13 * a22 * a41) + (a12 * a21 * a43) + (a11 * a23 * a42);
	result.m[3][3] = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32) - (a13 * a22 * a31) - (a12 * a21 * a33) - (a11 * a23 * a32);

	result *= 1.0f / absolute;

	return result;
}

} // namespace Math
} // namespace Cake
