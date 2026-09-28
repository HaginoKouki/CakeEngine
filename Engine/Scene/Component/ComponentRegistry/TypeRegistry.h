#pragma once
/*====================================
 *
 * 登録されたコンポーネント型の一覧を保持する登録簿。
 * 型情報（TypeInfo）に加えて、型を知らないコードからコンポーネントを
 * 生成・取得・削除・更新するための関数ポインタ（ComponentOps）を持つ。
 *
 * インスペクタの AddComponent メニューも、シーンの読み込みも、毎フレームの更新も、
 * 「型名や型IDしか知らない」状態からここを引いて実体を操作する。
 *
 * 【Update は書いた型だけが呼ばれる】
 * コンポーネントに以下のメンバ関数があれば、自動的に毎フレーム呼ばれるようになる。
 *
 *     void Update(const UpdateContext& ctx, GameObjectId self);
 *
 * 無い型（MeshRendererComponent など）は ops.update が nullptr のままで、
 * 更新の対象にならない。継承もインターフェース実装も不要で、
 * 「書けば呼ばれる、書かなければ呼ばれない」だけ。
 *
 * 【実行順】
 * Register の第1引数で優先度を指定する。小さいほど先に走る。
 * 同じ優先度なら登録順（安定ソート）。
 *
 * 【依存宣言（RequireComponent.h）】
 * CAKE_REQUIRE_COMPONENTS で宣言された依存は requiredTypes に入る。
 * 依存を守った追加は AddComponent、削除してよいかの判定は FindDependent で行う。
 * ComponentOps の add / remove は依存を見ない生の操作なので、
 * 型を知らないコードからの追加は必ず AddComponent を通すこと。
 *
 * 【登録の締め切り（FinalizeRegistration）】
 * エンジン側（RegisterAllComponents）とゲーム側（Game::RegisterGameComponents）の
 * 両方を登録し終えたら、Application が FinalizeRegistration を1回呼ぶ。
 * ここで未登録の依存先と循環を検査し、以後の Register は受け付けない。
 * 検査を片方の登録関数の中で行うと、もう片方で登録した型の依存が検査から漏れるため、
 * 締め切りは登録関数の外に置いている。
 *
 * 【依存の向き】
 * このヘッダは Scene 層にあり、Scene.h を include する（TypeRegistry -> Scene）。逆向きではない。
 * Scene 側は TypeRegistry を一切知らないままにしてある。
 *
 * 【登録は明示的に行う】
 * ヘッダ内の静的オブジェクトによる自己登録は、初期化順が保証されず、
 * ライブラリ化した際にリンカへ捨てられるため採用していない。
 * 追加は ComponentRegistration.cpp（ゲーム側は GameModule.cpp）に1行書くこと。
 *
 * ====================================*/
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Scene/Component/ComponentRegistry/RequireComponent.h"
#include "Engine/Foundation/Reflection/TypeInfo.h"
#include "Engine/Scene/Object/ComponentPool.h"
#include "Engine/Scene/Object/GameObjectId.h"
#include "Engine/Scene/System/UpdateContext.h"
#include "Engine/Scene/Scene.h"

namespace Cake {

// 優先度の目安。小さいほど先に走る.
constexpr int kEarlyUpdatePriority = -100; // 入力の取り込みなど.
constexpr int kDefaultUpdatePriority = 0;
constexpr int kLateUpdatePriority = 100; // カメラ追従など、他の移動が終わった後に動くもの.

// コンポーネントが Update を持っているかをコンパイル時に判定する.
// 継承関係を要求せず、シグネチャが合うかどうかだけを見る.
template <class T>
concept HasComponentUpdate = requires(T& component, const UpdateContext& ctx, GameObjectId self) {
	component.Update(ctx, self);
};

// 型を知らないコードからコンポーネントを操作するための入口.
// キャプチャなしラムダから作るため、すべて素の関数ポインタ.
// 【注意】add / remove は依存宣言を見ない生の操作。
// 追加は TypeRegistry::AddComponent、削除の前には TypeRegistry::FindDependent を使うこと.
struct ComponentOps {
	void* (*add)(Scene&, GameObjectId) = nullptr;
	void* (*get)(Scene&, GameObjectId) = nullptr;
	bool (*remove)(Scene&, GameObjectId) = nullptr;
	void (*copy)(Scene&, GameObjectId dst, const void* src) = nullptr;

	// Update を持たない型では nullptr のまま.
	void (*update)(const UpdateContext&) = nullptr;
};

struct ComponentTypeInfo {
	TypeInfo type;
	ComponentOps ops;
	ComponentTypeId id = 0;
	int priority = kDefaultUpdatePriority;

	// この型を付けるときに必要な型（CAKE_REQUIRE_COMPONENTS）.
	// FinalizeRegistration（内部の ValidateRequirements）が未登録・循環の依存を取り除く.
	std::vector<ComponentTypeId> requiredTypes;
};

/// <summary>
/// 登録されたコンポーネント型の一覧を保持する登録簿.
/// </summary>
class TypeRegistry {
private:
	std::vector<std::unique_ptr<ComponentTypeInfo>> storage_;
	std::unordered_map<std::string, const ComponentTypeInfo*> byName_;
	std::unordered_map<ComponentTypeId, const ComponentTypeInfo*> byId_;
	std::vector<const ComponentTypeInfo*> ordered_;     // 登録順。メニューの並びに使う.
	std::vector<const ComponentTypeInfo*> updateOrder_; // 優先度順。Update を持つ型のみ.

	// FinalizeRegistration 済みか。true になった後の Register は拒否する.
	bool finalized_ = false;

	TypeRegistry() = default;

public:
	static TypeRegistry& GetInstance();

	TypeRegistry(const TypeRegistry&) = delete;
	TypeRegistry& operator=(const TypeRegistry&) = delete;

	// 型を登録する。CAKE_REFLECT が書かれていない型を渡すとコンパイルエラーになる.
	// priority は Update の実行順（小さいほど先）。Update が無ければ無視される.
	// FinalizeRegistration の後に呼ぶとエラーを出して無視する.
	template <class T>
	void Register(int priority = kDefaultUpdatePriority);

	// 型名から登録情報を取得する。無ければ nullptr.
	const ComponentTypeInfo* Find(const std::string& typeName) const;
	// 型IDから登録情報を取得する。無ければ nullptr.
	const ComponentTypeInfo* Find(ComponentTypeId id) const;

	// 登録順の一覧を取得する.
	// AddComponent メニューの表示に使う.
	const std::vector<const ComponentTypeInfo*>& GetAll() const { return ordered_; }

	// 優先度順の一覧を取得する.
	// Update を持つ型だけが並ぶ.
	const std::vector<const ComponentTypeInfo*>& GetUpdateOrder() const { return updateOrder_; }

	// ===== 依存宣言 =====

	// 依存先を先に追加してから、info の型を追加する。既に持っていれば既存を返す.
	// 依存先を先に付けるので、インスペクタでは依存先が上に並ぶ.
	// 【注意】戻り値のポインタは次のコンポーネント追加まで有効.
	void* AddComponent(Scene& scene, GameObjectId id, const ComponentTypeInfo& info) const;

	// id が持つコンポーネントのうち、type を必要としているものを1つ返す。無ければ nullptr.
	// 非 nullptr なら type は削除してはいけない.
	const ComponentTypeInfo* FindDependent(const Scene& scene, GameObjectId id, ComponentTypeId type) const;

	// 登録を締め切る。エンジン側とゲーム側の登録がすべて終わった後に1回だけ呼ぶこと.
	// 依存宣言を検査し（未登録の依存先と循環はエラーを出して無効にする）、以後の Register を拒否する.
	void FinalizeRegistration();

	bool IsFinalized() const { return finalized_; }

private:
	// 依存宣言を検査する。FinalizeRegistration から呼ぶ.
	// 未登録の依存先と循環を見つけたらエラーを出し、その依存を無効にする.
	void ValidateRequirements();

	// 重複登録を弾いて格納し、更新順を並べ直す。実装は .cpp（ログを出すため）.
	void Store(std::unique_ptr<ComponentTypeInfo> info);

	// AddComponent の本体。depth は循環に対する安全装置
	// （FinalizeRegistration の前に呼ばれても無限再帰にならないようにする）.
	void* AddComponentRecursive(Scene& scene, GameObjectId id, const ComponentTypeInfo& info, size_t depth) const;
};

template <class T>
void TypeRegistry::Register(int priority) {
	static_assert(
		!RequiredComponents<T>::List::template Contains<T>,
		"自分自身を依存先にはできません"
	);

	auto info = std::make_unique<ComponentTypeInfo>();
	info->type = Reflect<T>::Build();
	info->id = GetComponentTypeId<T>();
	info->priority = priority;
	info->requiredTypes = RequiredComponents<T>::List::CollectIds();

	// キャプチャなしラムダは関数ポインタへ暗黙変換できる.
	info->ops.add = [](Scene& scene, GameObjectId id) -> void* {
		return scene.AddComponent<T>(id);
	};
	info->ops.get = [](Scene& scene, GameObjectId id) -> void* {
		return scene.GetComponent<T>(id);
	};
	info->ops.remove = [](Scene& scene, GameObjectId id) -> bool {
		return scene.RemoveComponent<T>(id);
	};
	info->ops.copy = [](Scene& scene, GameObjectId dst, const void* src) {
		// AddComponent がプールを再確保すると src が無効になるため、先に値を控える.
		const T source = *static_cast<const T*>(src);
		if (T* target = scene.AddComponent<T>(dst)) {
			*target = source;
		}
	};

	// Update を持つ型だけ、プールを走査する関数を用意する.
	if constexpr (HasComponentUpdate<T>) {
		info->ops.update = [](const UpdateContext& ctx) {
			ctx.scene.GetPool<T>()->ForEach([&](GameObjectId owner, T& component) {
				const GameObject* object = ctx.scene.Find(owner);
				if (object == nullptr || !object->IsActive()) {
					return; // 非アクティブは更新しない.
				}
				component.Update(ctx, owner);
			});
		};
	}

	Store(std::move(info));
}

} // namespace Cake
