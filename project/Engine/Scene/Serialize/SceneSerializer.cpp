#include "SceneSerializer.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <vector>

#include "externals/nlohmann/json.hpp"

#include "Engine/Foundation/Debug/DebugLog.h"

#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Asset/Database/AssetRef.h"

#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"
#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

using json = nlohmann::json;

namespace Cake {
namespace SceneSerializer {
namespace {
constexpr const char* kLogCategory = "SceneSerializer";

// 保存された index から復元後の GameObjectId を引く表.
using IdMap = std::unordered_map<uint32_t, GameObjectId>;

#pragma region 基本型のヘルパ

json WriteVector2(const Vector2& v) {
	return json::array({v.x, v.y});
}
json WriteVector3(const Vector3& v) {
	return json::array({v.x, v.y, v.z});
}
json WriteVector4(const Vector4& v) {
	return json::array({v.x, v.y, v.z, v.w});
}

// 配列でない・要素数が足りない場合は既定値を保つ（壊れたファイルで落とさない）.
void ReadVector2(const json& value, Vector2& out) {
	if (value.is_array() && value.size() >= 2) {
		out = {value[0].get<float>(), value[1].get<float>()};
	}
}
void ReadVector3(const json& value, Vector3& out) {
	if (value.is_array() && value.size() >= 3) {
		out = {value[0].get<float>(), value[1].get<float>(), value[2].get<float>()};
	}
}
void ReadVector4(const json& value, Vector4& out) {
	if (value.is_array() && value.size() >= 4) {
		out = {value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>()};
	}
}

json WriteTransform(const Transform& transform) {
	json result;
	result["translate"] = WriteVector3(transform.translate);
	result["rotate"] = WriteVector3(transform.rotate);
	result["scale"] = WriteVector3(transform.scale);
	return result;
}
void ReadTransform(const json& value, Transform& out) {
	if (!value.is_object()) {
		return;
	}
	if (value.contains("translate")) {
		ReadVector3(value["translate"], out.translate);
	}
	if (value.contains("rotate")) {
		ReadVector3(value["rotate"], out.rotate);
	}
	if (value.contains("scale")) {
		ReadVector3(value["scale"], out.scale);
	}
}

// グラデーションは {"colorKeys": [{time, color}...], "alphaKeys": [{time, alpha}...]} で保存する.
// 個数は保存しない（配列の要素数がそのまま個数になる）.
json WriteGradient(const Gradient& gradient) {
	json colorKeys = json::array();
	for (int i = 0; i < Gradient::ClampKeyCount(gradient.colorKeyCount); ++i) {
		json key;
		key["time"] = gradient.colorKeys[i].time;
		key["color"] = WriteVector3(gradient.colorKeys[i].color);
		colorKeys.push_back(key);
	}
	json alphaKeys = json::array();
	for (int i = 0; i < Gradient::ClampKeyCount(gradient.alphaKeyCount); ++i) {
		json key;
		key["time"] = gradient.alphaKeys[i].time;
		key["alpha"] = gradient.alphaKeys[i].alpha;
		alphaKeys.push_back(key);
	}

	json result;
	result["colorKeys"] = colorKeys;
	result["alphaKeys"] = alphaKeys;
	return result;
}
// 9個目以降は捨てる。キーが1つも読めなかった列は既定値のまま（壊れたファイルで落とさない）.
void ReadGradient(const json& value, Gradient& out) {
	if (!value.is_object()) {
		return;
	}

	if (value.contains("colorKeys") && value["colorKeys"].is_array()) {
		int count = 0;
		for (const json& keyJson : value["colorKeys"]) {
			if (count >= Gradient::kMaxKeys) {
				break;
			}
			if (!keyJson.is_object()) {
				continue;
			}
			ColorKey key;
			if (keyJson.contains("time") && keyJson["time"].is_number()) {
				key.time = keyJson["time"].get<float>();
			}
			if (keyJson.contains("color")) {
				ReadVector3(keyJson["color"], key.color);
			}
			out.colorKeys[count++] = key;
		}
		if (count > 0) {
			out.colorKeyCount = count;
		}
	}

	if (value.contains("alphaKeys") && value["alphaKeys"].is_array()) {
		int count = 0;
		for (const json& keyJson : value["alphaKeys"]) {
			if (count >= Gradient::kMaxKeys) {
				break;
			}
			if (!keyJson.is_object()) {
				continue;
			}
			AlphaKey key;
			if (keyJson.contains("time") && keyJson["time"].is_number()) {
				key.time = keyJson["time"].get<float>();
			}
			if (keyJson.contains("alpha") && keyJson["alpha"].is_number()) {
				key.alpha = keyJson["alpha"].get<float>();
			}
			out.alphaKeys[count++] = key;
		}
		if (count > 0) {
			out.alphaKeyCount = count;
		}
	}
}

// アセット参照は {guid, localId} の組で保存する.
template <class HandleT>
json WriteAssetRef(const AssetRef<HandleT>& ref) {
	json result;
	result["guid"] = ref.guid.ToString();
	result["localId"] = ref.localId;
	return result;
}

template <class HandleT>
void ReadAssetRef(const json& value, AssetRef<HandleT>& out, AssetDatabase& database) {
	out.Clear();
	if (!value.is_object() || !value.contains("guid") || !value["guid"].is_string()) {
		return;
	}

	Guid guid;
	if (!Guid::TryParse(value["guid"].get<std::string>(), guid)) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "GUIDの形式が不正です");
		return;
	}
	if (!guid.IsValid()) {
		return; // 未設定として保存されたもの.
	}

	LocalId localId = kSelfLocalId;
	if (value.contains("localId") && value["localId"].is_number_unsigned()) {
		localId = value["localId"].get<LocalId>();
	}

	out.Set(guid, localId);
	out.Resolve(database); // 読み込み時点で実体まで用意する.
}

#pragma endregion

#pragma region プロパティの読み書き

json WriteProperty(const void* instance, const PropertyDesc& desc) {
	const void* value = GetPropertyPtr(instance, desc);

	switch (desc.type) {
		case PropertyType::Bool:
			return *static_cast<const bool*>(value);
		case PropertyType::Int:
			return *static_cast<const int*>(value);
		case PropertyType::Float:
			return *static_cast<const float*>(value);

		case PropertyType::Vector2:
			return WriteVector2(*static_cast<const Vector2*>(value));
		case PropertyType::Vector3:
			return WriteVector3(*static_cast<const Vector3*>(value));
		case PropertyType::Vector4:
		case PropertyType::Color:
			return WriteVector4(*static_cast<const Vector4*>(value));

		case PropertyType::String:
			return *static_cast<const std::string*>(value);
		case PropertyType::Transform:
			return WriteTransform(*static_cast<const Transform*>(value));
		case PropertyType::Gradient:
			return WriteGradient(*static_cast<const Gradient*>(value));

		case PropertyType::AssetRefModel:
			return WriteAssetRef(*static_cast<const AssetRef<ModelHandle>*>(value));
		case PropertyType::AssetRefTexture:
			return WriteAssetRef(*static_cast<const AssetRef<TextureHandle>*>(value));
		case PropertyType::AssetRefMaterial:
			return WriteAssetRef(*static_cast<const AssetRef<MaterialHandle>*>(value));

		case PropertyType::EntityRef: {
			// generation は復元されないので index だけを保存する.
			const GameObjectId* id = static_cast<const GameObjectId*>(value);
			return id->IsValid() ? json(id->index) : json(nullptr);
		}

		case PropertyType::Unknown:
		default:
			// PropertyType を増やしてここへの追加を忘れると、この警告で気付ける.
			DebugLog::GetInstance().Log(
				LogLevel::Warn, kLogCategory,
				std::string("保存に未対応のプロパティ型です: ") + desc.name
			);
			return nullptr;
	}
}

void ReadProperty(void* instance, const PropertyDesc& desc, const json& value, AssetDatabase& database, const IdMap& idMap) {
	if (value.is_null()) {
		return; // 既定値のまま.
	}
	void* target = GetPropertyPtr(instance, desc);

	switch (desc.type) {
		case PropertyType::Bool:
			if (value.is_boolean()) {
				*static_cast<bool*>(target) = value.get<bool>();
			}
			break;
		case PropertyType::Int:
			if (value.is_number_integer()) {
				*static_cast<int*>(target) = value.get<int>();
			}
			break;
		case PropertyType::Float:
			if (value.is_number()) {
				*static_cast<float*>(target) = value.get<float>();
			}
			break;

		case PropertyType::Vector2:
			ReadVector2(value, *static_cast<Vector2*>(target));
			break;
		case PropertyType::Vector3:
			ReadVector3(value, *static_cast<Vector3*>(target));
			break;
		case PropertyType::Vector4:
		case PropertyType::Color:
			ReadVector4(value, *static_cast<Vector4*>(target));
			break;

		case PropertyType::String:
			if (value.is_string()) {
				*static_cast<std::string*>(target) = value.get<std::string>();
			}
			break;
		case PropertyType::Transform:
			ReadTransform(value, *static_cast<Transform*>(target));
			break;

		case PropertyType::AssetRefModel:
			ReadAssetRef(value, *static_cast<AssetRef<ModelHandle>*>(target), database);
			break;
		case PropertyType::AssetRefTexture:
			ReadAssetRef(value, *static_cast<AssetRef<TextureHandle>*>(target), database);
			break;
		case PropertyType::AssetRefMaterial:
			ReadAssetRef(value, *static_cast<AssetRef<MaterialHandle>*>(target), database);
			break;

		case PropertyType::EntityRef: {
			// 保存された index を、復元後の GameObjectId へ読み替える.
			GameObjectId* id = static_cast<GameObjectId*>(target);
			*id = GameObjectId{};
			if (!value.is_number_unsigned()) {
				break;
			}
			auto it = idMap.find(value.get<uint32_t>());
			if (it != idMap.end()) {
				*id = it->second;
			} else {
				DebugLog::GetInstance().Log(
					LogLevel::Warn, kLogCategory,
					std::string("参照先のオブジェクトが見つかりません: ") + desc.name
				);
			}
			break;
		}

		case PropertyType::Unknown:
		default:
			DebugLog::GetInstance().Log(
				LogLevel::Warn, kLogCategory,
				std::string("読み込みに未対応のプロパティ型です: ") + desc.name
			);
			break;
	}
}

#pragma endregion

// 親が子より先に並ぶよう、階層順に集める.
void CollectInHierarchyOrder(Scene& scene, GameObjectId id, std::vector<GameObjectId>& out) {
	const GameObject* object = scene.Find(id);
	if (object == nullptr) {
		return;
	}
	out.push_back(id);
	for (GameObjectId child : object->GetTransform().GetChildren()) {
		CollectInHierarchyOrder(scene, child, out);
	}
}

#pragma region 変換の本体

// シーンを json オブジェクトへ変換する。ファイル版も文字列版もここを通る.
json BuildSceneJson(Scene& scene) {
	TypeRegistry& registry = TypeRegistry::GetInstance();

	std::vector<GameObjectId> ordered;
	for (GameObjectId root : scene.GetRoots()) {
		CollectInHierarchyOrder(scene, root, ordered);
	}

	json objects = json::array();
	for (GameObjectId id : ordered) {
		GameObject* object = scene.Find(id);
		if (object == nullptr) {
			continue;
		}

		json objectJson;
		objectJson["id"] = id.index;
		objectJson["name"] = object->GetName();
		objectJson["active"] = object->IsActive();

		const GameObjectId parent = object->GetTransform().GetParent();
		if (parent.IsValid()) {
			objectJson["parent"] = parent.index;
		}
		objectJson["transform"] = WriteTransform(object->GetTransform().GetLocalTransform());

		json components = json::array();
		for (const ComponentRef& ref : object->GetComponentRefs()) {
			const ComponentTypeInfo* info = registry.Find(ref.type);
			if (info == nullptr) {
				DebugLog::GetInstance().Log(
					LogLevel::Warn, kLogCategory,
					"未登録の型は保存できません (RegisterAllComponents を確認してください)"
				);
				continue;
			}

			void* instance = info->ops.get(scene, id);
			if (instance == nullptr) {
				continue;
			}

			json componentJson;
			componentJson["type"] = info->type.name;
			for (const PropertyDesc& desc : info->type.properties) {
				componentJson[desc.name] = WriteProperty(instance, desc);
			}
			components.push_back(std::move(componentJson));
		}
		objectJson["components"] = std::move(components);
		objects.push_back(std::move(objectJson));
	}

	json root;
	root["version"] = kSceneVersion;
	root["objects"] = std::move(objects);
	return root;
}

// json オブジェクトからシーンを復元する。既存の中身は破棄される.
bool ApplySceneJson(Scene& scene, const json& root, AssetDatabase& assetDatabase) {
	if (!root.contains("objects") || !root["objects"].is_array()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "objects がありません");
		return false;
	}

	// 既存の中身を破棄する.
	scene.Clear();

	const json& objects = root["objects"];
	TypeRegistry& registry = TypeRegistry::GetInstance();

	// --- 1周目: オブジェクトを作り、保存されたindexとの対応を作る ---
	// 親やEntityRefの解決には全オブジェクトが揃っている必要があるため2周に分ける.
	IdMap idMap;
	for (const json& objectJson : objects) {
		if (!objectJson.contains("id") || !objectJson["id"].is_number_unsigned()) {
			continue;
		}
		const std::string name = objectJson.value("name", std::string("GameObject"));
		const GameObjectId created = scene.CreateGameObject(name);
		idMap[objectJson["id"].get<uint32_t>()] = created;
	}

	// --- 2周目: 親子・Transform・コンポーネント ---
	for (const json& objectJson : objects) {
		if (!objectJson.contains("id") || !objectJson["id"].is_number_unsigned()) {
			continue;
		}
		auto found = idMap.find(objectJson["id"].get<uint32_t>());
		if (found == idMap.end()) {
			continue;
		}
		const GameObjectId id = found->second;

		if (objectJson.contains("parent") && objectJson["parent"].is_number_unsigned()) {
			auto parent = idMap.find(objectJson["parent"].get<uint32_t>());
			if (parent != idMap.end()) {
				scene.SetParent(id, parent->second);
			}
		}

		GameObject* object = scene.Find(id);
		if (object == nullptr) {
			continue;
		}
		object->SetActive(objectJson.value("active", true));

		if (objectJson.contains("transform")) {
			ReadTransform(objectJson["transform"], object->GetTransform().GetLocalMutable());
		}

		if (!objectJson.contains("components") || !objectJson["components"].is_array()) {
			continue;
		}
		for (const json& componentJson : objectJson["components"]) {
			if (!componentJson.contains("type") || !componentJson["type"].is_string()) {
				continue;
			}
			const std::string typeName = componentJson["type"].get<std::string>();

			const ComponentTypeInfo* info = registry.Find(typeName);
			if (info == nullptr) {
				DebugLog::GetInstance().Log(
					LogLevel::Warn, kLogCategory,
					"未登録の型なので読み飛ばします: " + typeName
				);
				continue;
			}

			// 依存先が JSON に無くても補う。後ろに依存先の JSON があれば、既存のものへ読み込まれる.
			void* instance = registry.AddComponent(scene, id, *info);
			if (instance == nullptr) {
				continue;
			}
			for (const PropertyDesc& desc : info->type.properties) {
				if (componentJson.contains(desc.name)) {
					ReadProperty(instance, desc, componentJson[desc.name], assetDatabase, idMap);
				}
			}
		}

		// ops.add がプールを再確保しうるので、object ポインタはここでは使わない.
	}

	scene.UpdateTransforms();
	return true;
}

#pragma endregion

} // namespace

bool SaveScene(Scene& scene, const std::string& path) {
	const json root = BuildSceneJson(scene);

	std::ofstream ofs(path);
	if (!ofs.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "書き込み用に開けません: " + path);
		return false;
	}
	ofs << root.dump(2);

	DebugLog::GetInstance().Log(
		LogLevel::Info, kLogCategory,
		"保存: " + path + " (" + std::to_string(root["objects"].size()) + " オブジェクト)"
	);
	return true;
}

bool LoadScene(Scene& scene, const std::string& path, AssetDatabase& assetDatabase) {
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "ファイルがありません: " + path);
		return false;
	}

	json root;
	try {
		ifs >> root;
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::string("JSONの解析に失敗: ") + e.what());
		return false;
	}

	if (!ApplySceneJson(scene, root, assetDatabase)) {
		return false;
	}

	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, "読み込み: " + path);
	return true;
}

std::string SaveSceneToString(Scene& scene) {
	// スナップショット用なので整形しない（サイズと速度を優先）.
	return BuildSceneJson(scene).dump();
}

bool LoadSceneFromString(Scene& scene, const std::string& text, AssetDatabase& assetDatabase) {
	if (text.empty()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "空のスナップショットは復元できません");
		return false;
	}

	json root;
	try {
		root = json::parse(text);
	} catch (const std::exception& e) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::string("JSONの解析に失敗: ") + e.what());
		return false;
	}

	return ApplySceneJson(scene, root, assetDatabase);
}

} // namespace SceneSerializer
} // namespace Cake
