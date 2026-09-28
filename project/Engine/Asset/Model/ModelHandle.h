#pragma once
/*====================================
 *
 * モデル／メッシュへの安定参照を表す軽量ハンドル。
 * 生ポインタ・生 span の代わりにこれを持ち回ることで、ModelManager 内部の
 * 格納方法（連続配置 vector）が再確保で引っ越しても参照が壊れないようにする。
 *
 * 【2階層】
 *   ModelHandle : ModelManager 内部プール（vector<ModelData>）の添字＋世代。
 *                 GameScene が保持するのはこれ。
 *   MeshHandle  : 「どのモデルの何番目の Mesh か」＝ ModelHandle ＋ mesh index。
 *                 Mesh は ModelData 内の vector<Mesh> に属するので、
 *                 モデルの index とモデル内 index の2つで一意に決まる。
 *                 これにより ModelData が引っ越しても両方 index で無効化されない。
 *
 * generation はアンロードでスロットを再利用したとき、古いハンドルを検出する予約枠。
 * 現状アンロードは無いので常に有効だが、フィールドは用意しておく。
 *
 * ====================================*/

#include <cstdint>

namespace Cake {

struct ModelHandle {
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

	uint32_t index = kInvalidIndex; // プール内の添字（モデル実体の場所）.
	uint32_t generation = 0;        // 世代番号（将来アンロード用。今は常に有効）.

	bool IsValid() const { return index != kInvalidIndex; }

	bool operator==(const ModelHandle& rhs) const {
		return index == rhs.index && generation == rhs.generation;
	}
	bool operator!=(const ModelHandle& rhs) const { return !(*this == rhs); }
};

struct MeshHandle {
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

	ModelHandle model{};                // どのモデルに属するか.
	uint32_t meshIndex = kInvalidIndex; // そのモデルの meshes 配列での添字.

	bool IsValid() const { return model.IsValid() && meshIndex != kInvalidIndex; }

	bool operator==(const MeshHandle& rhs) const {
		return model == rhs.model && meshIndex == rhs.meshIndex;
	}
	bool operator!=(const MeshHandle& rhs) const { return !(*this == rhs); }
};

} // namespace Cake
