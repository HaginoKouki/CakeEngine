#pragma once
/*====================================
 *
 * アセットファイルの「中」にあるものを指す識別子（サブアセットID）。
 *
 * GUID がファイルを指すのに対し、LocalId はそのファイル内のどれかを指す。
 * 組み合わせて {GUID, LocalId} で1つの参照になる。
 *
 *   LocalId == 0 : アセット本体（.mat ファイルそのもの、など）
 *   LocalId != 0 : そのファイルに埋め込まれたもの（OBJ の .mtl 由来マテリアル、など）
 *
 * 値は名前から導出する。連番にすると、モデルを再インポートして順序が変わったときに
 * 参照が別のものへ向いてしまうため。名前さえ変えなければ再インポートしても不変。
 *
 * 【種別タグを混ぜること】
 * 名前だけから導出すると、同じファイルの中にある同名のメッシュとマテリアルが
 * 同じ LocalId になる。サブアセットの ID は必ず MakeTaggedLocalId 系で作る。
 * Asset 層の MakeSubAssetId(ObjectType, 名前) がその入口。
 *
 * 【導出元にしてよい名前】
 * ファイルに書かれている名前だけ。エンジン側でリネームした後の名前
 * （MaterialManager が衝突回避で付ける "Mat.1" など）を使ってはいけない。
 * 読み込み順で変わるため、保存した参照が次回起動で別のものを指す。
 *
 * ====================================*/
#include <cstdint>
#include <string_view>

namespace Cake {

using LocalId = uint64_t;

// アセット本体を指す値.
constexpr LocalId kSelfLocalId = 0;

namespace Detail {

// FNV-1a 64bit。ArtifactKey 側にも同じ定数があるが、あちらは Cake 直下なので
// 名前がぶつからないようここへ入れてある.
inline constexpr uint64_t kLocalIdFnvOffsetBasis = 14695981039346656037ull;
inline constexpr uint64_t kLocalIdFnvPrime = 1099511628211ull;

constexpr uint64_t FnvStep(uint64_t hash, uint8_t byte) {
	hash ^= static_cast<uint64_t>(byte);
	hash *= kLocalIdFnvPrime;
	return hash;
}

} // namespace Detail

// 種別タグと名前から LocalId を導出する。
// タグを先に畳み込むので、同名でも種別が違えば別の値になる。
// タグも結果の値に焼き込まれて保存されるので、enum の数値のような変わりうる値ではなく
// 固定の値を渡すこと（Asset 層の SubAssetTagOf がその表）。
// 0 は「本体」を意味する予約値なので、万一0になったら1へずらす.
constexpr LocalId MakeTaggedLocalId(uint32_t tag, std::string_view name) {
	uint64_t hash = Detail::kLocalIdFnvOffsetBasis;
	// タグは上位バイトから順に畳み込む（環境によらず同じ値になるようにする）.
	for (int shift = 24; shift >= 0; shift -= 8) {
		hash = Detail::FnvStep(hash, static_cast<uint8_t>((tag >> shift) & 0xFFu));
	}
	for (char c : name) {
		hash = Detail::FnvStep(hash, static_cast<uint8_t>(c));
	}
	return (hash == kSelfLocalId) ? 1ull : hash;
}

// 【非推奨】名前だけから導出する旧方式。種別をまたぐと衝突する.
// 使っているのは旧 AssetDatabase の埋め込みマテリアル探索だけ。フェーズ5の切り替えで
// AssetDatabase と一緒に削除すること（仕様書 12章）.
constexpr LocalId MakeLocalId(std::string_view name) {
	uint64_t hash = Detail::kLocalIdFnvOffsetBasis;
	for (char c : name) {
		hash = Detail::FnvStep(hash, static_cast<uint8_t>(c));
	}
	return (hash == kSelfLocalId) ? 1ull : hash;
}

} // namespace Cake
