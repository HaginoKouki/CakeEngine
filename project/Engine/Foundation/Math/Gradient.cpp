#include "Gradient.h"

#include <algorithm>
#include "Engine/Foundation/Math/Easing.h"

namespace Cake {
namespace {

// 2つのキーの間で t がどの割合にあるか（0〜1）.
float Ratio(float startTime, float endTime, float t) {
	const float span = endTime - startTime;
	return (span > 0.0f) ? (t - startTime) / span : 1.0f; // 同じ位置のキーが並んでいるときの0除算を避ける.
}

// 昇順に並んだ色のキー列から、t の位置の色を求める.
Vector3 EvaluateColor(const ColorKey* keys, int count, float t) {
	if (t <= keys[0].time) {
		return keys[0].color;
	}
	for (int i = 0; i + 1 < count; ++i) {
		const ColorKey& a = keys[i];
		const ColorKey& b = keys[i + 1];
		if (t < b.time) {
			return Easing::Lerp<Vector3>(a.color, b.color, Ratio(a.time, b.time, t));
		}
	}
	return keys[count - 1].color;
}

// 昇順に並んだ透明度のキー列から、t の位置の透明度を求める.
float EvaluateAlpha(const AlphaKey* keys, int count, float t) {
	if (t <= keys[0].time) {
		return keys[0].alpha;
	}
	for (int i = 0; i + 1 < count; ++i) {
		const AlphaKey& a = keys[i];
		const AlphaKey& b = keys[i + 1];
		if (t < b.time) {
			return Easing::Lerp<float>(a.alpha, b.alpha, Ratio(a.time, b.time, t));
		}
	}
	return keys[count - 1].alpha;
}

} // namespace

int Gradient::ClampKeyCount(int count) {
	return std::clamp(count, 1, kMaxKeys);
}

Vector4 Gradient::Evaluate(float t) const {
	const Vector3 color = EvaluateColor(colorKeys, ClampKeyCount(colorKeyCount), t);
	const float alpha = EvaluateAlpha(alphaKeys, ClampKeyCount(alphaKeyCount), t);
	return {color.x, color.y, color.z, alpha};
}

Gradient Gradient::Sorted() const {
	Gradient result = *this;
	result.colorKeyCount = ClampKeyCount(colorKeyCount);
	result.alphaKeyCount = ClampKeyCount(alphaKeyCount);

	// キーは最大8個なので、毎フレーム並べ替えても負担は無視できる.
	auto byTime = [](const auto& a, const auto& b) { return a.time < b.time; };
	std::sort(result.colorKeys, result.colorKeys + result.colorKeyCount, byTime);
	std::sort(result.alphaKeys, result.alphaKeys + result.alphaKeyCount, byTime);
	return result;
}

} // namespace Cake
