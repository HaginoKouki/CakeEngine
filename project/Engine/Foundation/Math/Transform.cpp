#include "Transform.h"

#include <cmath>

namespace Cake {

namespace {
Vector3 ExtractEuler(const Vector3& axisX, const Vector3& axisY, const Vector3& axisZ) {
	Vector3 rotate{};

	// axisX.z は -sin(Y) に相当する
	// ジンバルロックの判定のために、安全な範囲（-1.0〜1.0）にクランプ
	float sinY = -axisX.z;
	if (sinY > 1.0f)
		sinY = 1.0f;
	if (sinY < -1.0f)
		sinY = -1.0f;

	// Y軸の回転角を求める
	rotate.y = std::asin(sinY);

	// ジンバルロックのチェック (cos(Y) が 0 に非常に近い場合)
	if (std::abs(sinY) > 0.999f) {
		// ジンバルロック時はX軸とZ軸の回転が連動するため、
		// Z軸の回転を0と仮定してX軸の回転を算出する
		rotate.z = 0.0f;
		if (sinY > 0.0f) {
			// Y = 90度のとき
			rotate.x = std::atan2(axisY.x, axisY.y);
		} else {
			// Y = -90度のとき
			rotate.x = -std::atan2(axisY.x, axisY.y);
		}
	} else {
		// 通常ケース: axisX の成分からZ軸の回転を、axisY, axisZ の成分からX軸の回転を求める
		rotate.x = std::atan2(axisY.z, axisZ.z);
		rotate.z = std::atan2(axisX.y, axisX.x);
	}

	return rotate;
}

}

Transform Math::DecomposeAffine(const Matrix4x4& m) {
	Transform out{};

	// 平行移動は第4行そのもの.
	out.translate = {m.m[3][0], m.m[3][1], m.m[3][2]};

	// 各基底ベクトルの長さがスケール.
	Vector3 axisX = {m.m[0][0], m.m[0][1], m.m[0][2]};
	Vector3 axisY = {m.m[1][0], m.m[1][1], m.m[1][2]};
	Vector3 axisZ = {m.m[2][0], m.m[2][1], m.m[2][2]};

	out.scale = {Vector3::Length(axisX), Vector3::Length(axisY), Vector3::Length(axisZ)};

	// 行列式が負なら鏡映が含まれる。1軸に負号を寄せる.
	if (Vector3::DotProduct(Vector3::CrossProduct(axisX, axisY), axisZ) < 0.0f) {
		out.scale.x = -out.scale.x;
		axisX = -axisX;
	}

	// スケールを除いた正規直交基底が回転.
	if (out.scale.x != 0.0f) {
		axisX /= out.scale.x;
	}
	if (out.scale.y != 0.0f) {
		axisY /= out.scale.y;
	}
	if (out.scale.z != 0.0f) {
		axisZ /= out.scale.z;
	}

	// ここからオイラー角へ。順序は MakeAffineMatrix と揃えること.
	out.rotate = ExtractEuler(axisX, axisY, axisZ);
	return out;
}

}
