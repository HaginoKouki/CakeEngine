#include "PropertyDrawer.h"

#ifdef USE_IMGUI

#include <string>
#include <format>
#include <vector>
#include <utility>

#include "externals/imgui/imgui.h"

#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Asset/Database/AssetRef.h"

#include "Engine/Editor/ImGui/Icons.h"
#include "Engine/Editor/ImGui/ImGuiCustomWidget.h"

#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Scene.h"

namespace Cake {

bool DrawProperty(void* instance, const PropertyDesc& desc, Scene& scene, AssetDatabase& assetDatabase) {
	void* value = GetPropertyPtr(instance, desc);

	// "表示名##識別子" の形にすると、ImGui は ## の前だけを表示し、全体をIDに使う.
	// 同じ表示名のプロパティが並んでもIDが衝突しない.
	const std::string label = std::string(desc.label) + "##" + desc.name;

	switch (desc.type) {
		case PropertyType::Bool:
			return DrawBoolProperty(label.c_str(), static_cast<bool*>(value));

		case PropertyType::Int:
			return DrawIntProperty(label.c_str(), static_cast<int*>(value), desc.uiMin, desc.uiMax, desc.dragSpeed, desc.widget);

		case PropertyType::Float:
			return DrawFloatProperty(label.c_str(), static_cast<float*>(value), desc.uiMin, desc.uiMax, desc.dragSpeed, desc.widget);

		case PropertyType::Vector2:
			return DrawVector2Property(label.c_str(), static_cast<float*>(value), desc.uiMin, desc.uiMax, desc.dragSpeed);

		case PropertyType::Vector3:
			return DrawVector3Property(label.c_str(), static_cast<float*>(value), desc.uiMin, desc.uiMax, desc.dragSpeed);

		case PropertyType::Vector4:
			return DrawVector4Property(label.c_str(), static_cast<float*>(value), desc.uiMin, desc.uiMax, desc.dragSpeed);

		case PropertyType::Color:
			return DrawColorProperty(label.c_str(), static_cast<float*>(value));

		case PropertyType::String:
			return DrawStringProperty(label.c_str(), static_cast<std::string*>(value));

		case PropertyType::Transform:
			return DrawTransformProperty(label.c_str(), static_cast<Transform*>(value));

		case PropertyType::AssetRefModel:
			return DrawAssetRefProperty(label.c_str(), static_cast<AssetRef<ModelHandle>*>(value), assetDatabase);

		case PropertyType::AssetRefTexture:
			return DrawAssetRefProperty(label.c_str(), static_cast<AssetRef<TextureHandle>*>(value), assetDatabase);

		case PropertyType::AssetRefMaterial:
			return DrawAssetRefProperty(label.c_str(), static_cast<AssetRef<MaterialHandle>*>(value), assetDatabase);

		case PropertyType::EntityRef:
			return DrawEntityRefProperty(label.c_str(), static_cast<GameObjectId*>(value), scene);

		case PropertyType::Unknown:
		default:
			DrawUnknownProperty(label.c_str());
			return false;
	}
}

bool DrawProperties(void* instance, const TypeInfo& typeInfo, Scene& scene, AssetDatabase& assetDatabase) {
	bool changed = false;
	for (const PropertyDesc& desc : typeInfo.properties) {
		if (DrawProperty(instance, desc, scene, assetDatabase)) {
			changed = true;
		}
	}
	return changed;
}

namespace {
constexpr ImGuiSliderFlags kFlags = ImGuiSliderFlags_AlwaysClamp;

inline bool HasText(const char* s) {
	return s != nullptr && s[0] != '\0';
}

int ToIntBound(float v) {
	if (v <= static_cast<float>(INT_MIN)) {
		return INT_MIN;
	}
	if (v >= static_cast<float>(INT_MAX)) {
		return INT_MAX;
	}
	return static_cast<int>(v);
}

// プロパティのラベルとウィジェットを横並びに表示する.
template <class Func>
bool DrawPropertyRow(const char* label, const void* id, Func&& widget) {
	bool result = false;

	ImGui::PushID(id); // BeginTable より前に積む（テーブル ID ごとユニーク化する）

	if (ImGui::BeginTable("##propertyTable", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 4.0f);
		ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch, 6.0f);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		if (HasText(label)) {
			ImGui::AlignTextToFramePadding(); // ウィジェットと縦位置を揃える.
			const char* end = std::strstr(label, "##");
			ImGui::TextUnformatted(label, end); // endまでを表示するので、##以降の識別子は表示されない.
		}

		ImGui::TableSetColumnIndex(1);
		result = std::forward<Func>(widget)();

		ImGui::EndTable();
	}

	ImGui::PopID();
	return result;
}


// シーン内のオブジェクトを階層順に集める.
void CollectObjects(Scene& scene, GameObjectId id, std::vector<GameObjectId>& out) {
	const GameObject* object = scene.Find(id);
	if (object == nullptr) {
		return;
	}
	out.push_back(id);
	for (GameObjectId child : object->GetTransform().GetChildren()) {
		CollectObjects(scene, child, out);
	}
}
// シーン内のオブジェクト参照のコンボ.
bool DrawEntityRefCombo(GameObjectId* target, const char* label, Scene& scene) {
	const GameObject* current = scene.Find(*target);
	const std::string preview = (current != nullptr) ? current->GetName() : std::string("(none)");

	bool changed = false;
	if (ImGui::BeginCombo(label, preview.c_str())) {
		if (ImGui::Selectable("(none)", !target->IsValid())) {
			*target = GameObjectId{};
			changed = true;
		}

		std::vector<GameObjectId> all;
		for (GameObjectId root : scene.GetRoots()) {
			CollectObjects(scene, root, all);
		}
		for (GameObjectId id : all) {
			const GameObject* object = scene.Find(id);
			if (object == nullptr) {
				continue;
			}
			const bool selected = (id == *target);
			ImGui::PushID(static_cast<int>(id.index));
			if (ImGui::Selectable(object->GetName().c_str(), selected) && !selected) {
				*target = id;
				changed = true;
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	return changed;
}

} // namespace

bool DrawBoolProperty(const char* label, bool* value) {
	return DrawPropertyRow(label, value, [&] {
		return ImCheckBox(*value);
	});
}
bool DrawIntProperty(const char* label, int* value, float minValue, float maxValue, float dragSpeed, NumericWidget widget) {
	const int v_min = ToIntBound(minValue);
	const int v_max = ToIntBound(maxValue);
	return DrawPropertyRow(label, value, [&] {
		if (widget == NumericWidget::Slider) {
			return ImSliderInt("", value, v_min, v_max, "%d", kFlags);
		}
		return ImDragInt("", value, dragSpeed, v_min, v_max, "%d", kFlags);
	});
}
bool DrawFloatProperty(const char* label, float* value, float minValue, float maxValue, float dragSpeed, NumericWidget widget) {
	return DrawPropertyRow(label, value, [&] {
		if (widget == NumericWidget::Slider) {
			return ImSliderFloat("", value, minValue, maxValue, "%.3f", kFlags);
		}
		return ImDragFloat("", value, dragSpeed, minValue, maxValue, "%.3f", kFlags);
	});
}

bool DrawStringProperty(const char* label, std::string* value) {
	return DrawPropertyRow(label, value, [&] {
		return ImStringField(*value);
	});
}

bool DrawVector2Property(const char* label, float* value, float minValue, float maxValue, float dragSpeed) {
	return DrawPropertyRow(label, value, [&] {
		return ImDragVector2("", value, dragSpeed, minValue, maxValue, "%.3f", kFlags);
	});
}
bool DrawVector3Property(const char* label, float* value, float minValue, float maxValue, float dragSpeed) {
	return DrawPropertyRow(label, value, [&] {
		return ImDragVector3("", value, dragSpeed, minValue, maxValue, "%.3f", kFlags);
	});
}
bool DrawVector4Property(const char* label, float* value, float minValue, float maxValue, float dragSpeed) {
	return DrawPropertyRow(label, value, [&] {
		return ImDragVector4("", value, dragSpeed, minValue, maxValue, "%.3f", kFlags);
	});
}

bool DrawTransformProperty(const char* label, Transform* value) {
	const float dragSpeed = 0.01f;
	bool changed = false;
	changed |= DrawPropertyRow("Translate", &value->translate, [&] {
		return ImDragVector3("", value->translate, dragSpeed, kUnboundedMin, kUnboundedMax, "%.3f", kFlags);
	});
	changed |= DrawPropertyRow("Rotate", &value->rotate, [&] {
		return ImDragVector3("", value->rotate, dragSpeed, kUnboundedMin, kUnboundedMax, "%.3f", kFlags);
	});
	changed |= DrawPropertyRow("Scale", &value->scale, [&] {
		return ImDragVector3("", value->scale, dragSpeed, kUnboundedMin, kUnboundedMax, "%.3f", kFlags);
	});
	return changed;
}
bool DrawColorProperty(const char* label, float* value) {
	return DrawPropertyRow(label, value, [&] {
		return ImColorBox(value, label);
	});
}

template <class HandleT>
bool DrawAssetRefProperty(const char* label, AssetRef<HandleT>* value, AssetDatabase& assetDatabase) {
	return DrawPropertyRow(label, value, [&] {
		return ImAssetRefCombo(label, value, assetDatabase);
	});
}
bool DrawEntityRefProperty(const char* label, GameObjectId* value, Scene& scene) {
	return DrawPropertyRow(label, value, [&] {
		return DrawEntityRefCombo(value, label, scene);
	});
}

bool DrawUnknownProperty(const char* label) {
	return DrawPropertyRow(label, label, [&] {
		ImGui::TextDisabled("(unsupported type)");
		return false;
	});
}

} // namespace Cake

#endif // USE_IMGUI
