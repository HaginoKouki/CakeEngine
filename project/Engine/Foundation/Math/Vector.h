#pragma once
/*====================================
 *
 * 数学ライブラリの基本：Vector2, Vector3, Vector4を定義する構造体群。
 * 行列演算等のすべての幾何学的計算の基盤となる。
 *
 * ====================================*/
#include <cassert>
#include <type_traits>
#include <concepts>


namespace Cake {

struct Vector2 {
	float x;
	float y;

	constexpr Vector2() : x(0.0f), y(0.0f) {}
	constexpr Vector2(float nX, float nY) : x(nX), y(nY) {}

	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 UnitX;
	static const Vector2 UnitY;

	// 算術演算子.
	template <std::convertible_to<float> T>
	constexpr Vector2 operator+(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector2 operator-(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector2 operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector2 operator/(const T& other) const;

	template<std::convertible_to<float> T>
	constexpr Vector2& operator+=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector2& operator-=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector2& operator*=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector2& operator/=(const T& other);

	constexpr Vector2 operator+(const Vector2& other) const;
	constexpr Vector2 operator-(const Vector2& other) const;
	constexpr Vector2& operator+=(const Vector2& other);
	constexpr Vector2& operator-=(const Vector2& other);
	static constexpr Vector2 Multiply(const Vector2&, const Vector2&);

	// 比較演算子.
	constexpr bool operator==(const Vector2& other) const = default;

	// 単項演算子.
	constexpr Vector2 operator-() const;
	constexpr Vector2& operator++();
	constexpr Vector2& operator--();

	/// <param name="degree">角度(度数法)</param>
	static Vector2 FromDegreeUnit(float degree);
	/// <param name="radian">ラジアン</param>
	static Vector2 FromRadianUnit(float radian);

};
inline constexpr Vector2 Vector2::Zero{0.0f, 0.0f};
inline constexpr Vector2 Vector2::One{1.0f, 1.0f};
inline constexpr Vector2 Vector2::UnitX{1.0f, 0.0f};
inline constexpr Vector2 Vector2::UnitY{0.0f, 1.0f};

template<std::convertible_to<float> T>
constexpr Vector2 Vector2::operator+(const T& other) const {
	Vector2 num;
	num.x = x + static_cast<float>(other);
	num.y = y + static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector2 Vector2::operator-(const T& other) const {
	Vector2 num;
	num.x = x - static_cast<float>(other);
	num.y = y - static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector2 Vector2::operator*(const T& other) const {
	Vector2 num;
	num.x = x * static_cast<float>(other);
	num.y = y * static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector2 operator*(const T& scalar, const Vector2& v) {
	return v * static_cast<float>(scalar);
}
template<std::convertible_to<float> T>
constexpr Vector2 Vector2::operator/(const T& other) const {
	assert(other != 0 && "Division by zero error in Vector2::operator/");
	Vector2 num;
	num.x = x / static_cast<float>(other);
	num.y = y / static_cast<float>(other);
	return num;
}

template<std::convertible_to<float> T>
constexpr Vector2& Vector2::operator+=(const T& other) {
	this->x += static_cast<float>(other);
	this->y += static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector2& Vector2::operator-=(const T& other) {
	this->x -= static_cast<float>(other);
	this->y -= static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector2& Vector2::operator*=(const T& other) {
	this->x *= static_cast<float>(other);
	this->y *= static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector2& Vector2::operator/=(const T& other) {
	assert(other != 0 && "Division by zero error in Vector2::operator/=");
	this->x /= static_cast<float>(other);
	this->y /= static_cast<float>(other);
	return *this;
}

constexpr Vector2 Vector2::operator+(const Vector2& other) const {
	return {x + other.x, y + other.y};
}
constexpr Vector2 Vector2::operator-(const Vector2& other) const {
	return {x - other.x, y - other.y};
}
constexpr Vector2& Vector2::operator+=(const Vector2& other) {
	x += other.x;
	y += other.y;
	return *this;
}
constexpr Vector2& Vector2::operator-=(const Vector2& other) {
	x -= other.x;
	y -= other.y;
	return *this;
}
constexpr Vector2 Vector2::Multiply(const Vector2& a, const Vector2& b) {
	return {a.x * b.x, a.y * b.y};
}

constexpr Vector2 Vector2::operator-() const {
	return { -x, -y };
}
constexpr Vector2& Vector2::operator++() {
	x++;
	y++;
	return *this;
}
constexpr Vector2& Vector2::operator--() {
	x--;
	y--;
	return *this;
}


struct Vector3 {
	float x;
	float y;
	float z;

	constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	constexpr Vector3(float nX, float nY, float nZ) : x(nX), y(nY), z(nZ) {}

	static const Vector3 Zero;
	static const Vector3 One;
	static const Vector3 UnitX;
	static const Vector3 UnitY;
	static const Vector3 UnitZ;

	template<std::convertible_to<float> T>
	constexpr Vector3 operator+(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector3 operator-(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector3 operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector3 operator/(const T& other) const;

	template<std::convertible_to<float> T>
	constexpr Vector3& operator+=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector3& operator-=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector3& operator*=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector3& operator/=(const T& other);

	constexpr Vector3 operator+(const Vector3& other) const;
	constexpr Vector3 operator-(const Vector3& other) const;
	constexpr Vector3& operator+=(const Vector3& other);
	constexpr Vector3& operator-=(const Vector3& other);
	constexpr static Vector3 Multiply(const Vector3&, const Vector3&);

	constexpr bool operator==(const Vector3& other) const = default;

	constexpr Vector3 operator-() const;
	constexpr Vector3& operator++();
	constexpr Vector3& operator--();
};
inline constexpr Vector3 Vector3::Zero{ 0.0f, 0.0f, 0.0f };
inline constexpr Vector3 Vector3::One{ 1.0f, 1.0f, 1.0f };
inline constexpr Vector3 Vector3::UnitX{ 1.0f, 0.0f, 0.0f };
inline constexpr Vector3 Vector3::UnitY{ 0.0f, 1.0f, 0.0f };
inline constexpr Vector3 Vector3::UnitZ{ 0.0f, 0.0f, 1.0f };

template<std::convertible_to<float> T>
constexpr Vector3 Vector3::operator+(const T& other) const {
	Vector3 num;
	num.x = x + static_cast<float>(other);
	num.y = y + static_cast<float>(other);
	num.z = z + static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector3 Vector3::operator-(const T& other) const {
	Vector3 num;
	num.x = x - static_cast<float>(other);
	num.y = y - static_cast<float>(other);
	num.z = z - static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector3 Vector3::operator*(const T& other) const {
	Vector3 num;
	num.x = x * static_cast<float>(other);
	num.y = y * static_cast<float>(other);
	num.z = z * static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector3 operator*(const T& scalar, const Vector3& v) {
	return v * static_cast<float>(scalar);
}
template<std::convertible_to<float> T>
constexpr Vector3 Vector3::operator/(const T& other) const {
	assert(other != 0 && "Division by zero error in Vector3::operator/");
	Vector3 num;
	num.x = x / static_cast<float>(other);
	num.y = y / static_cast<float>(other);
	num.z = z / static_cast<float>(other);
	return num;
}

template<std::convertible_to<float> T>
constexpr Vector3& Vector3::operator+=(const T& other) {
	x += static_cast<float>(other);
	y += static_cast<float>(other);
	z += static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector3& Vector3::operator-=(const T& other) {
	x -= static_cast<float>(other);
	y -= static_cast<float>(other);
	z -= static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector3& Vector3::operator*=(const T& other) {
	x *= static_cast<float>(other);
	y *= static_cast<float>(other);
	z *= static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector3& Vector3::operator/=(const T& other) {
	assert(other != 0 && "Division by zero error in Vector3::operator/=");
	x /= static_cast<float>(other);
	y /= static_cast<float>(other);
	z /= static_cast<float>(other);
	return *this;
}

constexpr Vector3 Vector3::operator+(const Vector3& other) const {
	Vector3 num;
	num.x = x + other.x;
	num.y = y + other.y;
	num.z = z + other.z;
	return num;
}
constexpr Vector3 Vector3::operator-(const Vector3& other) const {
	Vector3 num;
	num.x = x - other.x;
	num.y = y - other.y;
	num.z = z - other.z;
	return num;
}
constexpr Vector3& Vector3::operator+=(const Vector3& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}
constexpr Vector3& Vector3::operator-=(const Vector3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}
constexpr Vector3 Vector3::Multiply(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
		v1.x * v2.x,
		v1.y * v2.y,
		v1.z * v2.z
	};
	return result;
}

constexpr Vector3 Vector3::operator-() const {
	return { -x, -y, -z };
}
constexpr Vector3& Vector3::operator++() {
	x++;
	y++;
	z++;
	return *this;
}
constexpr Vector3& Vector3::operator--() {
	x--;
	y--;
	z--;
	return *this;
}

struct Vector4 {
	float x;
	float y;
	float z;
	float w;

	constexpr Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	constexpr Vector4(float nX, float nY, float nZ, float nW) : x(nX), y(nY), z(nZ), w(nW) {}

	static const Vector4 Zero;
	static const Vector4 One;
	static const Vector4 UnitX;
	static const Vector4 UnitY;
	static const Vector4 UnitZ;
	static const Vector4 UnitW;

	template<std::convertible_to<float> T>
	constexpr Vector4 operator+(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector4 operator-(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector4 operator*(const T& other) const;
	template<std::convertible_to<float> T>
	constexpr Vector4 operator/(const T& other) const;

	template<std::convertible_to<float> T>
	constexpr Vector4& operator+=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector4& operator-=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector4& operator*=(const T& other);
	template<std::convertible_to<float> T>
	constexpr Vector4& operator/=(const T& other);

	constexpr Vector4 operator+(const Vector4& other) const;
	constexpr Vector4 operator-(const Vector4& other) const;
	constexpr Vector4& operator+=(const Vector4& other);
	constexpr Vector4& operator-=(const Vector4& other);
	constexpr static Vector4 Multiply(const Vector4&, const Vector4&);

	constexpr bool operator==(const Vector4& other) const = default;

	constexpr Vector4 operator-() const;
	constexpr Vector4& operator++();
	constexpr Vector4& operator--();
};
inline constexpr Vector4 Vector4::Zero{ 0.0f, 0.0f, 0.0f, 0.0f };
inline constexpr Vector4 Vector4::One{ 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Vector4 Vector4::UnitX{ 1.0f, 0.0f, 0.0f, 0.0f };
inline constexpr Vector4 Vector4::UnitY{ 0.0f, 1.0f, 0.0f, 0.0f };
inline constexpr Vector4 Vector4::UnitZ{ 0.0f, 0.0f, 1.0f, 0.0f };
inline constexpr Vector4 Vector4::UnitW{ 0.0f, 0.0f, 0.0f, 1.0f };

template<std::convertible_to<float> T>
constexpr Vector4 Vector4::operator+(const T& other) const {
	Vector4 num;
	num.x = x + static_cast<float>(other);
	num.y = y + static_cast<float>(other);
	num.z = z + static_cast<float>(other);
	num.w = w + static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector4 Vector4::operator-(const T& other) const {
	Vector4 num;
	num.x = x - static_cast<float>(other);
	num.y = y - static_cast<float>(other);
	num.z = z - static_cast<float>(other);
	num.w = w - static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector4 Vector4::operator*(const T& other) const {
	Vector4 num;
	num.x = x * static_cast<float>(other);
	num.y = y * static_cast<float>(other);
	num.z = z * static_cast<float>(other);
	num.w = w * static_cast<float>(other);
	return num;
}
template<std::convertible_to<float> T>
constexpr Vector4 operator*(const T& scalar, const Vector4& v) {
	return v * static_cast<float>(scalar);
}
template<std::convertible_to<float> T>
constexpr Vector4 Vector4::operator/(const T& other) const {
	assert(other != 0 && "Division by zero error in Vector4::operator/");
	Vector4 num;
	num.x = x / static_cast<float>(other);
	num.y = y / static_cast<float>(other);
	num.z = z / static_cast<float>(other);
	num.w = w / static_cast<float>(other);
	return num;
}

template<std::convertible_to<float> T>
constexpr Vector4& Vector4::operator+=(const T& other) {
	x += static_cast<float>(other);
	y += static_cast<float>(other);
	z += static_cast<float>(other);
	w += static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector4& Vector4::operator-=(const T& other) {
	x -= static_cast<float>(other);
	y -= static_cast<float>(other);
	z -= static_cast<float>(other);
	w -= static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector4& Vector4::operator*=(const T& other) {
	x *= static_cast<float>(other);
	y *= static_cast<float>(other);
	z *= static_cast<float>(other);
	w *= static_cast<float>(other);
	return *this;
}
template<std::convertible_to<float> T>
constexpr Vector4& Vector4::operator/=(const T& other) {
	assert(other != 0 && "Division by zero error in Vector4::operator/=");
	x /= static_cast<float>(other);
	y /= static_cast<float>(other);
	z /= static_cast<float>(other);
	w /= static_cast<float>(other);
	return *this;
}

constexpr Vector4 Vector4::operator+(const Vector4& other) const {
	Vector4 num;
	num.x = x + other.x;
	num.y = y + other.y;
	num.z = z + other.z;
	num.w = w + other.w;
	return num;
}
constexpr Vector4 Vector4::operator-(const Vector4& other) const {
	Vector4 num;
	num.x = x - other.x;
	num.y = y - other.y;
	num.z = z - other.z;
	num.w = w - other.w;
	return num;
}
constexpr Vector4& Vector4::operator+=(const Vector4& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}
constexpr Vector4& Vector4::operator-=(const Vector4& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}
constexpr Vector4 Vector4::Multiply(const Vector4& a, const Vector4& b) {
	Vector4 result = {
		a.x * b.x,
		a.y * b.y,
		a.z * b.z,
		a.w * b.w
	};
	return result;
}

constexpr Vector4 Vector4::operator-() const {
	return { -x, -y, -z, -w };
}
constexpr Vector4& Vector4::operator++() {
	x++;
	y++;
	z++;
	w++;
	return *this;
}
constexpr Vector4& Vector4::operator--() {
	x--;
	y--;
	z--;
	w--;
	return *this;
}

} // namespace Cake
