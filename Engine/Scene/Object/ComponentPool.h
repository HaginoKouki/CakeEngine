#pragma once
/*====================================
 *
 * コンポーネントを型ごとに連続配置で所有するプール。
 *
 * 【なぜ型ごとに分けるか】
 * 同じ型のコンポーネントが1本の vector に並ぶため、System が「全 MeshRenderer を処理」
 * するときにメモリを順方向に舐めるだけで済む（キャッシュに優しい）。
 * GameObject 側はプール内の添字（ComponentRef）だけを持つ。
 *
 * 【持ち主IDを並行配列で持つ】
 * System はコンポーネントだけでは Transform に辿り着けないため、items_ と同じ添字で
 * 持ち主の GameObjectId を保持する。コンポーネント構造体そのものには入れないので、
 * リフレクションとシリアライズは影響を受けない。
 *
 * 【コンポーネントは素の構造体のまま】
 * 仮想関数を持つのはプール側（IComponentPool）であってコンポーネントではない。
 * これにより各コンポーネントは standard-layout を保て、offsetof によるリフレクションと
 * memcpy によるスナップショット（Play/Stop）が素直に使える。
 *
 * 【型IDについて】
 * ComponentTypeId は「初回に使われた順」で採番されるため、実行のたびに変わりうる。
 * 絶対にシリアライズしないこと。保存に使うのは型名（型登録が持つ）。
 *
 * ====================================*/
#include <cstdint>
#include <vector>

#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

using ComponentTypeId = uint32_t;

// 型IDを採番する。inline なので全翻訳単位でカウンタが1つに保たれる.
inline ComponentTypeId NextComponentTypeId() {
	static ComponentTypeId next = 0;
	return next++;
}

// 型ごとに一意なIDを返す。初回呼び出し時に採番される.
template <class T>
ComponentTypeId GetComponentTypeId() {
	static const ComponentTypeId id = NextComponentTypeId();
	return id;
}

// GameObject が保持する、プール内コンポーネントへの参照.
struct ComponentRef {
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

	ComponentTypeId type = 0;
	uint32_t index = kInvalidIndex;
	uint32_t generation = 0;

	bool IsValid() const { return index != kInvalidIndex; }
};

// 型を消してプールを扱うための入口。Scene が型ごとのプールをまとめて持つために使う.
class IComponentPool {
public:
	virtual ~IComponentPool() = default;

	virtual bool IsAlive(uint32_t index, uint32_t generation) const = 0;
	virtual void Remove(uint32_t index, uint32_t generation) = 0;
	// 型消しアクセス。リフレクション（型情報と組で使う）用.
	virtual void* GetRaw(uint32_t index, uint32_t generation) = 0;
	virtual GameObjectId GetOwner(uint32_t index, uint32_t generation) const = 0;
	virtual size_t GetAliveCount() const = 0;
};

template <class T>
class ComponentPool final : public IComponentPool {
private:
	std::vector<T> items_;              // 実体（連続配置）.
	std::vector<GameObjectId> owners_;  // items_ と同じ添字。持ち主.
	std::vector<uint32_t> generations_; // items_ と同じ添字。スロットの世代.
	std::vector<uint8_t> alive_;        // items_ と同じ添字。走査時に空きを飛ばすため.
	std::vector<uint32_t> freeList_;    // 破棄で空いた添字。再利用する.
	size_t aliveCount_ = 0;

public:
	// 追加して参照を返す。空きスロットがあれば再利用する.
	ComponentRef Add(GameObjectId owner, const T& value = T{}) {
		uint32_t index = 0;
		if (!freeList_.empty()) {
			index = freeList_.back();
			freeList_.pop_back();
			items_[index] = value;
			owners_[index] = owner;
			alive_[index] = 1;
		} else {
			index = static_cast<uint32_t>(items_.size());
			items_.push_back(value);
			owners_.push_back(owner);
			generations_.push_back(1); // 世代は1始まり（0は無効値と区別する）.
			alive_.push_back(1);
		}
		++aliveCount_;

		ComponentRef ref;
		ref.type = GetComponentTypeId<T>();
		ref.index = index;
		ref.generation = generations_[index];
		return ref;
	}

	// 【注意】返したポインタは次の Add まで有効。vector の再確保で引っ越すため保持しないこと.
	T* Get(uint32_t index, uint32_t generation) {
		if (!IsAlive(index, generation)) {
			return nullptr;
		}
		return &items_[index];
	}
	const T* Get(uint32_t index, uint32_t generation) const {
		if (!IsAlive(index, generation)) {
			return nullptr;
		}
		return &items_[index];
	}

	bool IsAlive(uint32_t index, uint32_t generation) const override {
		return index < items_.size() && alive_[index] != 0 && generations_[index] == generation;
	}

	void Remove(uint32_t index, uint32_t generation) override {
		if (!IsAlive(index, generation)) {
			return;
		}
		items_[index] = T{}; // 文字列などを抱えている場合に解放する.
		owners_[index] = GameObjectId{};
		alive_[index] = 0;
		// 世代を進めて、破棄前に配られた参照を無効化する。0 は無効値なので飛ばす.
		if (++generations_[index] == 0) {
			generations_[index] = 1;
		}
		freeList_.push_back(index);
		--aliveCount_;
	}

	void* GetRaw(uint32_t index, uint32_t generation) override {
		return Get(index, generation);
	}

	GameObjectId GetOwner(uint32_t index, uint32_t generation) const override {
		if (!IsAlive(index, generation)) {
			return GameObjectId{};
		}
		return owners_[index];
	}

	size_t GetAliveCount() const override { return aliveCount_; }

	// 生きているコンポーネントだけを順に処理する。System はこれを使う.
	// fn は (GameObjectId owner, T& component) を受け取る.
	template <class Fn>
	void ForEach(Fn&& fn) {
		for (size_t i = 0; i < items_.size(); ++i) {
			if (alive_[i] != 0) {
				fn(owners_[i], items_[i]);
			}
		}
	}
};

} // namespace Cake
