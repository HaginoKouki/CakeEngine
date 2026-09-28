#include "TypeRegistry.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "TypeRegistry";

// 循環検出での訪問状態.
enum class VisitState : uint8_t {
	Unvisited,
	Visiting, // 今たどっている経路の上にある.
	Done,
};

using MutableTypeMap = std::unordered_map<ComponentTypeId, ComponentTypeInfo*>;

// 深さ優先で依存をたどり、経路上の型へ戻る依存（＝循環）を取り除く.
void RemoveCycles(
	ComponentTypeInfo& info,
	const MutableTypeMap& byId,
	std::unordered_map<ComponentTypeId, VisitState>& states
) {
	if (states[info.id] != VisitState::Unvisited) {
		return;
	}
	states[info.id] = VisitState::Visiting;

	std::erase_if(info.requiredTypes, [&](ComponentTypeId required) {
		auto it = byId.find(required);
		if (it == byId.end()) {
			return true; // 未登録（1段目で除去済みのはずだが念のため）.
		}
		if (states[required] == VisitState::Visiting) {
			DebugLog::GetInstance().Log(
				LogLevel::Error, kLogCategory,
				std::string("依存が循環しています: ") + info.type.name + " -> " + it->second->type.name + " (この依存を無効にしました)"
			);
			return true;
		}
		// ここで書き換わるのは別の型の requiredTypes だけ（自己依存は上で弾いている）.
		RemoveCycles(*it->second, byId, states);
		return false;
	});

	states[info.id] = VisitState::Done;
}

} // namespace

TypeRegistry& TypeRegistry::GetInstance() {
	static TypeRegistry instance;
	return instance;
}

void TypeRegistry::Store(std::unique_ptr<ComponentTypeInfo> info) {
	const std::string typeName = info->type.name;

	if (finalized_) {
		// 締め切り後に足すと、依存宣言が検査されないまま使われる.
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			"登録の締め切り後に登録しようとしたので無視します: " + typeName
				+ " (FinalizeRegistration より前に登録してください)"
		);
		return;
	}

	if (byName_.contains(typeName)) {
		// 同名の型が2つあるとシリアライズ時にどちらか判別できない.
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "型名が重複しています: " + typeName);
		return;
	}

	const ComponentTypeInfo* raw = info.get();
	storage_.push_back(std::move(info));
	byName_[typeName] = raw;
	byId_[raw->id] = raw;
	ordered_.push_back(raw);

	if (raw->ops.update != nullptr) {
		updateOrder_.push_back(raw);
		// stable_sort なので、同じ優先度の型は登録順を保つ.
		std::stable_sort(
			updateOrder_.begin(), updateOrder_.end(),
			[](const ComponentTypeInfo* a, const ComponentTypeInfo* b) {
				return a->priority < b->priority;
			}
		);
	}

	std::string message = "登録: " + typeName + " (プロパティ " + std::to_string(raw->type.properties.size()) + " 件";
	if (raw->ops.update != nullptr) {
		message += " / Update 優先度 " + std::to_string(raw->priority);
	}
	message += ")";
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, message);
}

const ComponentTypeInfo* TypeRegistry::Find(const std::string& typeName) const {
	auto it = byName_.find(typeName);
	if (it == byName_.end()) {
		return nullptr;
	}
	return it->second;
}

const ComponentTypeInfo* TypeRegistry::Find(ComponentTypeId id) const {
	auto it = byId_.find(id);
	if (it == byId_.end()) {
		return nullptr;
	}
	return it->second;
}

#pragma region 依存宣言

void* TypeRegistry::AddComponent(Scene& scene, GameObjectId id, const ComponentTypeInfo& info) const {
	return AddComponentRecursive(scene, id, info, 0);
}

void* TypeRegistry::AddComponentRecursive(
	Scene& scene, GameObjectId id, const ComponentTypeInfo& info, size_t depth
) const {
	if (info.ops.add == nullptr) {
		return nullptr;
	}
	// 登録されている型の数より深くたどるのは、循環しているときだけ.
	if (depth > storage_.size()) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			std::string("依存の追跡を打ち切りました（循環の可能性）: ") + info.type.name + " (全型の登録後に FinalizeRegistration を呼んでいるか確認してください)"
		);
		return nullptr;
	}

	// 依存先を先に付ける。インスペクタで依存先が上に並ぶ.
	// 先に付けておけば、最後に取る自分のポインタが後続の追加で無効化されることもない.
	for (ComponentTypeId required : info.requiredTypes) {
		if (const ComponentTypeInfo* requiredInfo = Find(required)) {
			AddComponentRecursive(scene, id, *requiredInfo, depth + 1);
		}
	}
	return info.ops.add(scene, id);
}

const ComponentTypeInfo* TypeRegistry::FindDependent(
	const Scene& scene, GameObjectId id, ComponentTypeId type
) const {
	const GameObject* object = scene.Find(id);
	if (object == nullptr) {
		return nullptr;
	}
	for (const ComponentRef& ref : object->GetComponentRefs()) {
		if (ref.type == type) {
			continue;
		}
		const ComponentTypeInfo* info = Find(ref.type);
		if (info == nullptr) {
			continue;
		}
		const std::vector<ComponentTypeId>& required = info->requiredTypes;
		if (std::find(required.begin(), required.end(), type) != required.end()) {
			return info;
		}
	}
	return nullptr;
}

void TypeRegistry::FinalizeRegistration() {
	if (finalized_) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "FinalizeRegistration が2回呼ばれました（2回目は何もしません）");
		return;
	}
	ValidateRequirements();
	finalized_ = true;
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, "登録を締め切りました: " + std::to_string(storage_.size()) + " 型");
}

void TypeRegistry::ValidateRequirements() {
	// byId_ は const ポインタなので、書き換え用に所有側（storage_）から引き直す.
	MutableTypeMap byId;
	for (const std::unique_ptr<ComponentTypeInfo>& info : storage_) {
		byId[info->id] = info.get();
	}

	// 1) 未登録の依存先を取り除く（Find で引けない型は追加しようがない）.
	for (const std::unique_ptr<ComponentTypeInfo>& info : storage_) {
		std::erase_if(info->requiredTypes, [&](ComponentTypeId required) {
			if (byId.contains(required)) {
				return false;
			}
			DebugLog::GetInstance().Log(
				LogLevel::Error, kLogCategory,
				std::string("依存先の型が登録されていません: ") + info->type.name + " (RegisterAllComponents と Game::RegisterGameComponents を確認してください)"
			);
			return true;
		});
	}

	// 2) 循環を取り除く。登録順にたどるので、どの依存が外れるかは毎回同じになる.
	std::unordered_map<ComponentTypeId, VisitState> states;
	for (const std::unique_ptr<ComponentTypeInfo>& info : storage_) {
		RemoveCycles(*info, byId, states);
	}

	// 3) 有効になった依存をログに残す（依存を持つ型だけ）.
	for (const std::unique_ptr<ComponentTypeInfo>& info : storage_) {
		if (info->requiredTypes.empty()) {
			continue;
		}
		std::string message = std::string("依存: ") + info->type.name + " ->";
		for (ComponentTypeId required : info->requiredTypes) {
			message += std::string(" ") + byId.at(required)->type.name;
		}
		DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, message);
	}
}

#pragma endregion

} // namespace Cake
