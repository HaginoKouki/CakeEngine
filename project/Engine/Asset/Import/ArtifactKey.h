#pragma once
/*====================================
 *
 * 「このアーティファクトを作り直す必要があるか」を1つの数値で表す鍵。
 *
 * インポート結果は Library にキャッシュされるが、以下のどれかが変われば
 * 中身が変わりうるので作り直さなければならない。
 *   1. 元ファイルの内容      … モデルを差し替えた.
 *   2. インポート設定        … .meta のスケールや巻き順反転を変えた.
 *   3. インポータのコード     … こちらがローダーを直した.
 * この3つをハッシュにまとめ、前回の値と一致すればキャッシュをそのまま使う。
 *
 * 【3番が特に重要】
 * 各 Importer は GetVersion() を返す。ローダーのバグを直したら必ずこの数値を
 * 上げること。上げ忘れると、直したはずの処理が古いキャッシュに隠されて
 * 「直したのに直らない」という最悪のデバッグをすることになる。
 *
 * 【更新時刻ではなく内容でハッシュする理由】
 * 更新時刻はブランチ切り替えやファイルコピーで簡単に変わる（内容は同じなのに
 * 全再インポートが走る）。逆に、時刻の分解能より速く書き換わると検出できない。
 * 大きなファイルでもハッシュは数ミリ秒なので、内容を見る方が確実。
 *
 * ====================================*/
#include <cstdint>
#include <string>

namespace Cake {

// FNV-1a 64bit。LocalId と同じ方式で揃えてある（実装を1つにしたければ共通化してよい）.
constexpr uint64_t kFnvOffsetBasis = 14695981039346656037ull;
constexpr uint64_t kFnvPrime = 1099511628211ull;

// 任意のバイト列を畳み込む。seed に前回の結果を渡せば連結できる.
uint64_t HashBytes(const void* data, size_t size, uint64_t seed = kFnvOffsetBasis);

// 文字列を畳み込む.
uint64_t HashString(const std::string& text, uint64_t seed = kFnvOffsetBasis);

// ファイル全体を読んで畳み込む。開けなければ 0 を返す（0 は「不明」を意味し、
// ArtifactKey::IsValid() が false になるのでキャッシュは使われない）.
uint64_t HashFileContent(const std::string& path);

struct ArtifactKey {
	uint64_t sourceHash = 0;      // 元ファイルの内容.
	uint64_t settingsHash = 0;    // .meta のインポート設定.
	uint32_t importerVersion = 0; // インポータのコード版数.

	// 3つを1つの値へ畳み込む。Library の索引にはこの値だけを保存する.
	uint64_t Combine() const;

	// ファイル名に使える16桁の16進表記.
	std::string ToHexString() const;

	// 元ファイルを読めなかった場合は無効。無効な鍵はキャッシュヒットしない.
	bool IsValid() const { return sourceHash != 0; }

	bool operator==(const ArtifactKey& rhs) const {
		return sourceHash == rhs.sourceHash
			&& settingsHash == rhs.settingsHash
			&& importerVersion == rhs.importerVersion;
	}
	bool operator!=(const ArtifactKey& rhs) const { return !(*this == rhs); }
};

} // namespace Cake
