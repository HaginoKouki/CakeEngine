#pragma once
/*====================================
 *
 * GameObject とコンポーネントプールを所有するシーン。
 *
 * GameObject は index + generation 方式のプールで管理し、外へは GameObjectId を
 * 払い出す。コンポーネントの実体は型ごとの ComponentPool が持ち、Scene はそれを
 * 型ID をキーにまとめて所有する。
 *
 * 【ポインタの寿命】
 * Find() や GetComponent() が返すポインタは、次にオブジェクトやコンポーネントを
 * 追加するまでの間だけ有効。vector の再確保で引っ越すため、フレームをまたいで
 * 保持してはいけない。持ち回るのは GameObjectId。
 *
 * 【更新】
 * ワールド行列の更新は UpdateTransforms() がルートから再帰で行う。
 * 親がダーティなら子も強制的に再計算する。
 *
 * ====================================*/
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Transform.h"

#include "Engine/Scene/Object/ComponentPool.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Scene {
private:
	std::vector<GameObject> objects_;
	std::vector<uint32_t> generations_; // objects_ と同じ添字。スロットの世代.
	std::vector<uint8_t> alive_;        // objects_ と同じ添字。走査時に空きを飛ばすため.
	std::vector<uint32_t> freeList_;    // 破棄で空いた添字。再利用する.

	// 型ID → その型のコンポーネントプール.
	std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>> pools_;

	// 親を持たないオブジェクト。UpdateTransforms の起点.
	std::vector<GameObjectId> roots_;

public:
	// ===== オブジェクトの生成・破棄 =====

	// parent が無効なら root として作る.
	GameObjectId CreateGameObject(const std::string& name = "GameObject", GameObjectId parent = {});

	// 子も再帰的に破棄する.
	void DestroyGameObject(GameObjectId id);
	// すべてのオブジェクトを破棄する.
	void Clear();

	bool IsAlive(GameObjectId id) const;

	// 【注意】戻り値のポインタは次の CreateGameObject まで有効.
	GameObject* Find(GameObjectId id);
	const GameObject* Find(GameObjectId id) const;

	// ===== コンポーネント =====

	// 同じ型を重複して持たせることはできない（既に持っていれば既存のものを返す）.
	template <class T>
	T* AddComponent(GameObjectId id);

	template <class T>
	T* GetComponent(GameObjectId id);

	template <class T>
	bool RemoveComponent(GameObjectId id);

	// 型ごとのプールを取得する（無ければ作る）。System が全件走査するのに使う.
	template <class T>
	ComponentPool<T>* GetPool();

	// ===== 階層 =====

	// parent が無効なら root へ移す。循環する指定は無視する.
	// worldPositionStays が true なら、張り替え後も見た目の位置を保つ.
	// false なら local をそのまま維持し、親の変換に従って移動する.
	void SetParent(GameObjectId child, GameObjectId parent, bool worldPositionStays = true);

	const std::vector<GameObjectId>& GetRoots() const { return roots_; }

	// ===== 更新 =====

	// ルートから再帰でワールド行列を確定する.
	void UpdateTransforms();

	size_t GetObjectCount() const { return objects_.size() - freeList_.size(); }

private:
	void DestroyRecursive(GameObjectId id);
	void DetachFromParent(GameObjectId id);
	void UpdateTransformsRecursive(GameObjectId id, const Matrix4x4& parentWorld, bool parentDirty);
};

#pragma region テンプレート実装

template <class T>
ComponentPool<T>* Scene::GetPool() {
	const ComponentTypeId type = GetComponentTypeId<T>();
	auto it = pools_.find(type);
	if (it == pools_.end()) {
		it = pools_.emplace(type, std::make_unique<ComponentPool<T>>()).first;
	}
	return static_cast<ComponentPool<T>*>(it->second.get());
}

template <class T>
T* Scene::AddComponent(GameObjectId id) {
	GameObject* object = Find(id);
	if (object == nullptr) {
		return nullptr;
	}

	// 同じ型を2つ持つと GetComponent の戻り値が一意に決まらないので、既存を返す.
	const ComponentTypeId type = GetComponentTypeId<T>();
	if (object->FindComponentRef(type) != nullptr) {
		return GetComponent<T>(id);
	}

	ComponentPool<T>* pool = GetPool<T>();
	const ComponentRef ref = pool->Add(id);
	object->components_.push_back(ref);
	return pool->Get(ref.index, ref.generation);
}

template <class T>
T* Scene::GetComponent(GameObjectId id) {
	GameObject* object = Find(id);
	if (object == nullptr) {
		return nullptr;
	}
	const ComponentRef* ref = object->FindComponentRef(GetComponentTypeId<T>());
	if (ref == nullptr) {
		return nullptr;
	}
	return GetPool<T>()->Get(ref->index, ref->generation);
}

template <class T>
bool Scene::RemoveComponent(GameObjectId id) {
	GameObject* object = Find(id);
	if (object == nullptr) {
		return false;
	}

	const ComponentTypeId type = GetComponentTypeId<T>();
	for (size_t i = 0; i < object->components_.size(); ++i) {
		if (object->components_[i].type != type) {
			continue;
		}
		const ComponentRef ref = object->components_[i];
		object->components_.erase(object->components_.begin() + i);
		GetPool<T>()->Remove(ref.index, ref.generation);
		return true;
	}
	return false;
}

#pragma endregion

} // namespace Cake
