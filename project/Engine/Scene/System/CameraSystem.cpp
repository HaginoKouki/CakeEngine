#include "CameraSystem.h"

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Render/CameraView.h"

#include "Engine/Scene/Component/CameraComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {

bool CollectMainCamera(Scene& scene, uint32_t targetWidth, uint32_t targetHeight, CameraView& out) {
	bool found = false;
	int bestPriority = 0;
	CameraView result{};

	scene.GetPool<CameraComponent>()->ForEach(
		[&](GameObjectId owner, CameraComponent& camera) {
			if (!camera.enabled) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}
			// 最初に見つかったものを暫定採用し、以降は priority がより小さいものだけ置き換える.
			if (found && camera.priority >= bestPriority) {
				return;
			}

			// world は UpdateTransforms が確定済み。ここでは計算しない.
		    // 行ベクトル規約なので2行目がローカル +Z 軸。スケールが乗るため正規化する.
			const Matrix4x4& world = object->GetTransform().GetWorldMatrix();

			result = MakeCameraView(world, camera.fovYDegree, camera.nearClip, camera.farClip, targetWidth, targetHeight);

			bestPriority = camera.priority;
			found = true;
		}
	);

	if (!found) {
		return false;
	}
	out = result;
	return true;
}

} // namespace Cake
