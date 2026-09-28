#pragma once
/*====================================
 *
 * index + generation のハンドルで実体を貸し出す汎用プール。
 * TextureManager / MaterialManager / ModelManager が各自で持っていた
 * pool_ / generations_ / Emplace / IsAlive / Resolve をここへ1本化する。
 *
 * 実体は std::vector に連続配置し、外へは HandleT（index + generation）を出す。
 *   - 連続配置：走査がキャッシュに優しい.
 *   - index 参照：vector が再確保で引っ越してもハンドルは無効化されない.
 *   - generation：解放したスロットを再利用したとき、古いハンドルを検出する.
 *
 * 【Replace と Recreate の違いが要】
 * 再インポート（ホットリロード）では Replace を使う。中身だけ差し替えて
 * generation を据え置くので、そのハンドルを持っている全ての参照が
 * 何もせずに新しい内容を見る。Unity が instanceID を保ったまま
 * オブジェクトの中身を差し替えるのと同じ考え方。
 * Recreate は generation を上げるので既存の参照が一斉に無効になる。
 * 「別物になった」ことを検出させたい場合だけに使うこと。
 *
 * 【HandleT の要件】
 * uint32_t index / uint32_t generation を公開メンバに持ち、
 * kInvalidIndex を持ち、既定構築で無効になること
 * （TextureHandle のように追加のメンバがあっても構わない。ここでは触らない）。
 *
 * 【生存期間】
 * Resolve が返すポインタは次の Emplace までしか有効でない（vector の再確保で
 * 引っ越すため）。ポインタを跨いで保持せず、必要なたびに Resolve すること。
 * MaterialManager::CreateAssetFrom で実際に踏んだ落とし穴。
 *
 * ====================================*/
#include <cstdint>
#include <utility>
#include <vector>

namespace Cake {

template <class T, class HandleT>
class HandlePool {
private:
	std::vector<T> pool_;
	std::vector<uint32_t> generations_; // pool_ と同じ添字.

public:
	// 途中の再確保でハンドルは無効化されないが、先に確保しておくと再確保が減る.
	void Reserve(size_t capacity) {
		pool_.reserve(capacity);
		generations_.reserve(capacity);
	}

	// 末尾へ追加してハンドルを払い出す。generation は 1 始まり
	// （0 は無効ハンドルの既定値なので、有効な世代と衝突させない）.
	HandleT Emplace(T&& value) {
		const uint32_t index = static_cast<uint32_t>(pool_.size());
		pool_.push_back(std::move(value));
		generations_.push_back(1);

		HandleT handle{};
		handle.index = index;
		handle.generation = 1;
		return handle;
	}

	// 中身だけ差し替える。generation は据え置きなので既存の参照は生き続ける。
	// 再インポート時はこちらを使う。無効なハンドルなら何もせず false.
	bool Replace(HandleT handle, T&& value) {
		if (!IsAlive(handle)) {
			return false;
		}
		pool_[handle.index] = std::move(value);
		return true;
	}

	// 中身を差し替え、generation を上げて既存の参照を無効化する。
	// 「同じスロットだが別物になった」ことを検出させたい場合のみ.
	HandleT Recreate(HandleT handle, T&& value) {
		if (!IsAlive(handle)) {
			return HandleT{};
		}
		pool_[handle.index] = std::move(value);
		++generations_[handle.index];

		HandleT next{};
		next.index = handle.index;
		next.generation = generations_[handle.index];
		return next;
	}

	// index が範囲内で、かつ generation が一致するか.
	bool IsAlive(HandleT handle) const {
		if (!handle.IsValid() || handle.index >= pool_.size()) {
			return false;
		}
		return generations_[handle.index] == handle.generation;
	}

	// 有効なら実体、無効なら nullptr。返り値は次の Emplace まで有効.
	T* Resolve(HandleT handle) {
		return IsAlive(handle) ? &pool_[handle.index] : nullptr;
	}
	const T* Resolve(HandleT handle) const {
		return IsAlive(handle) ? &pool_[handle.index] : nullptr;
	}

	// 添字からハンドルを組み直す（generation を補う）。エディタの一覧表示用.
	HandleT HandleFromIndex(uint32_t index) const {
		if (index >= pool_.size()) {
			return HandleT{};
		}
		HandleT handle{};
		handle.index = index;
		handle.generation = generations_[index];
		return handle;
	}

	// 全件走査（エディタ・一括保存用）.
	std::vector<T>& GetAll() { return pool_; }
	const std::vector<T>& GetAll() const { return pool_; }

	size_t GetCount() const { return pool_.size(); }
};

} // namespace Cake
