#include "LightSystem.h"

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Render/Light.h"

#include "Engine/Scene/Component/LightComponent.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {

bool CollectDirectionalLight(Scene& scene, DirectionalLight& out) {
	bool found = false;
	int bestPriority = 0;
	DirectionalLight result{};

	scene.GetPool<LightComponent>()->ForEach(
		[&](GameObjectId owner, LightComponent& light) {
			if (!light.enabled) {
				return;
			}
			const GameObject* object = scene.Find(owner);
			if (object == nullptr || !object->IsActive()) {
				return;
			}
			// 最初に見つかったものを暫定採用し、以降は priority がより小さいものだけ置き換える.
			if (found && light.priority >= bestPriority) {
				return;
			}

			// world は UpdateTransforms が確定済み。ここでは計算しない.
		    // 行ベクトル規約なので2行目がローカル +Z 軸。スケールが乗るため正規化する.
			const Matrix4x4& world = object->GetTransform().GetWorldMatrix();
			const Vector3 forward = {world.m[2][0], world.m[2][1], world.m[2][2]};

			result.color = light.color;
			result.direction = Vector3::Normalize(forward);
			result.intensity = light.intensity;

			bestPriority = light.priority;
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
