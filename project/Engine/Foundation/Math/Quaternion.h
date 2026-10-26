#pragma once
/*====================================
 *
 * 回転を表す Quaternion を定義する構造体。
 * 算術演算子と、回転 Quaternion を作る静的関数を持つ。
 * 正規化・共役・逆数などの計算は QuaternionMath.h で Math 名前空間の関数として提供する。
 *
 * ====================================*/
#include <concepts>

namespace Cake {

struct Vector3;

struct Quaternion {
	float x;
	float y;
	float z;
	float w;

	constexpr Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	constexpr Quaternion(float nX, float nY, float nZ, float nW) : x(nX), y(nY), z(nZ), w(nW) {}

	static const Quaternion Identity;

	template<std::convertible_to<float> T>
	constexpr Quaternion operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Quaternion& operator*=(const T& other);

	constexpr Quaternion operator-() const;
	constexpr Quaternion operator+(const Quaternion& other) const;
	constexpr Quaternion operator-(const Quaternion& other) const;
	static Quaternion Multiply(Quaternion q1, Quaternion q2);

	constexpr bool operator==(const Quaternion& other) const = default;

	// 虚部(x, y, z)をVector3として取り出す.
	Vector3 v() const;

	/// <param name="euler">オイラー角（ラジアン）</param>
	/// <returns>回転 Quaternion（単位 Quaternion）</returns>
	static Quaternion FromEuler(Vector3 euler);

	/// <summary>
	/// 向き from から向き to への最短の回転 Quaternion を求める
	/// </summary>
	/// <param name="from">初期向き（単位ベクトル）</param>
	/// <param name="to">目標向き（単位ベクトル）</param>
	/// <returns>回転 Quaternion（単位 Quaternion）</returns>
	static Quaternion DirectionToDirection(Vector3 from, Vector3 to);

	/// <summary>
	/// 指定した軸と角度に基づいて回転 Quaternion を求める
	/// </summary>
	/// <param name="axis">回転軸</param>
	/// <param name="angleRad">角度（ラジアン）</param>
	/// <returns>回転 Quaternion（単位 Quaternion）</returns>
	static Quaternion MakeRotateAxisAngleQuaternion(Vector3 axis, float angleRad);
};
inline constexpr Quaternion Quaternion::Identity{ 0.0f, 0.0f, 0.0f, 1.0f };

template<std::convertible_to<float> T>
constexpr Quaternion Quaternion::operator*(const T& other) const {
	return {
		x * static_cast<float>(other),
		y * static_cast<float>(other),
		z * static_cast<float>(other),
		w * static_cast<float>(other)
	};
}
template<std::convertible_to<float> T>
constexpr Quaternion operator*(const T& scalar, const Quaternion& q) {
	return q * static_cast<float>(scalar);
}
template<std::convertible_to<float> T>
constexpr Quaternion& Quaternion::operator*=(const T& other) {
	x *= static_cast<float>(other);
	y *= static_cast<float>(other);
	z *= static_cast<float>(other);
	w *= static_cast<float>(other);
	return *this;
}

constexpr Quaternion Quaternion::operator-() const {
	return { -x, -y, -z, -w };
}
constexpr Quaternion Quaternion::operator+(const Quaternion& other) const {
	return { x + other.x, y + other.y, z + other.z, w + other.w };
}
constexpr Quaternion Quaternion::operator-(const Quaternion& other) const {
	return { x - other.x, y - other.y, z - other.z, w - other.w };
}

} // namespace Cake
