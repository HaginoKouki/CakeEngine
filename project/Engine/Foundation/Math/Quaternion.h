#pragma once
#include <type_traits>
#include <concepts>

namespace Cake {

class Vector2;
class Vector3;
class Vector4;

class Matrix2x2;
class Matrix3x3;
class Matrix4x4;

class Quaternion {
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

public:
	Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quaternion(float nX, float nY, float nZ, float nW) : x(nX), y(nY), z(nZ), w(nW) {}
	Quaternion(const Quaternion&) = default;
	Quaternion& operator=(const Quaternion&) = default;

	static const Quaternion Identity;

	template<std::convertible_to<float> T>
	Quaternion operator*(const T& other) const;

	template<std::convertible_to<float> T>
	Quaternion& operator*=(const T& other);

	Quaternion operator-() const;
	Quaternion operator+(const Quaternion& other) const;
	Quaternion operator-(const Quaternion& other) const;
	static Quaternion Multiply(const Quaternion&, const Quaternion&);

	bool operator==(const Quaternion& other) const;

	// 虚部(x, y, z)をVector3として取り出す.
	Vector3 v() const;

	/// <param name="q">（単位 Quaternion）</param>
	/// <returns>オイラー角（ラジアン）</returns>
	static Vector3 ToEuler(const Quaternion& q);
	/// <param name="v">オイラー角（ラジアン）</param>
	/// <returns>回転 Quaternion（単位 Quaternion）</returns>
	static Quaternion FromEuler(const Vector3& v);

	// <returns>ノルム(Quaternion の大きさ)</returns>
	static float Norm(const Quaternion&);
	/// <returns>正規化済みクォータニオン（単位 Quaternion）</returns>
	static Quaternion Normalize(const Quaternion&);
	/// <returns>共役 Quaternion (虚部を反転させた Quaternion)</returns>
	static Quaternion Conjugate(const Quaternion&);
	/// <returns>逆 Quaternion</returns>
	static Quaternion Inverse(const Quaternion&);

	/// <param name="q0">（単位 Quaternion）</param>
	/// <param name="q1">（単位 Quaternion）</param>
	/// <returns>内積</returns>
	static float DotProduct(const Quaternion& q0, const Quaternion& q1);

	/// <summary>
	/// 向き from から向き to への最短の回転 Quaternion を求める
	/// </summary>
	/// <param name="from">初期向き（単位ベクトル）</param>
	/// <param name="to">目標向き（単位ベクトル）</param>
	/// <returns>回転 Quaternion（単位 Quaternion）</returns>
	static Quaternion DirectionToDirection(const Vector3& from, const Vector3& to);

	/// <summary>
	/// 指定した軸と角度に基づいて回転 Quaternion を求める
	/// </summary>
	/// <param name="angleRad">角度（ラジアン）</param>
	/// <param name="axis">回転軸</param>
	/// <returns>回転 Quaternion（単位 Quaternion）</returns>
	static Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angleRad);
};
inline const Quaternion Quaternion::Identity{ 0.0f, 0.0f, 0.0f, 1.0f };

template<std::convertible_to<float> T>
Quaternion Quaternion::operator*(const T& other) const {
	Quaternion result;
	result.x = x * static_cast<float>(other);
	result.y = y * static_cast<float>(other);
	result.z = z * static_cast<float>(other);
	result.w = w * static_cast<float>(other);
	return result;
}
template<std::convertible_to<float> T>
Quaternion operator*(T scalar, const Quaternion& q) {
	return q * static_cast<float>(scalar);
}

template<std::convertible_to<float> T>
Quaternion& Quaternion::operator*=(const T& other) {
	x *= static_cast<float>(other);
	y *= static_cast<float>(other);
	z *= static_cast<float>(other);
	w *= static_cast<float>(other);
	return *this;
}

Quaternion Quaternion::operator-() const {
	return { -x,-y,-z,-w };
}

}
