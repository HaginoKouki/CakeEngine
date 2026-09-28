#include "GizmoDrawList.h"

#include <cmath>
#include <numbers>

namespace Cake {

void GizmoDrawList::AddLine(const Vector3& a, const Vector3& b, const Vector4& color) {
	vertices_.push_back(GizmoVertex{Vector4{a.x, a.y, a.z, 1.0f}, color});
	vertices_.push_back(GizmoVertex{Vector4{b.x, b.y, b.z, 1.0f}, color});
}

void GizmoDrawList::AddRay(const Vector3& origin, const Vector3& direction, const Vector4& color) {
	AddLine(origin, origin + direction, color);
}

void GizmoDrawList::AddGrid(const GizmoGrid& grid) {
	if (!grid.enabled || grid.halfCount <= 0 || grid.cellSize <= 0.0f || grid.opacity <= 0.0f) {
		return;
	}

	const float extent = grid.cellSize * static_cast<float>(grid.halfCount);

	// 元の色は保ったまま、アルファだけ差し替える.
	// scale は「細線は主線よりさらに薄く」を1本の opacity から作るための係数.
	auto fade = [&](const Vector4& color, float scale) {
		return Vector4{color.x, color.y, color.z, grid.opacity * scale};
	};

	for (int i = -grid.halfCount; i <= grid.halfCount; ++i) {
		const float position = grid.cellSize * static_cast<float>(i);

		// X = position の線（Z方向に伸びる）。i==0 のこれがZ軸.
		AddLine(Vector3{position, 0.0f, -extent}, Vector3{position, 0.0f, extent}, GizmoColor::kGrid);
		// Z = position の線（X方向に伸びる）。i==0 のこれがX軸.
		AddLine(Vector3{-extent, 0.0f, position}, Vector3{extent, 0.0f, position}, GizmoColor::kGrid);
	}
}

void GizmoDrawList::AddWireBox(const Matrix4x4& world, const Vector3& center, const Vector3& size, const Vector4& color) {
	const Vector3 half = size * 0.5f;

	// ローカル8隅 → ワールドへ。順序は 下面0-3 / 上面4-7 で対応させる.
	Vector3 corner[8];
	for (int i = 0; i < 8; ++i) {
		const Vector3 local{
			center.x + ((i & 1) ? half.x : -half.x),
			center.y + ((i & 4) ? half.y : -half.y),
			center.z + ((i & 2) ? half.z : -half.z),
		};
		corner[i] = Vector3::Transform(local, world);
	}

	// 下面（0,1,3,2 の順に一周）.
	AddLine(corner[0], corner[1], color);
	AddLine(corner[1], corner[3], color);
	AddLine(corner[3], corner[2], color);
	AddLine(corner[2], corner[0], color);
	// 上面.
	AddLine(corner[4], corner[5], color);
	AddLine(corner[5], corner[7], color);
	AddLine(corner[7], corner[6], color);
	AddLine(corner[6], corner[4], color);
	// 柱.
	for (int i = 0; i < 4; ++i) {
		AddLine(corner[i], corner[i + 4], color);
	}
}

void GizmoDrawList::AddWireSphere(const Matrix4x4& world, const Vector3& center, float radius, const Vector4& color, uint32_t segments) {
	// world から受け取るのは中心の位置だけ。
	// CollisionSystem 側も「中心は world で移す・半径は素のまま」で判定しているので、
	// ここで world のスケールを掛けると絵と判定が食い違う.
	AddWireSphere(Vector3::Transform(center, world), radius, color, segments);
}

void GizmoDrawList::AddWireSphere(const Vector3& center, float radius, const Vector4& color, uint32_t segments) {
	// 半径0以下は判定にも参加しないので、線も出さない.
	if (radius <= 0.0f) {
		return;
	}
	if (segments < 3) {
		segments = 3;
	}
	const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segments);

	// XY / YZ / ZX の3枚。軸の組み合わせをラムダで差し替えるだけにする.
	// 円はワールド軸に沿って描く。球は回しても形が変わらないため、
	// 向きを持たせても情報が増えない（むしろ回転が効くと誤解させる）.
	auto drawCircle = [&](int axisA, int axisB) {
		Vector3 previous{};
		for (uint32_t i = 0; i <= segments; ++i) {
			const float theta = step * static_cast<float>(i);
			float position[3] = {center.x, center.y, center.z};
			position[axisA] += std::cos(theta) * radius;
			position[axisB] += std::sin(theta) * radius;

			const Vector3 point{position[0], position[1], position[2]};
			if (i > 0) {
				AddLine(previous, point, color);
			}
			previous = point;
		}
	};
	drawCircle(0, 1);
	drawCircle(1, 2);
	drawCircle(2, 0);
}

void GizmoDrawList::AddFrustum(const Matrix4x4& viewProjection, const Vector4& color) {
	const Matrix4x4 inverse = Matrix4x4::Inverse(viewProjection);

	// DirectXのNDCは xy が -1..1、z が 0..1（0が手前）.
	// Vector3::Transform は w で割ってくれるので、そのまま隅が戻る.
	const Vector2 ndc[4] = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};

	Vector3 nearCorner[4];
	Vector3 farCorner[4];
	for (int i = 0; i < 4; ++i) {
		nearCorner[i] = Vector3::Transform(Vector3{ndc[i].x, ndc[i].y, 0.0f}, inverse);
		farCorner[i] = Vector3::Transform(Vector3{ndc[i].x, ndc[i].y, 1.0f}, inverse);
	}

	for (int i = 0; i < 4; ++i) {
		const int next = (i + 1) % 4;
		AddLine(nearCorner[i], nearCorner[next], color);
		AddLine(farCorner[i], farCorner[next], color);
		AddLine(nearCorner[i], farCorner[i], color);
	}
}

void GizmoDrawList::AddAxes(const Matrix4x4& world, float length) {
	const Vector3 origin{world.m[3][0], world.m[3][1], world.m[3][2]};

	// 各行が回転＋スケール込みのローカル軸。正規化して長さを揃える.
	const Vector3 axisX = Vector3::Normalize({world.m[0][0], world.m[0][1], world.m[0][2]});
	const Vector3 axisY = Vector3::Normalize({world.m[1][0], world.m[1][1], world.m[1][2]});
	const Vector3 axisZ = Vector3::Normalize({world.m[2][0], world.m[2][1], world.m[2][2]});

	AddLine(origin, origin + axisX * length, Vector4{1.0f, 0.25f, 0.25f, 1.0f});
	AddLine(origin, origin + axisY * length, Vector4{0.35f, 1.0f, 0.25f, 1.0f});
	AddLine(origin, origin + axisZ * length, Vector4{0.30f, 0.55f, 1.0f, 1.0f});
}

} // namespace Cake
