#pragma once
/*====================================
 *
 * インポータを束ね、「必要なときだけ再インポートする」判断を担う。
 * 呼び出し側から見た仕事はひとつだけで、GUID を渡すとアーティファクトの
 * バイト列が返ってくる。それがキャッシュから来たのか今作られたのかは意識しない。
 *
 * 【判断の流れ】
 *   1. 拡張子から Importer を選ぶ.
 *   2. 元ファイルの内容・.meta の設定・Importer の版数から ArtifactKey を作る.
 *   3. ArtifactCache に同じ鍵があればそれを読んで終わり（速い経路）.
 *   4. 無ければ Importer を走らせ、結果を Library へ保存する（遅い経路）.
 *
 * 【この層より上は Importer を知らない】
 * ○○Registry は EnsureImported が返すバイト列を自分の Artifact 構造体へ
 * 読み戻すだけで、OBJ なのか FBX なのかを知らない。
 * 新しい形式に対応するときは Importer を1つ登録するだけで済む。
 *
 * 【所有】
 * 登録された Importer は unique_ptr で所有する。ゲーム側が独自形式の
 * インポータを足す場合も、Application の初期化で Register するだけでよい。
 *
 * ====================================*/
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Asset/Import/ArtifactCache.h"
#include "Engine/Asset/Import/AssetImporter.h"

namespace Cake {

// インポートの結末。呼び出し側がログや進捗表示を出し分けるのに使う.
enum class ImportStatus {
	Cached,     // キャッシュがそのまま使えた.
	Imported,   // 新規または再インポートを実行した.
	NoImporter, // 対応する Importer が無い（対象外の拡張子）.
	Failed,     // Importer が失敗した、またはファイルを読めなかった.
};

class ImportPipeline {
private:
	ArtifactCache cache_;
	std::vector<std::unique_ptr<AssetImporter>> importers_;

	// 拡張子に対応する Importer を探す。見つからなければ nullptr.
	AssetImporter* FindImporter(const std::string& path) const;

	// 元ファイルと .meta と Importer から鍵を組み立てる.
	ArtifactKey MakeKey(const std::string& sourcePath, const AssetMeta& meta, const AssetImporter& importer) const;

public:
	// Library フォルダを開き、キャッシュ索引を読み込む.
	void Initialize(const std::string& libraryPath);

	// Importer を登録する。同じ拡張子を複数が名乗った場合は先に登録した方が勝つ.
	// 組み込みの登録は Asset::Initialize、ゲーム独自のものは Application から.
	void Register(std::unique_ptr<AssetImporter> importer);

	// アーティファクトを用意して out へ入れる。
	// キャッシュが有効ならそれを読み、無ければインポートして保存する.
	ImportStatus EnsureImported(
		const Guid& guid,
		const std::string& sourcePath,
		const AssetMeta& meta,
		std::vector<uint8_t>& out
	);

	// キャッシュを無視して必ずインポートし直す。
	// エディタの Reimport ボタンと、ファイル監視での自動再読み込みから呼ぶ.
	ImportStatus Reimport(
		const Guid& guid,
		const std::string& sourcePath,
		const AssetMeta& meta,
		std::vector<uint8_t>& out
	);

	// 拡張子を扱える Importer が居るか。AssetDatabase のスキャン対象の判定に使う.
	bool CanImport(const std::string& path) const { return FindImporter(path) != nullptr; }

	// 索引の書き出し。まとめてスキャンした後と終了時に呼ぶ.
	void Flush() { cache_.SaveIndex(); }

	ArtifactCache& GetCache() { return cache_; }
};

} // namespace Cake
