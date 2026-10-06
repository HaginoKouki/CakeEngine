#include "PropertyDrawer.h"

#ifdef USE_IMGUI

#include <string>
#include <format>
#include <vector>
#include <utility>
#include <algorithm>

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
		case PropertyType::Gradient:
			return DrawGradientProperty(label.c_str(), static_cast<Gradient*>(value));
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

// グラデーションの見た目を帯で描く。上半分が色、下半分が透明度（白=不透明、黒=透明）.
// 渡すのは並べ替え済みのコピー（Evaluate が昇順を前提にするため）.
void DrawGradientPreview(const Gradient& sorted, float width, float height) {
	constexpr int kSegments = 32; // 帯の分割数。キーの間を細かく区切って補間の様子を見せる.

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const float middleY = origin.y + height * 0.5f;
	const float bottomY = origin.y + height;

	for (int i = 0; i < kSegments; ++i) {
		const float t0 = static_cast<float>(i) / kSegments;
		const float t1 = static_cast<float>(i + 1) / kSegments;
		const Vector4 c0 = sorted.Evaluate(t0);
		const Vector4 c1 = sorted.Evaluate(t1);
		const float x0 = origin.x + width * t0;
		const float x1 = origin.x + width * t1;

		const ImU32 rgb0 = ImGui::GetColorU32(ImVec4(c0.x, c0.y, c0.z, 1.0f));
		const ImU32 rgb1 = ImGui::GetColorU32(ImVec4(c1.x, c1.y, c1.z, 1.0f));
		const ImU32 alpha0 = ImGui::GetColorU32(ImVec4(c0.w, c0.w, c0.w, 1.0f));
		const ImU32 alpha1 = ImGui::GetColorU32(ImVec4(c1.w, c1.w, c1.w, 1.0f));

		// 引数の色は「左上・右上・右下・左下」の順.
		drawList->AddRectFilledMultiColor(ImVec2(x0, origin.y), ImVec2(x1, middleY), rgb0, rgb1, rgb1, rgb0);
		drawList->AddRectFilledMultiColor(ImVec2(x0, middleY), ImVec2(x1, bottomY), alpha0, alpha1, alpha1, alpha0);
	}

	// DrawList に直接描いただけではカーソルが進まないので、同じ大きさの空白で場所を確保する.
	ImGui::Dummy(ImVec2(width, height));
}

// 色のキー一覧。1行に「位置・色・削除」を並べる.
bool DrawColorKeys(Gradient& gradient) {
	bool changed = false;
	gradient.colorKeyCount = Gradient::ClampKeyCount(gradient.colorKeyCount);

	int removeIndex = -1;
	for (int i = 0; i < gradient.colorKeyCount; ++i) {
		ColorKey& key = gradient.colorKeys[i];
		ImGui::PushID(i);
		ImGui::SetNextItemWidth(64.0f);
		changed |= ImGui::DragFloat("##time", &key.time, 0.005f, 0.0f, 1.0f, "%.2f", kFlags);
		ImGui::SameLine();
		changed |= ImGui::ColorEdit3("##color", &key.color.x, ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();
		// 最後の1個は消せない（Evaluate はキーが1つ以上ある前提）.
		ImGui::BeginDisabled(gradient.colorKeyCount <= 1);
		if (ImGui::SmallButton("x")) {
			removeIndex = i;
		}
		ImGui::EndDisabled();
		ImGui::PopID();
	}

	// 削除はループの後で行う（ループ中に詰めると、以降の行の添字がずれる）.
	if (removeIndex >= 0) {
		for (int i = removeIndex; i + 1 < gradient.colorKeyCount; ++i) {
			gradient.colorKeys[i] = gradient.colorKeys[i + 1];
		}
		--gradient.colorKeyCount;
		changed = true;
	}

	ImGui::BeginDisabled(gradient.colorKeyCount >= Gradient::kMaxKeys);
	if (ImGui::SmallButton("+ Color Key")) {
		// 真ん中に、今その位置にある色で足す（足しただけでは見た目が変わらないようにする）.
		const Vector4 current = gradient.Sorted().Evaluate(0.5f);
		gradient.colorKeys[gradient.colorKeyCount] = ColorKey{0.5f, {current.x, current.y, current.z}};
		++gradient.colorKeyCount;
		changed = true;
	}
	ImGui::EndDisabled();
	return changed;
}

// 透明度のキー一覧。1行に「位置・透明度・削除」を並べる.
bool DrawAlphaKeys(Gradient& gradient) {
	bool changed = false;
	gradient.alphaKeyCount = Gradient::ClampKeyCount(gradient.alphaKeyCount);

	int removeIndex = -1;
	for (int i = 0; i < gradient.alphaKeyCount; ++i) {
		AlphaKey& key = gradient.alphaKeys[i];
		ImGui::PushID(i);
		ImGui::SetNextItemWidth(64.0f);
		changed |= ImGui::DragFloat("##time", &key.time, 0.005f, 0.0f, 1.0f, "%.2f", kFlags);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(96.0f);
		changed |= ImGui::SliderFloat("##alpha", &key.alpha, 0.0f, 1.0f, "%.2f", kFlags);
		ImGui::SameLine();
		ImGui::BeginDisabled(gradient.alphaKeyCount <= 1);
		if (ImGui::SmallButton("x")) {
			removeIndex = i;
		}
		ImGui::EndDisabled();
		ImGui::PopID();
	}

	if (removeIndex >= 0) {
		for (int i = removeIndex; i + 1 < gradient.alphaKeyCount; ++i) {
			gradient.alphaKeys[i] = gradient.alphaKeys[i + 1];
		}
		--gradient.alphaKeyCount;
		changed = true;
	}

	ImGui::BeginDisabled(gradient.alphaKeyCount >= Gradient::kMaxKeys);
	if (ImGui::SmallButton("+ Alpha Key")) {
		const Vector4 current = gradient.Sorted().Evaluate(0.5f);
		gradient.alphaKeys[gradient.alphaKeyCount] = AlphaKey{0.5f, current.w};
		++gradient.alphaKeyCount;
		changed = true;
	}
	ImGui::EndDisabled();
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
bool DrawGradientProperty(const char* label, Gradient* value) {
	return DrawPropertyRow(label, value, [&] {
		bool changed = false;
		// 帯は並べ替えたコピーで描く（保存上のキーの並びは time 順とは限らないため）.
		DrawGradientPreview(value->Sorted(), ImGui::GetContentRegionAvail().x, 24.0f);
		if (ImGui::TreeNode("Color Keys")) {
			changed |= DrawColorKeys(*value);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Alpha Keys")) {
			changed |= DrawAlphaKeys(*value);
			ImGui::TreePop();
		}
		return changed;
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
