#include "CollisionSystem.h"

#include <cmath>
#include <vector>

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Scene/Component/BoxColliderComponent.h"
#include "Engine/Scene/Component/SphereColliderComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {
namespace {

// 2つの軸が平行だと、その外積は長さ0になって分離軸として使えない。
// 判定をごくわずかに太らせておくことで、0除算も取りこぼしも起こさずに済ませる.
constexpr float kParallelEpsilon = 1.0e-6f;

// これ以下のスケールが掛かった軸は向きが定まらない。
// 潰れた箱は判定そのものから外す（絵にも出ていないはずなので黙って飛ばす）.
constexpr float kMinAxisScale = 1.0e-6f;

enum class ShapeKind {
	Sphere,
	Box,
};

// 向き付きの直方体。axis は正規化済み、half はスケール適用後の半分の長さ.
struct Obb {
	Vector3 center{};
	Vector3 axis[3]{};
	float half[3]{};
};

// 判定に参加するコライダー1つぶん。
// 形の違いをここで吸収するので、総当たりのループは型を意識しない.
struct ColliderEntry {
	GameObjectId owner;
	ShapeKind kind = ShapeKind::Sphere;

	int layer = 1;
	int mask = -1;

	// 早期棄却用。形全体を包む球（Sphere ならそれ自身）.
	Vector3 center{};
	float boundRadius = 0.0f;

	float radius = 0.0f; // Sphere のみ使う.
	Obb obb{};           // Box のみ使う.

	// 結果の書き戻し先。コンポーネント本体のメンバを直接指す.
	int* hitCount = nullptr;
	GameObjectId* hits = nullptr;
	int maxHits = 0;
};

/*---------------------------------
 *
 * 小道具
 *
 ---------------------------------*/

// ローカル座標をワールドへ移す（平行移動は第4行）.
Vector3 TransformPoint(const Matrix4x4& m, const Vector3& p) {
	return {
		p.x * m.m[0][0] + p.y * m.m[1][0] + p.z * m.m[2][0] + m.m[3][0],
		p.x * m.m[0][1] + p.y * m.m[1][1] + p.z * m.m[2][1] + m.m[3][1],
		p.x * m.m[0][2] + p.y * m.m[1][2] + p.z * m.m[2][2] + m.m[3][2],
	};
}

// 行ベクトル規約なので、第i行がローカル軸i（スケール込み）.
Vector3 GetAxisRow(const Matrix4x4& m, int i) {
	return {m.m[i][0], m.m[i][1], m.m[i][2]};
}

// 溢れたら捨てる。配列外へ書かないことだけ守る.
void AddHit(const ColliderEntry& entry, GameObjectId other) {
	if (*entry.hitCount >= entry.maxHits) {
		return;
	}
	entry.hits[*entry.hitCount] = other;
	++(*entry.hitCount);
}

/*---------------------------------
 *
 * 形ごとの交差判定
 *
 ---------------------------------*/

// 球×球。平方根を取らずに比べる（毎フレーム数万回通るため）.
bool IntersectSphereSphere(const Vector3& centerA, float radiusA, const Vector3& centerB, float radiusB) {
	const Vector3 diff = centerB - centerA;
	const float sumRadius = radiusA + radiusB;
	return Vector3::DotProduct(diff, diff) <= sumRadius * sumRadius;
}

// 球×箱。箱のローカル軸へ射影し、はみ出した分だけを距離に足していく。
// 「箱の表面上で球の中心にいちばん近い点」までの距離を出しているのと同じ.
bool IntersectSphereObb(const Vector3& center, float radius, const Obb& box) {
	const Vector3 diff = center - box.center;

	float distanceSq = 0.0f;
	for (int i = 0; i < 3; ++i) {
		const float projection = Vector3::DotProduct(diff, box.axis[i]);
		const float excess = std::fabs(projection) - box.half[i];
		if (excess > 0.0f) {
			distanceSq += excess * excess;
		}
	}
	return distanceSq <= radius * radius;
}

// 箱×箱。分離軸判定（SAT）。
// 「ある軸へ両方を影として落としたとき、影が離れていれば重なっていない」を
// 15本の軸（Aの3軸・Bの3軸・両者の軸の外積9通り）で調べる。
// 1本でも離れていれば即 false、15本すべてで重なっていれば true.
bool IntersectObbObb(const Obb& a, const Obb& b) {
	// R[i][j] は「Aの軸i から見たBの軸j の傾き」.
	float R[3][3];
	float absR[3][3];
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			R[i][j] = Vector3::DotProduct(a.axis[i], b.axis[j]);
			absR[i][j] = std::fabs(R[i][j]) + kParallelEpsilon;
		}
	}

	// 中心間ベクトルを A のローカル軸へ移す.
	const Vector3 diff = b.center - a.center;
	const float t[3] = {
		Vector3::DotProduct(diff, a.axis[0]),
		Vector3::DotProduct(diff, a.axis[1]),
		Vector3::DotProduct(diff, a.axis[2]),
	};

	// --- 軸 = A の各軸 ---
	for (int i = 0; i < 3; ++i) {
		const float ra = a.half[i];
		const float rb = b.half[0] * absR[i][0] + b.half[1] * absR[i][1] + b.half[2] * absR[i][2];
		if (std::fabs(t[i]) > ra + rb) {
			return false;
		}
	}

	// --- 軸 = B の各軸 ---
	for (int j = 0; j < 3; ++j) {
		const float ra = a.half[0] * absR[0][j] + a.half[1] * absR[1][j] + a.half[2] * absR[2][j];
		const float rb = b.half[j];
		const float distance = t[0] * R[0][j] + t[1] * R[1][j] + t[2] * R[2][j];
		if (std::fabs(distance) > ra + rb) {
			return false;
		}
	}

	// --- 軸 = Ai × Bj（9通り）---
	// 添字の入れ替わりが規則的でないので、ループにまとめず素直に9本並べる.
	float ra = 0.0f;
	float rb = 0.0f;

	// A0 × B0.
	ra = a.half[1] * absR[2][0] + a.half[2] * absR[1][0];
	rb = b.half[1] * absR[0][2] + b.half[2] * absR[0][1];
	if (std::fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) {
		return false;
	}
	// A0 × B1.
	ra = a.half[1] * absR[2][1] + a.half[2] * absR[1][1];
	rb = b.half[0] * absR[0][2] + b.half[2] * absR[0][0];
	if (std::fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) {
		return false;
	}
	// A0 × B2.
	ra = a.half[1] * absR[2][2] + a.half[2] * absR[1][2];
	rb = b.half[0] * absR[0][1] + b.half[1] * absR[0][0];
	if (std::fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) {
		return false;
	}

	// A1 × B0.
	ra = a.half[0] * absR[2][0] + a.half[2] * absR[0][0];
	rb = b.half[1] * absR[1][2] + b.half[2] * absR[1][1];
	if (std::fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) {
		return false;
	}
	// A1 × B1.
	ra = a.half[0] * absR[2][1] + a.half[2] * absR[0][1];
	rb = b.half[0] * absR[1][2] + b.half[2] * absR[1][0];
	if (std::fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) {
		return false;
	}
	// A1 × B2.
	ra = a.half[0] * absR[2][2] + a.half[2] * absR[0][2];
	rb = b.half[0] * absR[1][1] + b.half[1] * absR[1][0];
	if (std::fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) {
		return false;
	}

	// A2 × B0.
	ra = a.half[0] * absR[1][0] + a.half[1] * absR[0][0];
	rb = b.half[1] * absR[2][2] + b.half[2] * absR[2][1];
	if (std::fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) {
		return false;
	}
	// A2 × B1.
	ra = a.half[0] * absR[1][1] + a.half[1] * absR[0][1];
	rb = b.half[0] * absR[2][2] + b.half[2] * absR[2][0];
	if (std::fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) {
		return false;
	}
	// A2 × B2.
	ra = a.half[0] * absR[1][2] + a.half[1] * absR[0][2];
	rb = b.half[0] * absR[2][1] + b.half[1] * absR[2][0];
	if (std::fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) {
		return false;
	}

	// 離れている軸が1本も無かった.
	return true;
}

// 形の組み合わせを振り分ける。呼ぶ側は種類を知らなくてよい.
bool Intersect(const ColliderEntry& a, const ColliderEntry& b) {
	if (a.kind == ShapeKind::Sphere && b.kind == ShapeKind::Sphere) {
		return IntersectSphereSphere(a.center, a.radius, b.center, b.radius);
	}
	if (a.kind == ShapeKind::Sphere) {
		return IntersectSphereObb(a.center, a.radius, b.obb);
	}
	if (b.kind == ShapeKind::Sphere) {
		return IntersectSphereObb(b.center, b.radius, a.obb);
	}
	return IntersectObbObb(a.obb, b.obb);
}

/*---------------------------------
 *
 * 収集
 *
 ---------------------------------*/

// 前フレームの結果を消しつつ、判定に参加するものだけ集める.
// 【重要】entry が持つのはコンポーネント本体へのポインタ。
// この後プールを触らない（追加も削除もしない）ので持ち回ってよい.
void CollectSpheres(Scene& scene, std::vector<ColliderEntry>& out) {
	scene.GetPool<SphereColliderComponent>()->ForEach(
		[&](GameObjectId owner, SphereColliderComponent& collider) {
			collider.hitCount = 0;

			if (!collider.enabled || collider.radius <= 0.0f) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}

			ColliderEntry entry;
			entry.owner = owner;
			entry.kind = ShapeKind::Sphere;
			entry.layer = collider.layer;
			entry.mask = collider.mask;
			entry.center = TransformPoint(object->GetTransform().GetWorldMatrix(), collider.center);
			entry.radius = collider.radius;
			entry.boundRadius = collider.radius;
			entry.hitCount = &collider.hitCount;
			entry.hits = collider.hits;
			entry.maxHits = SphereColliderComponent::kMaxHits;
			out.push_back(entry);
		}
	);
}

void CollectBoxes(Scene& scene, std::vector<ColliderEntry>& out) {
	scene.GetPool<BoxColliderComponent>()->ForEach(
		[&](GameObjectId owner, BoxColliderComponent& collider) {
			collider.hitCount = 0;

			if (!collider.enabled) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}

			const Matrix4x4& world = object->GetTransform().GetWorldMatrix();

			// 行の長さがその軸のスケール、正規化した向きがその軸の方向.
			Obb obb;
			obb.center = TransformPoint(world, collider.center);

			const float size[3] = {collider.size.x, collider.size.y, collider.size.z};
			float boundRadiusSq = 0.0f;

			for (int i = 0; i < 3; ++i) {
				const Vector3 row = GetAxisRow(world, i);
				const float scale = Vector3::Length(row);
				if (scale <= kMinAxisScale) {
					return; // 潰れた変換。向きが決まらないので参加させない.
				}
				obb.axis[i] = row / scale;
				// 負の size を入れられても壊れないよう絶対値で受ける.
				obb.half[i] = std::fabs(size[i]) * 0.5f * scale;
				boundRadiusSq += obb.half[i] * obb.half[i];
			}

			if (boundRadiusSq <= 0.0f) {
				return; // 全辺が0。判定のしようがない.
			}

			ColliderEntry entry;
			entry.owner = owner;
			entry.kind = ShapeKind::Box;
			entry.layer = collider.layer;
			entry.mask = collider.mask;
			entry.center = obb.center;
			entry.boundRadius = std::sqrt(boundRadiusSq); // 8隅までの距離＝対角の半分.
			entry.obb = obb;
			entry.hitCount = &collider.hitCount;
			entry.hits = collider.hits;
			entry.maxHits = BoxColliderComponent::kMaxHits;
			out.push_back(entry);
		}
	);
}

} // namespace

void RunCollisionDetection(Scene& scene) {
	std::vector<ColliderEntry> entries;
	entries.reserve(
		scene.GetPool<SphereColliderComponent>()->GetAliveCount() + scene.GetPool<BoxColliderComponent>()->GetAliveCount()
	);

	CollectSpheres(scene, entries);
	CollectBoxes(scene, entries);

	// 総当たり。同じ組を2回見ないよう j は i+1 から始める.
	for (size_t i = 0; i < entries.size(); ++i) {
		for (size_t j = i + 1; j < entries.size(); ++j) {
			const ColliderEntry& a = entries[i];
			const ColliderEntry& b = entries[j];

			// どちらも相手を見ていない組は調べない.
			const bool aWantsB = (a.mask & b.layer) != 0;
			const bool bWantsA = (b.mask & a.layer) != 0;
			if (!aWantsB && !bWantsA) {
				continue;
			}

			// 形を包む球で先に落とす。
			// 箱同士は分離軸を15本調べるので、届かない組をここで止める効果が大きい.
			const Vector3 diff = b.center - a.center;
			const float sumRadius = a.boundRadius + b.boundRadius;
			if (Vector3::DotProduct(diff, diff) > sumRadius * sumRadius) {
				continue;
			}

			if (!Intersect(a, b)) {
				continue;
			}

			if (aWantsB) {
				AddHit(a, b.owner);
			}
			if (bWantsA) {
				AddHit(b, a.owner);
			}
		}
	}
}

} // namespace Cake
