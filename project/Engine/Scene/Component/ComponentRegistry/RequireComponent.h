#pragma once
/*====================================
 *
 * コンポーネント間の依存（「この型を付けるなら、あの型も必要」）を宣言する。
 * Unity の [RequireComponent] に相当する。
 *
 *   CAKE_REFLECT(MeshRendererComponent)
 *       ...
 *   CAKE_REFLECT_END()
 *   CAKE_REQUIRE_COMPONENTS(MeshRendererComponent, MeshFilterComponent)
 *
 * 【置き場所】
 * 構造体の定義と同じヘッダ、CAKE_REFLECT_END() の直後（namespace Cake の中）に置くこと。
 * 依存はその型の性質なので、登録側（ComponentRegistration.cpp）ではなく定義の隣に書く。
 * また、特殊化が見えない翻訳単位で RequiredComponents<T> が使われると
 * 翻訳単位ごとに中身が食い違うため、T の定義と必ず同じヘッダに置くこと。
 *
 * 依存先の型は前方宣言だけでよい（ここでは型の名前しか使わない）。
 *
 * 【効くのは TypeRegistry 経由の操作だけ】
 * TypeRegistry::AddComponent は依存先を先に自動で追加し、
 * TypeRegistry::FindDependent は削除してはいけない型を教える。
 * Scene::AddComponent / RemoveComponent を直接呼ぶコードは素通りする
 * （Scene はリフレクションを知らない、という依存の向きを守るため）。
 * そのため、依存先を使う System は「依存先が無い」場合も落ちないように書くこと。
 *
 * ====================================*/
#include <type_traits>
#include <vector>

#include "Engine/Scene/Object/ComponentPool.h"

namespace Cake {

// 依存先の型を並べるだけの入れ物.
template <class... Ts>
struct ComponentTypeList {
	// U が並びに含まれているか.
	template <class U>
	static constexpr bool Contains = (std::is_same_v<U, Ts> || ...);

	// 並んでいる型の ComponentTypeId を集める。空なら空の配列.
	static std::vector<ComponentTypeId> CollectIds() {
		return {GetComponentTypeId<Ts>()...};
	}
};

// 既定では依存なし。CAKE_REQUIRE_COMPONENTS が特殊化する.
template <class T>
struct RequiredComponents {
	using List = ComponentTypeList<>;
};

} // namespace Cake

// TypeName を付けるときに必要な型を宣言する。namespace Cake の中で使う.
#define CAKE_REQUIRE_COMPONENTS(TypeName, ...)               \
	template <>                                              \
	struct RequiredComponents<TypeName> {                    \
		using List = ::Cake::ComponentTypeList<__VA_ARGS__>; \
	};
