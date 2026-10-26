#pragma once
/*====================================
 *
 * 数学ライブラリの基本：Matrix2x2, Matrix3x3, Matrix4x4を定義する構造体群。
 * 算術演算子と、各種変換行列を作る静的関数(Make～)を持つ。
 * 逆行列・転置などの計算は MatrixMath.h で Math 名前空間の関数として提供する。
 *
 * ====================================*/
#include <concepts>

namespace Cake {

struct Vector2;
struct Vector3;

struct Quaternion;

/*---------------------------------
*
* Matrix2x2
*
---------------------------------*/
struct Matrix2x2 {
	float m[2][2];

	constexpr Matrix2x2(
		float m00 = 0.0f, float m01 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f
	) : m{ { m00, m01 }, { m10, m11 } } {}

	static const Matrix2x2 Zero;
	static const Matrix2x2 One;
	static const Matrix2x2 Identity;

	constexpr Matrix2x2 operator+(const Matrix2x2& other) const;
	constexpr Matrix2x2 operator-(const Matrix2x2& other) const;
	constexpr Matrix2x2 operator*(const Matrix2x2& other) const;
	constexpr Matrix2x2& operator+=(const Matrix2x2& other);
	constexpr Matrix2x2& operator-=(const Matrix2x2& other);
	constexpr Matrix2x2& operator*=(const Matrix2x2& other);

	template<std::convertible_to<float> T>
	constexpr Matrix2x2 operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Matrix2x2& operator*=(const T& other);

	/// <returns>回転行列</returns>
	static Matrix2x2 MakeRotationMatrix(float theta);
};
inline constexpr Matrix2x2 Matrix2x2::Zero{
	0.0f, 0.0f,
	0.0f, 0.0f
};
inline constexpr Matrix2x2 Matrix2x2::One{
	1.0f, 1.0f,
	1.0f, 1.0f
};
inline constexpr Matrix2x2 Matrix2x2::Identity{
	1.0f, 0.0f,
	0.0f, 1.0f
};

constexpr Matrix2x2 Matrix2x2::operator+(const Matrix2x2& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	return result;
}
constexpr Matrix2x2 Matrix2x2::operator-(const Matrix2x2& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	return result;
}
constexpr Matrix2x2 Matrix2x2::operator*(const Matrix2x2& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = (m[y][0] * other.m[0][x]) + (m[y][1] * other.m[1][x]);
		}
	}
	return result;
}
constexpr Matrix2x2& Matrix2x2::operator+=(const Matrix2x2& other) {
	*this = *this + other;
	return *this;
}
constexpr Matrix2x2& Matrix2x2::operator-=(const Matrix2x2& other) {
	*this = *this - other;
	return *this;
}
constexpr Matrix2x2& Matrix2x2::operator*=(const Matrix2x2& other) {
	*this = *this * other;
	return *this;
}

template<std::convertible_to<float> T>
constexpr Matrix2x2 Matrix2x2::operator*(const T& other) const {
	Matrix2x2 result;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			result.m[y][x] = m[y][x] * static_cast<float>(other);
		}
	}
	return result;
}
template<std::convertible_to<float> T>
constexpr Matrix2x2& Matrix2x2::operator*=(const T& other) {
	*this = *this * other;
	return *this;
}
template<std::convertible_to<float> T>
constexpr Matrix2x2 operator*(const T& scalar, const Matrix2x2& m) {
	return m * static_cast<float>(scalar);
}


/*---------------------------------
*
* Matrix3x3
*
---------------------------------*/
struct Matrix3x3 {
	float m[3][3];

	constexpr Matrix3x3(
		float m00 = 0.0f, float m01 = 0.0f, float m02 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f, float m12 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 0.0f
	) : m{ { m00, m01, m02 }, { m10, m11, m12 }, { m20, m21, m22 } } {}

	static const Matrix3x3 Zero;
	static const Matrix3x3 One;
	static const Matrix3x3 Identity;

	constexpr Matrix3x3 operator+(const Matrix3x3& other) const;
	constexpr Matrix3x3 operator-(const Matrix3x3& other) const;
	constexpr Matrix3x3 operator*(const Matrix3x3& other) const;
	constexpr Matrix3x3& operator+=(const Matrix3x3& other);
	constexpr Matrix3x3& operator-=(const Matrix3x3& other);
	constexpr Matrix3x3& operator*=(const Matrix3x3& other);

	template<std::convertible_to<float> T>
	constexpr Matrix3x3 operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Matrix3x3& operator*=(const T& other);

	/// <returns>平行移動行列</returns>
	static Matrix3x3 MakeTranslateMatrix(Vector2 translate);
	/// <returns>回転行列</returns>
	static Matrix3x3 MakeRotationMatrix(float theta);
	/// <returns>拡大縮小行列</returns>
	static Matrix3x3 MakeScalingMatrix(Vector2 scale);
	/// <returns>アフィン変換行列</returns>
	static Matrix3x3 MakeAffineMatrix(Vector2 scale, float theta, Vector2 translate);

	/// <summary>
	/// カメラ座標系から正規化デバイス座標系(NDC)へ変換するための正射影行列を作成する.
	/// </summary>
	/// <param name="left">カメラ座標系での左端</param>
	/// <param name="top">カメラ座標系での上端</param>
	/// <param name="right">カメラ座標系での右端</param>
	/// <param name="bottom">カメラ座標系での下端</param>
	/// <returns>正射影行列</returns>
	static Matrix3x3 MakeOrthoMatrix(float left, float top, float right, float bottom);
	/// <summary>
	/// 正規化デバイス座標系(NDC)からビューポート座標系へ変換するためのビューポート行列を作成する.
	/// </summary>
	/// <param name="left">スクリーン座標系での左端</param>
	/// <param name="top">スクリーン座標系での上端</param>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <returns>ビューポート行列</returns>
	static Matrix3x3 MakeViewportMatrix(float left, float top, float width, float height);
};
inline constexpr Matrix3x3 Matrix3x3::Zero{
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
};
inline constexpr Matrix3x3 Matrix3x3::One{
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f
};
inline constexpr Matrix3x3 Matrix3x3::Identity{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
};

constexpr Matrix3x3 Matrix3x3::operator+(const Matrix3x3& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	return result;
}
constexpr Matrix3x3 Matrix3x3::operator-(const Matrix3x3& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	return result;
}
constexpr Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][0] * other.m[0][x] + m[y][1] * other.m[1][x] + m[y][2] * other.m[2][x];
		}
	}
	return result;
}
constexpr Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& other) {
	*this = *this + other;
	return *this;
}
constexpr Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& other) {
	*this = *this - other;
	return *this;
}
constexpr Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& other) {
	*this = *this * other;
	return *this;
}

template<std::convertible_to<float> T>
constexpr Matrix3x3 Matrix3x3::operator*(const T& other) const {
	Matrix3x3 result;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			result.m[y][x] = m[y][x] * static_cast<float>(other);
		}
	}
	return result;
}
template<std::convertible_to<float> T>
constexpr Matrix3x3& Matrix3x3::operator*=(const T& other) {
	*this = *this * other;
	return *this;
}
template<std::convertible_to<float> T>
constexpr Matrix3x3 operator*(const T& scalar, const Matrix3x3& m) {
	return m * static_cast<float>(scalar);
}


/*---------------------------------
*
* Matrix4x4
*
---------------------------------*/
struct Matrix4x4 {
	float m[4][4];

	constexpr Matrix4x4(
		float m00 = 0.0f, float m01 = 0.0f, float m02 = 0.0f, float m03 = 0.0f,
		float m10 = 0.0f, float m11 = 0.0f, float m12 = 0.0f, float m13 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 0.0f, float m23 = 0.0f,
		float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 0.0f
	) : m{
		{ m00, m01, m02, m03 },
		{ m10, m11, m12, m13 },
		{ m20, m21, m22, m23 },
		{ m30, m31, m32, m33 }
	} {}

	static const Matrix4x4 Zero;
	static const Matrix4x4 One;
	static const Matrix4x4 Identity;

	constexpr Matrix4x4 operator+(const Matrix4x4& other) const;
	constexpr Matrix4x4 operator-(const Matrix4x4& other) const;
	constexpr Matrix4x4 operator*(const Matrix4x4& other) const;
	constexpr Matrix4x4& operator+=(const Matrix4x4& other);
	constexpr Matrix4x4& operator-=(const Matrix4x4& other);
	constexpr Matrix4x4& operator*=(const Matrix4x4& other);

	template<std::convertible_to<float> T>
	constexpr Matrix4x4 operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Matrix4x4& operator*=(const T& other);

	/// <returns>拡大縮小行列</returns>
	static Matrix4x4 MakeScalingMatrix(Vector3 scale);
	/// <returns>平行移動行列</returns>
	static Matrix4x4 MakeTranslateMatrix(Vector3 translate);
	/// <returns>X軸回転行列</returns>
	static Matrix4x4 MakeXRotationMatrix(float theta);
	/// <returns>Y軸回転行列</returns>
	static Matrix4x4 MakeYRotationMatrix(float theta);
	/// <returns>Z軸回転行列</returns>
	static Matrix4x4 MakeZRotationMatrix(float theta);
	/// <returns>回転行列</returns>
	static Matrix4x4 MakeRotationMatrix(float rotationX, float rotationY, float rotationZ);
	/// <returns>回転行列</returns>
	static Matrix4x4 MakeRotationMatrix(Vector3 rotation);
	/// <param name="q">（単位 Quaternion）</param>
	/// <returns>回転行列</returns>
	static Matrix4x4 MakeRotationMatrix(Quaternion q);
	/// <returns>アフィン変換行列</returns>
	static Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotation, Vector3 translate);

	/// <summary>
	/// カメラ座標系から同次クリップ空間へ変換する正射影行列を作成する.
	/// </summary>
	/// <param name="left">カメラ座標系での左端</param>
	/// <param name="top">カメラ座標系での上端</param>
	/// <param name="right">カメラ座標系での右端</param>
	/// <param name="bottom">カメラ座標系での下端</param>
	/// <param name="nearClip">ニアクリップ</param>
	/// <param name="farClip">ファークリップ</param>
	/// <returns>正射影行列</returns>
	static Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
	/// <summary>
	/// 同次クリップ空間から正規化デバイス座標系(NDC)へ変換する透視投影行列を作成する.
	/// </summary>
	/// <param name="fovY">視野角</param>
	/// <param name="aspectRatio">アスペクト比</param>
	/// <param name="nearClip">ニアクリップ</param>
	/// <param name="farClip">ファークリップ</param>
	/// <returns>透視投影行列</returns>
	static Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	/// <summary>
	/// 正規化デバイス座標系(NDC)からスクリーン座標系(SCS)へ変換するビューポート行列を作成する.
	/// </summary>
	/// <param name="left">スクリーン座標系での左端</param>
	/// <param name="top">スクリーン座標系での上端</param>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <param name="minDepth">最小深度値</param>
	/// <param name="maxDepth">最大深度値</param>
	/// <returns>ビューポート変換行列</returns>
	static Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
};
inline constexpr Matrix4x4 Matrix4x4::Zero{
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
};
inline constexpr Matrix4x4 Matrix4x4::One{
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};
inline constexpr Matrix4x4 Matrix4x4::Identity{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

constexpr Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] + other.m[y][x];
		}
	}
	return result;
}
constexpr Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] - other.m[y][x];
		}
	}
	return result;
}
constexpr Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][0] * other.m[0][x] + m[y][1] * other.m[1][x] + m[y][2] * other.m[2][x] + m[y][3] * other.m[3][x];
		}
	}
	return result;
}
constexpr Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other) {
	*this = *this + other;
	return *this;
}
constexpr Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other) {
	*this = *this - other;
	return *this;
}
constexpr Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other) {
	*this = *this * other;
	return *this;
}

template<std::convertible_to<float> T>
constexpr Matrix4x4 Matrix4x4::operator*(const T& other) const {
	Matrix4x4 result;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			result.m[y][x] = m[y][x] * static_cast<float>(other);
		}
	}
	return result;
}
template<std::convertible_to<float> T>
constexpr Matrix4x4& Matrix4x4::operator*=(const T& other) {
	*this = *this * other;
	return *this;
}
template<std::convertible_to<float> T>
constexpr Matrix4x4 operator*(const T& scalar, const Matrix4x4& m) {
	return m * static_cast<float>(scalar);
}

} // namespace Cake
