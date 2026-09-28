#pragma once
/*====================================
 *
 * アセットの「種別」を表す2つの列挙。この分離が、サブアセットを扱うための土台になる。
 *
 *   ImporterType … ファイル1つに対する分類。.meta に記録し、GUID を振る単位.
 *   ObjectType   … 参照1つに対する分類。{GUID, LocalId} が指す先の種類.
 *
 * 1つのファイルが複数のオブジェクトを含むので、両者は1対1ではない。
 *
 *   bunny.obj                              ImporterType::Model
 *     {G, 0}                               ObjectType::ModelPrefab
 *     {G, MakeSubAssetId(Mesh,     "body")} ObjectType::Mesh
 *     {G, MakeSubAssetId(Material, "fur")}  ObjectType::Material
 *
 * AssetRef・ドラッグ＆ドロップ・選択・アイコンはすべて ObjectType を見る。
 * .meta とインポータは ImporterType を見る。
 *
 * 【Folder と Default も列挙に入れる理由】
 * .meta を全ファイル・全フォルダに発行する方針なので、フォルダも未対応拡張子も
 * GUID を持ち、索引に載り、Project ウィンドウで選択される。つまり両者とも
 * 「分類が要るもの」であって、例外扱いにすると判定が方々に散る。
 *
 * 【置き場所】
 * Import と Database の両方から使うため、どちらのフォルダにも入れず Asset 直下に置く。
 *
 * ====================================*/
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "Engine/Foundation/Identity/LocalId.h"

namespace Cake {

// ファイル1つの分類。.meta には ToString の文字列で保存する.
enum class ImporterType : uint8_t {
	Default, // 対応するインポータが無いファイル。索引には載るが中身は読まない.
	Folder,
	Model,
	Texture,
	Material,
	Scene,

	Count,
};

// 参照 {GUID, LocalId} が指す先の分類。
// LocalId はこの数値ではなく SubAssetTagOf の固定タグから作るので、種別を足したり
// 並べ替えたりしても保存済みの参照は切れない。今後この数値を保存に使う場所
// （成果物の先頭部分など）を作るときは、同じく固定タグか版数で守ること.
enum class ObjectType : uint8_t {
	None, // 無効値.
	Default,
	Folder,
	ModelPrefab, // モデルファイルの主オブジェクト。ノード階層のテンプレート.
	Mesh,
	Material,
	Texture,
	Scene,

	Count,
};

constexpr size_t kImporterTypeCount = static_cast<size_t>(ImporterType::Count);
constexpr size_t kObjectTypeCount = static_cast<size_t>(ObjectType::Count);

// .meta とログに出す名前。enum の並びを変えても保存内容が変わらないよう文字列で持つ.
const char* ToString(ImporterType type);
const char* ToString(ObjectType type);

// ToString の逆。未知の文字列は Default / None を返す.
ImporterType ParseImporterType(std::string_view text);
ObjectType ParseObjectType(std::string_view text);

// そのファイルの主オブジェクト（LocalId == 0）の種別.
ObjectType MainObjectTypeOf(ImporterType importer);

// パスの拡張子から ImporterType を決める（大文字小文字は区別しない）。
// フォルダはここを通さず、呼び出し側が ImporterType::Folder を指定する.
ImporterType DetectImporterType(std::string_view path);

namespace Detail {

// 4文字の文字列を32ビットの値にする（先頭の文字が上位バイト）.
constexpr uint32_t FourCC(const char (&text)[5]) {
	return (static_cast<uint32_t>(static_cast<uint8_t>(text[0])) << 24)
		| (static_cast<uint32_t>(static_cast<uint8_t>(text[1])) << 16)
		| (static_cast<uint32_t>(static_cast<uint8_t>(text[2])) << 8)
		| static_cast<uint32_t>(static_cast<uint8_t>(text[3]));
}

} // namespace Detail

// サブアセットの LocalId に畳み込む種別タグ。
//
// 【値を変えてはいけない】
// タグは LocalId を通してシーン・.mat・成果物に保存される。1文字でも変えると、
// その種別のサブアセットを指す参照がすべて別の値になって切れる。
// ObjectType の数値をそのまま使わないのは、種別の追加や並べ替えで数値がずれても
// 既存の参照を壊さないため（.meta が importer を文字列で持つのと同じ理由）。
//
// 【種別を増やしたとき】
// ここに新しい4文字を足す。足し忘れと、既存のタグとの重なりは AssetTypes.cpp の
// static_assert が検出する.
constexpr uint32_t SubAssetTagOf(ObjectType type) {
	switch (type) {
		case ObjectType::ModelPrefab:
			return Detail::FourCC("PRFB");
		case ObjectType::Mesh:
			return Detail::FourCC("MESH");
		case ObjectType::Material:
			return Detail::FourCC("MATL");
		case ObjectType::Texture:
			return Detail::FourCC("TEXR");
		case ObjectType::Scene:
			return Detail::FourCC("SCNE");
		// 以下はサブアセットにならない（元ファイル側にだけある種別と無効値）.
		case ObjectType::None:
		case ObjectType::Default:
		case ObjectType::Folder:
		case ObjectType::Count:
			return 0;
	}
	return 0;
}

// サブアセットの LocalId を作る。
// nameInFile には SubAssetNameTable が作った「ファイル内の名前」を渡すこと.
constexpr LocalId MakeSubAssetId(ObjectType type, std::string_view nameInFile) {
	return MakeTaggedLocalId(SubAssetTagOf(type), nameInFile);
}

// ファイル内のサブアセット名を決定的に一意化する。
// 空名は種別ごとの既定名で埋め、重複には "_1" "_2" を付ける（最初のものは素の名前）。
//
// 使い方：1ファイル分の名前を出現順にすべて Add し、最後に Build で結果を受け取る。
//
//   SubAssetNameTable table;
//   for (const auto& mesh : meshes) { table.Add(ObjectType::Mesh, mesh.name); }
//   for (const auto& material : materials) { table.Add(ObjectType::Material, material.name); }
//   const std::vector<SubAssetNameTable::Entry> entries = table.Build(); // Add した順に並ぶ.
//
// 【全部を見てから決める理由】
// 1つずつ決めると、ファイル内に1つしかない名前（"body_1" など）が、先に現れた重複
// （2つ目の "body"）に "_1" として取られる。そうなると名前を変えていないのに、
// 並び順が変わっただけで "body_1" への参照が別のオブジェクトを指す。
// そこで、ファイルに書かれている名前を先にすべて押さえ、連番はそれらを避けて振る。
// 同名の2つ目以降に付く名前は、その同名どうしの順序と、ファイル内の名前の集合だけで決まる。
//
// 【順序に依存する点（残るもの）】
// 同じ名前が2つ以上ある場合、どれが素の名前でどれに "_1" が付くかは出現順で決まる。
// これは名前を付け分ける以外に避けようがないので、Inspector で
// 「名前が重複しているサブアセットがある」と警告すること.
class SubAssetNameTable {
private:
	struct Request {
		ObjectType type = ObjectType::None;
		std::string baseName; // 前後の空白を除き、空なら既定名で埋めたもの.
	};

	std::vector<Request> requests_;

public:
	struct Entry {
		ObjectType type = ObjectType::None;
		std::string name; // 一意化した後の「ファイル内の名前」.
		LocalId localId = kSelfLocalId;
	};

	// 名前を1つ積む。戻り値は Build の結果での添字（Add した順の番号）.
	// 1ファイルにつき1つの表を使うこと.
	size_t Add(ObjectType type, std::string_view rawName);

	// 積んだ名前をまとめて一意化する。結果は Add した順に並ぶ。
	// 表の中身は変えないので、何度呼んでも同じ結果になる.
	std::vector<Entry> Build() const;

	void Clear() { requests_.clear(); }

	size_t GetCount() const { return requests_.size(); }
};

} // namespace Cake
