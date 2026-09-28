#pragma once
/*====================================
 *
 * アセットを一意に識別する128ビットの値（GUID）。
 * ファイルパスや内容から計算するのではなく、インポート時に1度だけランダム生成し、
 * .meta ファイルへ永続化して使う。これによりアセットを移動・リネーム・編集しても
 * 参照が切れない。
 *
 * 実行時は16バイトのPODとして持ち回り、文字列化するのはJSONへ書き出すときだけ。
 * シーンやプレハブの全コンポーネントに埋まるため、std::string で保持してはいけない。
 *
 * ====================================*/
#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

namespace Cake {

struct Guid {
	// 文字列に変換する際の桁数.
	static constexpr size_t kStringLength = 32;

	// GUIDの前半.
	uint64_t high = 0;
	// GUIDの後半.
	uint64_t low = 0;

	// GUIDが有効かどうかを判定する.
	// 全ビット0を「無効」として扱う.
	bool IsValid() const { return high != 0 || low != 0; }

	// 比較演算子一式を自動生成する.
	auto operator<=>(const Guid&) const = default;
	bool operator==(const Guid&) const = default;

	// GUIDを区切りなし32桁の小文字16進数に変換する.
	std::string ToString() const;

	// 新しいGUIDを生成する.
	static Guid Generate();

	// ToStringの逆変換(文字列をGuidに変換).
	// 文字列が不正な場合はfalseを返す.
	static bool TryParse(std::string_view text, Guid& out);

	// 無効値（全ビット0）を返す.
	static Guid Invalid() { return Guid{}; }
};

} // namespace Cake

// unordered_map / unordered_set のキーに使えるようにする.
template <>
struct std::hash<Cake::Guid> {
	size_t operator()(const Cake::Guid& guid) const noexcept {
		// 128ビットを64ビットへ畳む。単純なXORだと上位下位が打ち消し合うので、
		// 乗算とシフトで撹拌してからハッシュ値とする（SplitMix64のファイナライザ）.
		uint64_t h = guid.high ^ (guid.low * 0x9E3779B97F4A7C15ull);
		h ^= h >> 30;
		h *= 0xBF58476D1CE4E5B9ull;
		h ^= h >> 27;
		h *= 0x94D049BB133111EBull;
		h ^= h >> 31;
		return static_cast<size_t>(h);
	}
};
