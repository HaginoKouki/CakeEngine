#include "AssetMeta.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "AssetMeta";
using json = nlohmann::json;
} // namespace

std::string GetMetaPath(const std::string& assetPath) {
	return assetPath + ".meta";
}

bool LoadAssetMeta(const std::string& assetPath, AssetMeta& out) {
	const std::string metaPath = GetMetaPath(assetPath);

	std::ifstream file(metaPath);
	if (!file.is_open()) {
		return false; // まだ .meta が無い。呼び出し側が新規発行する.
	}

	// 3番目の false で例外を投げない版になる。壊れた JSON は is_discarded() で判る.
	const json root = json::parse(file, nullptr, false);
	if (root.is_discarded() || !root.is_object()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "JSONとして読めません: " + metaPath);
		return false;
	}

	if (!root.contains("guid") || !root["guid"].is_string()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "guid の項目がありません: " + metaPath);
		return false;
	}

	Guid guid;
	if (!Guid::TryParse(root["guid"].get<std::string>(), guid) || !guid.IsValid()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "guid の書式が不正です: " + metaPath);
		return false;
	}

	AssetMeta meta;
	meta.guid = guid;

	// 以下の項目は、無ければ既定値、あっても型が違えば「壊れている」として読み込みを失敗させる。
	// json::value() は型が違うと例外を投げるので使わない（手で書き換えられた .meta で
	// 起動ごと落ちるのを防ぐ。失敗させれば、呼び出し側は GUID を振り直さずに飛ばす）.

	// v1 には version が無い.
	meta.version = 1;
	if (const auto it = root.find("version"); it != root.end()) {
		if (!it->is_number_integer()) {
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "version が整数ではありません: " + metaPath);
			return false;
		}
		const int64_t version = it->get<int64_t>();
		if (version < 1 || version > (std::numeric_limits<int>::max)()) { // Windows.h の max マクロを避けるため括弧で囲む.
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "version の値が不正です: " + metaPath);
			return false;
		}
		meta.version = static_cast<int>(version);
	}

	// v1 には importer が無い。その場合は Default になり、
	// LoadOrCreateAssetMeta が拡張子から判定した値で埋め直す.
	meta.importer = ImporterType::Default;
	if (const auto it = root.find("importer"); it != root.end()) {
		if (!it->is_string()) {
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "importer が文字列ではありません: " + metaPath);
			return false;
		}
		meta.importer = ParseImporterType(it->get_ref<const std::string&>());
	}

	if (const auto it = root.find("settings"); it != root.end()) {
		if (!it->is_object()) {
			// 空として読み進めると、次の保存で手書きの内容を黙って消してしまう.
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "settings がオブジェクトではありません: " + metaPath);
			return false;
		}
		meta.settings = *it;
	}

	out = std::move(meta);
	return true;
}

bool SaveAssetMeta(const std::string& assetPath, const AssetMeta& meta) {
	if (!meta.guid.IsValid()) {
		// 無効な GUID を書くと、次回起動で「読めるが無効」という厄介な状態になる.
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "無効なGUIDは保存しません: " + assetPath);
		return false;
	}
	if (meta.version > kCurrentMetaVersion) {
		// 新しいエンジンが書いた .meta。このエンジンの知らない項目があり得るので、
		// 書き戻すと消してしまう（version も下がる）。どの呼び出し元からでもここで止める.
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			"新しい版の .meta は書き換えません: " + GetMetaPath(assetPath)
				+ " (version " + std::to_string(meta.version) + ")"
		);
		return false;
	}

	// nlohmann::json はキーを名前順に並べて書き出すので、項目の並びは毎回同じになる.
	json root;
	root["version"] = kCurrentMetaVersion;
	root["guid"] = meta.guid.ToString();
	root["importer"] = ToString(meta.importer);
	root["settings"] = meta.settings.is_object() ? meta.settings : json::object();

	const std::string metaPath = GetMetaPath(assetPath);
	std::ofstream file(metaPath);
	if (!file.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, ".meta を書き込めません: " + metaPath);
		return false;
	}

	// タブでインデントする（プロジェクトのコード同様、差分を読みやすくするため）.
	file << root.dump(1, '\t') << std::endl;
	return file.good();
}

bool LoadOrCreateAssetMeta(const std::string& assetPath, ImporterType importer, AssetMeta& out) {
	const std::string metaPath = GetMetaPath(assetPath);

	std::error_code ec;
	const bool metaExists = std::filesystem::exists(metaPath, ec);

	// --- 既存の .meta を使う ---
	if (metaExists) {
		AssetMeta meta;
		if (!LoadAssetMeta(assetPath, meta)) {
			// 【重要】ここで GUID を振り直さない。振り直すと、この GUID を指していた
			// 参照が一斉に切れる。壊れた .meta は人が直すべきものとして飛ばす.
			DebugLog::GetInstance().Log(
				LogLevel::Error, kLogCategory,
				".meta を読めないのでこのアセットを飛ばします: " + assetPath + " (手で直すか、参照が切れるのを承知で削除してください)"
			);
			return false;
		}

		// 新しいエンジンが書いた .meta。知らない項目を消してしまうので書き戻さない.
		if (meta.version > kCurrentMetaVersion) {
			DebugLog::GetInstance().Log(
				LogLevel::Warn, kLogCategory,
				"新しい版の .meta です。書き換えずにそのまま使います: " + metaPath
			);
			out = std::move(meta);
			return true;
		}

		bool dirty = false;
		if (meta.version < kCurrentMetaVersion) {
			meta.version = kCurrentMetaVersion; // v1 → v2 の移行。guid はそのまま.
			dirty = true;
		}
		// 拡張子が変わった、または対応インポータが増えた場合に追従する。
		// 逆に Default へ戻すことはしない（知らないインポータ名を潰さないため）.
		if (importer != ImporterType::Default && meta.importer != importer) {
			meta.importer = importer;
			dirty = true;
		}
		if (dirty) {
			// 書けなくても索引は張れるので、失敗しても続行する（ログは Save 側が出す）.
			SaveAssetMeta(assetPath, meta);
		}

		out = std::move(meta);
		return true;
	}

	// --- 新規発行 ---
	AssetMeta meta;
	meta.guid = Guid::Generate();
	meta.version = kCurrentMetaVersion;
	meta.importer = importer;

	if (!meta.guid.IsValid()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "GUIDを生成できません: " + assetPath);
		return false;
	}
	if (!SaveAssetMeta(assetPath, meta)) {
		// 書けないまま索引に載せると、次回起動で別の GUID が振られて参照が切れる.
		return false;
	}

	out = std::move(meta);
	return true;
}

} // namespace Cake
