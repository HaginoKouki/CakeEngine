#pragma once
/*====================================
 *
 * シーンに配置される実体。
 * 自身は「識別子・名前・Transform・コンポーネント参照のリスト」だけを持つ器で、
 * コンポーネントの実体は型ごとの ComponentPool が所有する。
 *
 * Transform は組み込みフィールドとして必ず1つ持つ（付け外しできない）。
 * それ以外のコンポーネントは components_ に ComponentRef として並ぶ。
 * 通常3〜5個なので線形探索で十分（unordered_map はこの規模だと割に合わない）。
 *
 * 生成・破棄・親子の張り替えは Scene が行う（friend 指定）。
 *
 * ====================================*/
#include <string>
#include <vector>

#include "Engine/Scene/Component/TransformComponent.h"
#include "Engine/Scene/Object/ComponentPool.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class GameObject {
	friend class Scene;

private:
	GameObjectId id_;
	std::string name_;
	TransformComponent transform_;
	std::vector<ComponentRef> components_;
	bool active_ = true;

public:
	GameObjectId GetId() const { return id_; }

	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	TransformComponent& GetTransform() { return transform_; }
	const TransformComponent& GetTransform() const { return transform_; }

	bool IsActive() const { return active_; }
	void SetActive(bool active) { active_ = active; }

	// 保持しているコンポーネント参照の一覧。インスペクタが全件を並べるのに使う.
	const std::vector<ComponentRef>& GetComponentRefs() const { return components_; }

	// 指定した型の参照を探す。持っていなければ nullptr.
	const ComponentRef* FindComponentRef(ComponentTypeId type) const {
		for (const ComponentRef& ref : components_) {
			if (ref.type == type) {
				return &ref;
			}
		}
		return nullptr;
	}
};

} // namespace Cake
