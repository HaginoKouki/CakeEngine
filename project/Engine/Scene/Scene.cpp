#include "Scene.h"

#include <algorithm>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "Scene";
}

GameObjectId Scene::CreateGameObject(const std::string& name, GameObjectId parent) {
	uint32_t index = 0;
	if (!freeList_.empty()) {
		index = freeList_.back();
		freeList_.pop_back();
		objects_[index] = GameObject{};
		alive_[index] = 1;
	} else {
		index = static_cast<uint32_t>(objects_.size());
		objects_.emplace_back();
		generations_.push_back(1); // 世代は1始まり.
		alive_.push_back(1);
	}

	GameObjectId id;
	id.index = index;
	id.generation = generations_[index];

	objects_[index].id_ = id;
	objects_[index].name_ = name;

	if (parent.IsValid() && IsAlive(parent)) {
		SetParent(id, parent);
	} else {
		roots_.push_back(id);
	}
	return id;
}

bool Scene::IsAlive(GameObjectId id) const {
	return id.index < objects_.size()
		&& alive_[id.index] != 0
		&& generations_[id.index] == id.generation;
}

GameObject* Scene::Find(GameObjectId id) {
	if (!IsAlive(id)) {
		return nullptr;
	}
	return &objects_[id.index];
}

const GameObject* Scene::Find(GameObjectId id) const {
	if (!IsAlive(id)) {
		return nullptr;
	}
	return &objects_[id.index];
}

void Scene::DestroyGameObject(GameObjectId id) {
	if (!IsAlive(id)) {
		return;
	}
	// 親（または roots_）からの参照を先に切る。子は道連れなので個別に切らなくてよい.
	DetachFromParent(id);
	DestroyRecursive(id);
}
void Scene::Clear() {
	for (const GameObjectId& id : roots_) {
		DestroyRecursive(id);
	}
	roots_.clear();
	particleStore_.Clear();
}

void Scene::DestroyRecursive(GameObjectId id) {
	GameObject* object = Find(id);
	if (object == nullptr) {
		return;
	}

	// 子を先に破棄する。再帰中に children_ が変化しうるのでコピーしてから回す.
	const std::vector<GameObjectId> children = object->transform_.children_;
	for (GameObjectId child : children) {
		DestroyRecursive(child);
	}

	// 保持しているコンポーネントをプールから外す.
	for (const ComponentRef& ref : object->components_) {
		auto it = pools_.find(ref.type);
		if (it != pools_.end()) {
			it->second->Remove(ref.index, ref.generation);
		}
	}

	const uint32_t index = id.index;
	objects_[index] = GameObject{}; // 名前や子リストを解放する.
	alive_[index] = 0;
	// 世代を進めて、破棄前に配られたIDを無効化する。0 は無効値なので飛ばす.
	if (++generations_[index] == 0) {
		generations_[index] = 1;
	}
	freeList_.push_back(index);
}

void Scene::DetachFromParent(GameObjectId id) {
	GameObject* object = Find(id);
	if (object == nullptr) {
		return;
	}

	const GameObjectId parent = object->transform_.parent_;
	if (parent.IsValid()) {
		if (GameObject* parentObject = Find(parent)) {
			std::vector<GameObjectId>& siblings = parentObject->transform_.children_;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), id), siblings.end());
		}
	} else {
		roots_.erase(std::remove(roots_.begin(), roots_.end(), id), roots_.end());
	}
	object->transform_.parent_ = {};
}

void Scene::SetParent(GameObjectId child, GameObjectId parent, bool worldPositionStays) {
	if (!IsAlive(child)) {
		return;
	}

	// 循環の防止。parent 側の祖先をたどって child に行き着くなら不正な指定.
	GameObjectId cursor = parent;
	while (cursor.IsValid()) {
		if (cursor == child) {
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "親子関係が循環するため無視しました");
			return;
		}
		const GameObject* ancestor = Find(cursor);
		if (ancestor == nullptr) {
			break;
		}
		cursor = ancestor->transform_.parent_;
	}

	// 張り替え前のワールド行列を控える.
	Matrix4x4 worldBefore = Matrix4x4::Identity;
	if (worldPositionStays) {
		if (const GameObject* object = Find(child)) {
			worldBefore = object->transform_.GetWorldMatrix();
		}
	}

	DetachFromParent(child);

	GameObject* childObject = Find(child);
	if (childObject == nullptr) {
		return;
	}

	if (parent.IsValid() && IsAlive(parent)) {
		childObject->transform_.parent_ = parent;
		Find(parent)->transform_.children_.push_back(child);
	} else {
		childObject->transform_.parent_ = {};
		roots_.push_back(child);
	}

	if (worldPositionStays) {
		Matrix4x4 parentWorld = Matrix4x4::Identity;
		if (parent.IsValid()) {
			if (const GameObject* parentObject = Find(parent)) {
				parentWorld = parentObject->transform_.GetWorldMatrix();
			}
		}
		// world = local × parentWorld なので、local = world × inverse(parentWorld).
		const Matrix4x4 newLocal = worldBefore * Matrix4x4::Inverse(parentWorld);
		childObject->transform_.GetLocalMutable() = Math::DecomposeAffine(newLocal);
	}

	childObject->transform_.MarkDirty();
}

void Scene::UpdateTransforms() {
	// roots_ は再帰中に変化しないが、破棄済みIDが混ざっていても安全に飛ばせるようにしてある.
	for (GameObjectId root : roots_) {
		UpdateTransformsRecursive(root, Matrix4x4::Identity, false);
	}
}

void Scene::UpdateTransformsRecursive(GameObjectId id, const Matrix4x4& parentWorld, bool parentDirty) {
	GameObject* object = Find(id);
	if (object == nullptr) {
		return;
	}

	TransformComponent& transform = object->transform_;

	// 祖先が動いた場合は自分の値が変わっていなくても再計算が必要.
	const bool needUpdate = parentDirty || transform.IsDirty();
	if (needUpdate) {
		transform.UpdateWorld(parentWorld);
	}

	const Matrix4x4 world = transform.GetWorldMatrix();
	// 子の走査中に objects_ は変化しないので、children_ への参照は有効なまま.
	for (GameObjectId child : transform.GetChildren()) {
		UpdateTransformsRecursive(child, world, needUpdate);
	}
}

} // namespace Cake
