#include "AssetTypes.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iterator>
#include <unordered_set>

namespace Cake {
namespace {

// 名前が空だったときに使う既定名.
const char* DefaultNameOf(ObjectType type) {
	switch (type) {
		case ObjectType::Mesh:
			return "Mesh";
		case ObjectType::Material:
			return "Material";
		case ObjectType::Texture:
			return "Texture";
		default:
			return "Object";
	}
}

std::string Trim(std::string_view text) {
	size_t begin = 0;
	size_t end = text.size();
	while (begin < end && std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
		++begin;
	}
	while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
		--end;
	}
	return std::string(text.substr(begin, end - begin));
}

std::string ToLowerExtension(std::string_view path) {
	std::string ext = std::filesystem::path(path).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return ext;
}

// サブアセットになる種別のタグが、0でなく互いに重なっていないか.
constexpr bool AreSubAssetTagsValid() {
	constexpr ObjectType kSubAssetTypes[] = {
		ObjectType::ModelPrefab, ObjectType::Mesh, ObjectType::Material, ObjectType::Texture, ObjectType::Scene,
	};
	for (size_t i = 0; i < std::size(kSubAssetTypes); ++i) {
		if (SubAssetTagOf(kSubAssetTypes[i]) == 0) {
			return false;
		}
		for (size_t j = i + 1; j < std::size(kSubAssetTypes); ++j) {
			if (SubAssetTagOf(kSubAssetTypes[i]) == SubAssetTagOf(kSubAssetTypes[j])) {
				return false;
			}
		}
	}
	return true;
}

} // namespace

// 種別タグが効いていることの確認。名前が同じでも種別が違えば別の値になる.
static_assert(MakeSubAssetId(ObjectType::Mesh, "body") != MakeSubAssetId(ObjectType::Material, "body"));
static_assert(MakeSubAssetId(ObjectType::Mesh, "body") != kSelfLocalId);
static_assert(MakeSubAssetId(ObjectType::Mesh, "body") == MakeSubAssetId(ObjectType::Mesh, "body"));

// ObjectType を増やしたら、SubAssetTagOf にタグを足し（サブアセットにならない種別なら 0）、
// AreSubAssetTagsValid の一覧とこの数を更新すること.
static_assert(kObjectTypeCount == 8, "ObjectType が増えています。SubAssetTagOf を確認してください");
static_assert(AreSubAssetTagsValid(), "SubAssetTagOf のタグが 0 か、他の種別と重なっています");

// 【導出結果の固定】LocalId は保存される値なので、導出の手順（MakeTaggedLocalId の
// ハッシュ、SubAssetTagOf のタグ）が変わると保存済みの参照がすべて切れる。
// ここが通らなくなったら、意図した変更でない限り元に戻すこと.
static_assert(MakeSubAssetId(ObjectType::Mesh, "body") == 0x47f58eadd65f7b98ull);
static_assert(MakeSubAssetId(ObjectType::Material, "body") == 0x35eb4bb7ca83d40bull);
static_assert(MakeSubAssetId(ObjectType::Texture, "body") == 0x1d259f16ee6acf40ull);
// 非 ASCII の名前（UTF-8 の「体」）。char の符号の扱いが変わっても値が変わらないことの確認.
static_assert(MakeSubAssetId(ObjectType::Mesh, "\xE4\xBD\x93") == 0x0028bc33988719d4ull);

const char* ToString(ImporterType type) {
	switch (type) {
		case ImporterType::Folder:
			return "Folder";
		case ImporterType::Model:
			return "Model";
		case ImporterType::Texture:
			return "Texture";
		case ImporterType::Material:
			return "Material";
		case ImporterType::Scene:
			return "Scene";
		case ImporterType::Default:
		default:
			return "Default";
	}
}

const char* ToString(ObjectType type) {
	switch (type) {
		case ObjectType::Default:
			return "Default";
		case ObjectType::Folder:
			return "Folder";
		case ObjectType::ModelPrefab:
			return "ModelPrefab";
		case ObjectType::Mesh:
			return "Mesh";
		case ObjectType::Material:
			return "Material";
		case ObjectType::Texture:
			return "Texture";
		case ObjectType::Scene:
			return "Scene";
		case ObjectType::None:
		default:
			return "None";
	}
}

ImporterType ParseImporterType(std::string_view text) {
	for (size_t i = 0; i < kImporterTypeCount; ++i) {
		const ImporterType type = static_cast<ImporterType>(i);
		if (text == ToString(type)) {
			return type;
		}
	}
	return ImporterType::Default;
}

ObjectType ParseObjectType(std::string_view text) {
	for (size_t i = 0; i < kObjectTypeCount; ++i) {
		const ObjectType type = static_cast<ObjectType>(i);
		if (text == ToString(type)) {
			return type;
		}
	}
	return ObjectType::None;
}

ObjectType MainObjectTypeOf(ImporterType importer) {
	switch (importer) {
		case ImporterType::Folder:
			return ObjectType::Folder;
		case ImporterType::Model:
			return ObjectType::ModelPrefab;
		case ImporterType::Texture:
			return ObjectType::Texture;
		case ImporterType::Material:
			return ObjectType::Material;
		case ImporterType::Scene:
			return ObjectType::Scene;
		case ImporterType::Default:
		default:
			return ObjectType::Default;
	}
}

ImporterType DetectImporterType(std::string_view path) {
	const std::string ext = ToLowerExtension(path);

	if (ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".fbx") {
		return ImporterType::Model;
	}
	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".dds" || ext == ".tga" || ext == ".bmp") {
		return ImporterType::Texture;
	}
	if (ext == ".mat") {
		return ImporterType::Material;
	}
	if (ext == ".scene") {
		return ImporterType::Scene;
	}

	// .mtl は OBJ の付属物で、単独では参照されない（中のマテリアルはモデルの
	// サブアセットとして LocalId で指す）。索引には Default として載り、
	// .meta も付くが、中身は読まない.
	return ImporterType::Default;
}

size_t SubAssetNameTable::Add(ObjectType type, std::string_view rawName) {
	if (static_cast<size_t>(type) >= kObjectTypeCount) {
		type = ObjectType::None; // 呼び出し側の誤りだが落とさない.
	}

	Request request;
	request.type = type;
	request.baseName = Trim(rawName);
	if (request.baseName.empty()) {
		request.baseName = DefaultNameOf(type);
	}

	requests_.push_back(std::move(request));
	return requests_.size() - 1;
}

std::vector<SubAssetNameTable::Entry> SubAssetNameTable::Build() const {
	// 種別ごとに別の名前の集まりとして扱う。LocalId に種別タグが入るので、
	// 種別が違えば同じ名前でも衝突しない.

	// 1) ファイルに書かれている名前（正規化後）を先にすべて押さえる。
	//    連番はこれらを避けて振るので、"body" の重複に振る連番が、
	//    ファイル内に元からある "body_1" を奪うことはない.
	std::unordered_set<std::string> used[kObjectTypeCount];
	for (const Request& request : requests_) {
		used[static_cast<size_t>(request.type)].insert(request.baseName);
	}

	// 2) 出現順に名前を決める。同じ名前の最初のものは素の名前、2つ目からは
	//    空いている連番（"_1" "_2" …、押さえてある名前は飛ばす）.
	//    異なる名前から作った連番どうしは形の上で重ならないので、ある名前の
	//    グループに付く名前は、そのグループ内の順序と、ファイル内の名前の集合だけで決まる.
	std::unordered_set<std::string> bareTaken[kObjectTypeCount];

	std::vector<Entry> entries;
	entries.reserve(requests_.size());
	for (const Request& request : requests_) {
		const size_t slot = static_cast<size_t>(request.type);

		std::string name = request.baseName;
		if (!bareTaken[slot].insert(request.baseName).second) {
			// 2つ目以降の同名.
			for (uint32_t suffix = 1;; ++suffix) {
				name = request.baseName + "_" + std::to_string(suffix);
				if (used[slot].insert(name).second) {
					break;
				}
			}
		}

		Entry entry;
		entry.type = request.type;
		entry.localId = MakeSubAssetId(request.type, name);
		entry.name = std::move(name);
		entries.push_back(std::move(entry));
	}
	return entries;
}

} // namespace Cake
