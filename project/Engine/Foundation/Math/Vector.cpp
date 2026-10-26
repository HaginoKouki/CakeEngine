#include "Vector.h"

#include <numbers>
#include <cmath>

#include "VectorMath.h"

namespace Cake {

Vector2 Vector2::FromDegreeUnit(float degree) {
	Vector2 result;
	result.x = cosf(degree * std::numbers::pi_v<float> / 180.0f);
	result.y = sinf(degree * std::numbers::pi_v<float> / 180.0f);
	result = Math::Normalize(result);
	return result;
}

Vector2 Vector2::FromRadianUnit(float radian) {
	Vector2 result;
	result.x = cosf(radian);
	result.y = sinf(radian);
	result = Math::Normalize(result);
	return result;
}

} // namespace Cake
