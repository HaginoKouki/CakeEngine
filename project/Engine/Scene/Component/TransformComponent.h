#pragma once
/*====================================
 *
 * GameObject の姿勢と親子関係を担う。
 * 他のコンポーネントと違いプールには入らず、GameObject の組み込みフィールドとして
 * 必ず1つ存在する。親子はシーングラフの構造そのものなので、後から付け外しできる
 * コンポーネントの枠には入れない。
 *
 * 【親子の持ち方】
 * 親も子も GameObjectId で参照する。生ポインタは Scene のプールが再確保されると
 * 壊れるため使わない（既存 Transform の parent ポインタもここでは使用しない）。
 * 子のリストを持つのは、親が動いたときにダーティを伝播させるため。
 * 親IDだけだと子を探すのに全オブジェクト走査が必要になる。
 *
 * 【ワールド行列】
 * local_.world にキャッシュする。合成順は既存 Transform::UpdateMatrix と同じく
 * 「子 × 親」。更新は Scene::UpdateTransforms がルートから再帰で行う。
 *
 * 親子の張り替えは Scene::SetParent 経由でのみ行う（friend 指定）。
 * 直接いじると親子のリンクが片側だけになる。
 *
 * ====================================*/
#include <vector>

#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Transform.h"

#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class TransformComponent {
	friend class Scene;

private:
	Transform local_{};
	Matrix4x4 world_ = Matrix4x4::Identity;

	GameObjectId parent_;
	std::vector<GameObjectId> children_;

	// 自分の値が変わったら立つフラグ。祖先が変わった場合は Scene 側が伝播させる.
	bool dirty_ = true;

public:
	const Transform& GetLocalTransform() const { return local_; }

	// 書き換え用。ImGui 等が直接触るため、取得した時点でダーティ扱いにする.
	Transform& GetLocalMutable() {
		dirty_ = true;
		return local_;
	}

	const Matrix4x4& GetWorldMatrix() const { return world_; }

	GameObjectId GetParent() const { return parent_; }
	const std::vector<GameObjectId>& GetChildren() const { return children_; }

	bool IsDirty() const { return dirty_; }
	void MarkDirty() { dirty_ = true; }

	// 親のワールド行列を受け取って自分のワールド行列を確定する.
	void UpdateWorld(const Matrix4x4& parentWorld) {
		world_ = Matrix4x4::MakeAffineMatrix(local_.scale, local_.rotate, local_.translate);
		world_ *= parentWorld; // 子 × 親（既存 Transform と同じ順序）.
		dirty_ = false;
	}
};

} // namespace Cake
