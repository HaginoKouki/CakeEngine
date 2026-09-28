#pragma once
/*====================================
 *
 * インポート結果（アーティファクト）を Library フォルダへ保管し、読み戻す。
 *
 * 【フォルダ構成】
 *   Library/ArtifactIndex.json   … GUID → 保存時の ArtifactKey.Combine() の対応表.
 *   Library/Artifacts/xx/xxxx…   … アーティファクト本体（バイナリ）.
 * 本体は鍵の16進表記でファイル名を付け、先頭2文字でサブフォルダに分ける
 * （1フォルダに数万ファイルが並ぶとエクスプローラも OS も遅くなるため）。
 *
 * 【Library はバージョン管理へ入れないこと】
 * .gitignore に Library/ を追加する。ここは元ファイルと .meta から
 * いつでも再生成できる純粋なキャッシュであり、成果物ではない。
 * 逆に .meta は必ずコミットする（GUID が失われると参照が全部切れる）。
 * 丸ごと削除しても、次回起動で全再インポートが走るだけで復旧する。
 *
 * 【最終書き込みの原子性】
 * Store は一時ファイルへ書いてからリネームする。書き込み中にクラッシュしても
 * 中途半端なアーティファクトが残らないようにするため。壊れたファイルが残ると
 * 次回起動でハッシュは一致するのに中身が読めないという厄介な状態になる。
 *
 * ====================================*/
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Asset/Import/ArtifactKey.h"

namespace Cake {

class ArtifactCache {
private:
	std::string libraryPath_ = "Library";

	// GUID → 保存済みアーティファクトの鍵。起動時に索引ファイルから読む.
	std::unordered_map<Guid, uint64_t> stored_;

	// 索引の内容が変わったか。変わっていなければ終了時に書き出さない.
	bool indexDirty_ = false;

	// 鍵からアーティファクト本体のパスを作る.
	std::string MakeArtifactPath(uint64_t combinedKey) const;

public:
	// Library フォルダを用意し、索引ファイルを読み込む.
	void Initialize(const std::string& libraryPath);

	// 索引に記録された鍵と一致し、かつ本体ファイルが実在するか。
	// 索引だけ残って本体が消えている場合に false を返せることが重要
	// （Artifacts フォルダだけ手で消された、というのは普通に起きる）.
	bool IsUpToDate(const Guid& guid, const ArtifactKey& key) const;

	// アーティファクトを保存し、索引を更新する。成功で true.
	bool Store(const Guid& guid, const ArtifactKey& key, const std::vector<uint8_t>& bytes);

	// 保存済みアーティファクトを読み出す。IsUpToDate が true でも
	// 読み込みに失敗しうる（ファイル破損）ので、必ず戻り値を見ること.
	bool Load(const Guid& guid, const ArtifactKey& key, std::vector<uint8_t>& out) const;

	// 1件を無効化する。強制再インポート用.
	void Invalidate(const Guid& guid);

	// 索引ファイルを書き出す。終了時と、まとめてインポートした後に呼ぶ.
	void SaveIndex();

	// キャッシュを全て捨てる。エディタの「Reimport All」用.
	void Clear();

	size_t GetCount() const { return stored_.size(); }
};

} // namespace Cake
