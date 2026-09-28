#pragma once
/*====================================
 *
 * アセットに付随する .meta ファイル（JSON）の読み書き。
 * アセット本体（.obj / .png など）の隣に "元のファイル名 + .meta" を置き、
 * そこへ GUID とインポート設定を永続化する。これによりファイルを移動・リネーム・
 * 編集してもシーンやプレハブからの参照が切れない。
 *
 * .meta はアセット本体と一緒にバージョン管理へコミットすること。
 * 逆に Library/ はコミットしない（あちらは .meta と元ファイルから再生成できる）。
 *
 * 【全ファイル・全フォルダに発行する】
 * 対応するインポータが無いファイルにも、フォルダにも .meta を作る（Unity と同じ）。
 * Project ウィンドウでは .meta 以外のすべてが選択対象になるので、
 * 「選択できるのに GUID が無いもの」を作らないための方針。
 * 例外は Assets ルート自身で、置き場所（親フォルダ）が無いため発行しない。
 *
 * 【重要】既存の .meta が読めなかった場合、GUID を振り直してはいけない。
 * 振り直すとその GUID を指していた参照が一斉に切れるため、エラーを出して
 * そのアセットを飛ばす方針にしている（LoadOrCreateAssetMeta を参照）。
 *
 * 【インポート設定を素の JSON で持つ理由】
 * 設定の形はインポータごとに違う。ここで型を決めると AssetMeta が全インポータを
 * 知ることになるため、キーと値の対応のままにしてある。知らないキーもそのまま
 * 読み書きするので、新しいエンジンが書いた .meta を古いエンジンで開いても
 * 設定が消えない。読み書きは GetSetting / SetSetting を通すこと。
 *
 * ====================================*/
#include <string>

#include "externals/nlohmann/json.hpp"

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Asset/AssetTypes.h"

namespace Cake {

// .meta のフォーマット版数。項目を追加・変更したらインクリメントする.
//   1 … guid のみ.
//   2 … importer と settings を追加.
constexpr int kCurrentMetaVersion = 2;

struct AssetMeta {
	Guid guid;
	int version = kCurrentMetaVersion;

	// このファイルをどう扱うか。拡張子から決まるが、.meta にも記録しておく.
	ImporterType importer = ImporterType::Default;

	// インポート設定。中身の形はインポータごとに違う.
	nlohmann::json settings = nlohmann::json::object();

	// 設定を1つ読む。無い場合と、型が合わない場合は fallback を返す
	// （.meta を手で書き換えられても落とさないため）.
	template <class T>
	T GetSetting(const std::string& key, const T& fallback) const {
		if (!settings.is_object()) {
			return fallback;
		}
		const auto it = settings.find(key);
		if (it == settings.end()) {
			return fallback;
		}
		try {
			return it->get<T>();
		} catch (const nlohmann::json::exception&) {
			return fallback;
		}
	}

	// 設定を1つ書く。呼んだだけでは保存されないので、SaveAssetMeta も呼ぶこと.
	template <class T>
	void SetSetting(const std::string& key, const T& value) {
		if (!settings.is_object()) {
			settings = nlohmann::json::object();
		}
		settings[key] = value;
	}
};

// アセットのパスから .meta のパスを作る（"a/b.obj" → "a/b.obj.meta"）.
// 拡張子を残すのは、bunny.obj と bunny.png の .meta が衝突しないようにするため.
// フォルダの場合は "a/Models" → "a/Models.meta" になる.
std::string GetMetaPath(const std::string& assetPath);

// .meta を読む。ファイルが無い・壊れている・項目の型が違う場合は false（out は変更しない）.
bool LoadAssetMeta(const std::string& assetPath, AssetMeta& out);

// .meta を書き出す。成功で true.
// meta.version がこのエンジンより新しい場合は書かずに false を返す
// （知らない項目を消さないため。呼び出し元によらずここで守る）.
bool SaveAssetMeta(const std::string& assetPath, const AssetMeta& meta);

// .meta があれば読み、無ければ GUID を新規発行して書き出す。
// AssetDatabase のスキャンから呼ぶ主入口。既存 .meta の読み込みに失敗した場合は
// 作り直さず false を返す（GUID の振り直しによる参照切れを防ぐため）。
// importer には拡張子（またはフォルダであること）から判定した種別を渡す.
bool LoadOrCreateAssetMeta(const std::string& assetPath, ImporterType importer, AssetMeta& out);

} // namespace Cake
