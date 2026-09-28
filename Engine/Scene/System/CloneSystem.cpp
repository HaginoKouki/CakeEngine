#include "CloneSystem.h"

#include <string>
#include <vector>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Math/Transform.h"

#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"

#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "CloneSystem";
}

GameObjectId DuplicateGameObject(Scene& scene, GameObjectId source, GameObjectId parent) {
	const GameObject* src = scene.Find(source);
	if (src == nullptr) {
		return {};
	}

	// CreateGameObject がプールを再確保すると src が無効になる。
	// 必要な情報はすべてここで値としてコピーしておくこと.
	const std::string name = src->GetName();
	const bool active = src->IsActive();
	const Transform local = src->GetTransform().GetLocalTransform();
	const std::vector<ComponentRef> refs = src->GetComponentRefs();
	const std::vector<GameObjectId> children = src->GetTransform().GetChildren();

	const GameObjectId created = scene.CreateGameObject(name, parent);
	if (!created.IsValid()) {
		return {};
	}

	if (GameObject* dst = scene.Find(created)) {
		dst->SetActive(active);
		dst->GetTransform().GetLocalMutable() = local;
	}

	// --- コンポーネントの複製 ---
	TypeRegistry& registry = TypeRegistry::GetInstance();
	for (const ComponentRef& ref : refs) {
		const ComponentTypeInfo* info = registry.Find(ref.type);
		if (info == nullptr) {
			DebugLog::GetInstance().Log(
				LogLevel::Warn, kLogCategory,
				"未登録の型は複製できません (RegisterAllComponents を確認してください)"
			);
			continue;
		}
		if (info->ops.copy == nullptr || info->ops.get == nullptr) {
			continue;
		}

		// get は毎回引き直す。前回の copy 内の AddComponent で
		// プールが再確保され、以前取ったポインタが無効になっている可能性がある.
		void* instance = info->ops.get(scene, source);
		if (instance != nullptr) {
			info->ops.copy(scene, created, instance);
		}
	}

	// --- 子を再帰的に複製する ---
	// children は先にコピー済みなので、再帰中に元の子リストが動いても安全.
	for (GameObjectId child : children) {
		DuplicateGameObject(scene, child, created);
	}

	return created;
}

} // namespace Cake
