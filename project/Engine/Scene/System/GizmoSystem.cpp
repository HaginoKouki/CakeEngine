#include "GizmoSystem.h"

#include <cmath>

#include "Engine/Render/CameraView.h"
#include "Engine/Render/Gizmo/GizmoDrawList.h"

#include "Engine/Scene/Component/Collider/BoxColliderComponent.h"
#include "Engine/Scene/Component/CameraComponent.h"
#include "Engine/Scene/Component/LightComponent.h"
#include "Engine/Scene/Component/Collider/SphereColliderComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {
namespace {

// 生きていて有効なオブジェクトだけ返す。死んでいれば nullptr.
const GameObject* FindActive(Scene& scene, GameObjectId owner) {
	const GameObject* object = scene.Find(owner);
	if (object == nullptr || !object->IsActive()) {
		return nullptr;
	}
	return object;
}

// 選択中なら強調色へ差し替える.
Vector4 PickColor(const GizmoSettings& settings, GameObjectId owner, const Vector4& normal) {
	if (settings.highlightSelected && settings.selected.IsValid() && owner == settings.selected) {
		return GizmoColor::kSelected;
	}
	return normal;
}

} // namespace

void CollectSceneGizmos(Scene& scene, const GizmoSettings& settings, GizmoDrawList& out) {
	if (!settings.enabled) {
		return;
	}

	// グリッド.
	out.AddGrid(settings.grid);

	// --- コライダー ---
	if (settings.showColliders) {
		scene.GetPool<BoxColliderComponent>()->ForEach(
			[&](GameObjectId owner, BoxColliderComponent& box) {
				const GameObject* object = FindActive(scene, owner);
				if (object == nullptr) {
					return;
				}
				out.AddWireBox(
					object->GetTransform().GetWorldMatrix(),
					box.center, box.size,
					PickColor(settings, owner, GizmoColor::kCollider)
				);
			}
		);

		scene.GetPool<SphereColliderComponent>()->ForEach(
			[&](GameObjectId owner, SphereColliderComponent& sphere) {
				const GameObject* object = FindActive(scene, owner);
				if (object == nullptr) {
					return;
				}
				out.AddWireSphere(
					object->GetTransform().GetWorldMatrix(),
					sphere.center, sphere.radius,
					PickColor(settings, owner, GizmoColor::kCollider)
				);
			}
		);
	}

	// --- カメラ（視錐台）---
	if (settings.showCameras) {
		scene.GetPool<CameraComponent>()->ForEach(
			[&](GameObjectId owner, CameraComponent& camera) {
				const GameObject* object = FindActive(scene, owner);
				if (object == nullptr) {
					return;
				}

				// 視錐台の形は解像度でなく比率だけで決まる。
			    // それでも MakeCameraView を通すのは、ゲーム本番と同じ経路にしておくため.
				constexpr uint32_t kHeight = 1000;
				const uint32_t width = static_cast<uint32_t>(std::lround(settings.gameAspect * kHeight));

				const CameraView view = MakeCameraView(
					object->GetTransform().GetWorldMatrix(),
					camera.fovYDegree, camera.nearClip, camera.farClip,
					width, kHeight
				);

				// far が極端に大きいと画面いっぱいの線になって邪魔なので、表示用に切り詰める.
				out.AddFrustum(
					view.viewMatrix * view.projectionMatrix,
					PickColor(settings, owner, GizmoColor::kCamera)
				);
			}
		);
	}

	// --- ライト（向き）---
	if (settings.showLights) {
		scene.GetPool<LightComponent>()->ForEach(
			[&](GameObjectId owner, LightComponent& light) {
				const GameObject* object = FindActive(scene, owner);
				if (object == nullptr) {
					return;
				}
				const Matrix4x4& world = object->GetTransform().GetWorldMatrix();
				const Vector3 origin{world.m[3][0], world.m[3][1], world.m[3][2]};
				const Vector3 forward = Vector3::Normalize({world.m[2][0], world.m[2][1], world.m[2][2]});

				const Vector4 color = PickColor(settings, owner, GizmoColor::kLight);
				out.AddWireSphere(Matrix4x4::MakeTranslateMatrix(origin), Vector3::Zero, 0.25f, color, 12);
				out.AddRay(origin, forward * 2.0f, color);
			}
		);
	}

	// --- 選択中のオブジェクトに軸を出す ---
	if (settings.highlightSelected && settings.selected.IsValid()) {
		if (const GameObject* object = scene.Find(settings.selected)) {
			out.AddAxes(object->GetTransform().GetWorldMatrix(), 1.0f);
		}
	}
}

} // namespace Cake
