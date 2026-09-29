#pragma once
/*====================================
 *
 * アセットのオブジェクト1つを指す識別子 {GUID, LocalId}。
 *
 *   guid    … ファイル（またはフォルダ）ごとに1つ。.meta に保存されている.
 *   localId … そのファイルの中のどれか。0 は主オブジェクト、それ以外はサブオブジェクト
 *             （Asset 層の MakeSubAssetId で作る）.
 *
 * 【保存に使うのはこれだけ】
 * シーン・.mat・成果物に書くのは AssetId だけで、実行中の管理にはハンドル
 * （AssetHandle<T>、フェーズ2）を使う。2つは役割が違うので混ぜないこと。
 *
 * 【Foundation 層に置く理由】
 * Guid・LocalId と同じく、GPU もファイルも知らない純粋な値型だから。
 * Asset 層（実行時側）と AssetPipeline 層（元ファイル側）の両方から使う。
 *
 * 【JSON・BinaryStream への読み書き】
 * ここには置かない。使うフェーズ（成果物はフェーズ2、シーンはフェーズ4）で、
 * それぞれのシリアライズ側に足す。
 *
 * ====================================*/
#include <compare>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <type_traits>

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Foundation/Identity/LocalId.h"

namespace Cake {

struct AssetId {
	Guid guid;
	LocalId localId = kSelfLocalId;

	// GUID が無効なら、localId に関係なく無効.
	bool IsValid() const { return guid.IsValid(); }

	// ファイルの主オブジェクト（.mat 本体、モデルの ModelPrefab など）を指しているか.
	bool IsMainObject() const { return localId == kSelfLocalId; }

	// guid → localId の順で比較する。同じファイルのオブジェクトが並んで整列する.
	auto operator<=>(const AssetId&) const = default;
	bool operator==(const AssetId&) const = default;

	// ログ用の文字列。"<guid 32桁>:<localId 16桁>"（どちらも小文字16進）.
	// 保存には使わないこと（保存形式はシリアライズ側で決める）.
	std::string ToString() const {
		return guid.ToString() + std::format(":{:016x}", localId);
	}

	// 無効値.
	static AssetId Invalid() { return AssetId{}; }

	// ファイルの主オブジェクトを指す AssetId.
	static AssetId MainOf(const Guid& fileGuid) { return AssetId{fileGuid, kSelfLocalId}; }
};

// 3章「24バイトの値型」。メンバを足すと保存形式やハッシュ表の効率に響くので固定する.
static_assert(sizeof(AssetId) == 24, "AssetId は 24 バイトの値型に保つこと");
static_assert(std::is_trivially_copyable_v<AssetId>, "AssetId は memcpy できる値型に保つこと");

} // namespace Cake

// unordered_map / unordered_set のキーに使えるようにする.
template <>
struct std::hash<Cake::AssetId> {
	size_t operator()(const Cake::AssetId& id) const noexcept {
		// 主オブジェクトは localId がすべて 0 なので、撹拌してから guid のハッシュと混ぜる
		// （SplitMix64 のファイナライザ。Guid のハッシュと同じ手法）.
		uint64_t x = id.localId + 0x9E3779B97F4A7C15ull;
		x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
		x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
		x ^= x >> 31;
		return std::hash<Cake::Guid>{}(id.guid) ^ static_cast<size_t>(x);
	}
};
