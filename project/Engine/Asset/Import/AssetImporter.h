#pragma once
/*====================================
 *
 * 「元ファイル → アーティファクト（バイト列）」の変換を担う基底クラス。
 * 拡張子ごとの解析処理は、プロジェクト内でここの派生クラスにしか存在しない。
 *
 * 【最重要のルール：GPU に触らないこと】
 * ImportContext に ID3D12Device は入っていない。これは意図的な制限で、
 * 「インポート結果は必ずディスクへ落とせる素のデータである」ことを型で保証する。
 * 頂点バッファの確保や SRV の生成は、アーティファクトを読み戻した後に
 * ○○Registry 側がやる。この線を跨ぐと Library キャッシュが成立しなくなる。
 *
 * 【GetVersion() の運用】
 * インポータの処理内容を変えたら必ずインクリメントすること。
 * 上げ忘れると古いキャッシュが使われ続け、修正が反映されない。
 * 「ローダーを直したのに直らない」の原因はほぼ全部これ。
 *
 * 【対象外：ネイティブアセット】
 * .mat や .scene のように「エディタで編集して書き戻す」アセットはここを通さない。
 * それらは専用のシリアライザで直接読み書きする。Library にキャッシュすると
 * 保存とキャッシュの二重管理になり、どちらが正か分からなくなるため。
 * ここを通すのは、こちらが読むだけの素材ファイル（.obj / .png など）に限る。
 *
 * ====================================*/
#include <cstdint>
#include <string>
#include <string_view>

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Foundation/Serialize/BinaryStream.h"

namespace Cake {

struct AssetMeta;
enum class AssetType;

// インポータへ渡す入力一式。GPU 関連は意図的に含めない.
struct ImportContext {
	std::string sourcePath; // 元ファイルの相対パス（スラッシュ区切り）.
	Guid guid;              // このアセットの GUID.
	const AssetMeta* meta = nullptr; // インポート設定（null にはならない）.
};

class AssetImporter {
public:
	virtual ~AssetImporter() = default;

	// このインポータが作るアセットの種類。索引の登録に使う.
	virtual AssetType GetAssetType() const = 0;

	// インポータのコード版数。処理を変えたら必ず上げること.
	virtual uint32_t GetVersion() const = 0;

	// 小文字のドット付き拡張子（".obj" など）を受け付けるか.
	virtual bool CanImport(std::string_view extension) const = 0;

	// 元ファイルを解析して out へ書き出す。失敗したら false（out の中身は破棄される）.
	// 【注意】失敗時にエンジンを落とさないこと。ログを出して false を返すだけにする。
	// 呼び出し側は無効なハンドルを返し、描画はエラーマテリアルで継続する.
	virtual bool Import(const ImportContext& context, BinaryWriter& out) = 0;

	// インポート設定のうち、結果に影響するものだけを畳み込んで返す。
	// ArtifactKey::settingsHash になる。ここに入れ忘れた設定は、変更しても
	// 再インポートが走らない（＝反映されない）ので注意.
	virtual uint64_t HashSettings(const AssetMeta& meta) const = 0;
};

} // namespace Cake
