#include "ImGuiCustomWidget.h"

#ifdef USE_IMGUI

#include <algorithm>

#include "Engine/Asset/Texture/TextureHandle.h"
#include "Engine/Asset/Texture/TextureManager.h"
#include "Engine/Editor/imgui/Icons.h"

namespace Cake {

namespace {
inline bool HasText(const char* s) {
	return s != nullptr && s[0] != '\0';
}

// ラベルを描いてカーソルを右へ
void DrawInlineLabel(const char* label) {
	if (!HasText(label))
		return;
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(label);
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 0.5f);
}

// n 個の [ラベル + ドラッグ] を横一列に並べるときの、ドラッグ 1 個あたりの幅
// 隙間の内訳: ラベルとドラッグの間 (itemSpacing/2) × ラベルがある個数
//             要素と要素の間       (itemSpacing)   × (n - 1)
float CalcSplitDragWidth(const char* const* labels, int n) {
	const float total = ImGui::GetContentRegionAvail().x;
	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	float textSum = 0.0f;
	int labeled = 0;
	for (int i = 0; i < n; ++i) {
		if (!HasText(labels[i]))
			continue;
		textSum += ImGui::CalcTextSize(labels[i]).x;
		++labeled;
	}
	const float spacingSum = spacing * (labeled * 0.5f + (n - 1));
	return std::max<float>(1.0f, (total - textSum - spacingSum) / static_cast<float>(n));
}

constexpr const char* kComboNoneLabel = "(none)";
constexpr const char* kComboEmptyLabel = "(empty)";

const char* StringVectorGetter(const void* userData, int index) {
	const auto* items = static_cast<const std::vector<std::string>*>(userData);
	if (index < 0 || index >= static_cast<int>(items->size())) {
		return nullptr;
	}
	return (*items)[index].c_str();
}

} // namespace

bool ImCheckBox(bool& value, const char* label) {
	ImGui::PushID(&value);
	DrawInlineLabel(label);
	const bool changed = ImGui::Checkbox("##v", &value);
	ImGui::PopID();
	return changed;
}

bool ImDragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);
	ImGui::SetNextItemWidth(width > 0.0f ? width : ImGui::GetContentRegionAvail().x);
	const bool changed = ImGui::DragInt("##v", v, std::max<float>(v_speed, 1.0f), v_min, v_max, format, flags);
	ImGui::PopID();
	return changed;
}
bool ImSliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);
	ImGui::SetNextItemWidth(width > 0.0f ? width : ImGui::GetContentRegionAvail().x);
	const bool changed = ImGui::SliderInt("##v", v, v_min, v_max, format, flags);
	ImGui::PopID();
	return changed;
}
bool ImDragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);
	ImGui::SetNextItemWidth(width > 0.0f ? width : ImGui::GetContentRegionAvail().x);
	const bool changed = ImGui::DragFloat("##v", v, v_speed, v_min, v_max, format, flags);
	ImGui::PopID();
	return changed;
}
bool ImSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);
	ImGui::SetNextItemWidth(width > 0.0f ? width : ImGui::GetContentRegionAvail().x);
	const bool changed = ImGui::SliderFloat("##v", v, v_min, v_max, format, flags);
	ImGui::PopID();
	return changed;
}

bool ImStringField(std::string& text, const char* label, float width) {
	ImGui::PushID(&text);
	DrawInlineLabel(label);
	ImGui::SetNextItemWidth(width > 0.0f ? width : ImGui::GetContentRegionAvail().x);
	char buffer[256] = {};
	strncpy_s(buffer, sizeof(buffer), text.c_str(), _TRUNCATE);
	if (ImGui::InputText("##v", buffer, sizeof(buffer))) {
		text = buffer;
		ImGui::PopID();
		return true;
	}
	ImGui::PopID();
	return false;
}

bool ImDragVector2(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);

	const char* labels[2] = {"X", "Y"};
	const float w = CalcSplitDragWidth(labels, 2); // ← 2 で割る

	bool result = false;
	result |= ImDragFloat(labels[0], &v[0], v_speed, v_min, v_max, format, flags, w);
	ImGui::SameLine();
	result |= ImDragFloat(labels[1], &v[1], v_speed, v_min, v_max, format, flags, w);
	ImGui::PopID();
	return result;
}
bool ImDragVector2(const char* label, Vector2& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	return ImDragVector2(label, &v.x, v_speed, v_min, v_max, format, flags, width);
}
bool ImDragVector3(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);

	const char* labels[3] = {"X", "Y", "Z"};
	const float w = CalcSplitDragWidth(labels, 3);

	bool result = false;
	result |= ImDragFloat(labels[0], &v[0], v_speed, v_min, v_max, format, flags, w);
	ImGui::SameLine();
	result |= ImDragFloat(labels[1], &v[1], v_speed, v_min, v_max, format, flags, w);
	ImGui::SameLine();
	result |= ImDragFloat(labels[2], &v[2], v_speed, v_min, v_max, format, flags, w);
	ImGui::PopID();
	return result;
}
bool ImDragVector3(const char* label, Vector3& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	return ImDragVector3(label, &v.x, v_speed, v_min, v_max, format, flags, width);
}
bool ImDragVector4(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	DrawInlineLabel(label);

	const char* labels[4] = {"X", "Y", "Z", "W"};
	const float w = CalcSplitDragWidth(labels, 4); // ← 4 で割る

	bool result = false;
	result |= ImDragFloat(labels[0], &v[0], v_speed, v_min, v_max, format, flags, w);
	ImGui::SameLine();
	result |= ImDragFloat(labels[1], &v[1], v_speed, v_min, v_max, format, flags, w);
	ImGui::SameLine();
	result |= ImDragFloat(labels[2], &v[2], v_speed, v_min, v_max, format, flags, w);
	ImGui::SameLine();
	result |= ImDragFloat(labels[3], &v[3], v_speed, v_min, v_max, format, flags, w);
	ImGui::PopID();
	return result;
}
bool ImDragVector4(const char* label, Vector4& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	return ImDragVector4(label, &v.x, v_speed, v_min, v_max, format, flags, width);
}

namespace {
bool ImDragMatrixImpl(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, int n, ImGuiSliderFlags flags, float width) {
	ImGui::PushID(v);
	if (HasText(label))
		ImGui::TextUnformatted(label);

	bool result = false;
	for (int r = 0; r < n; ++r) {
		ImGui::PushID(r);

		char names[4][8];
		const char* ptrs[4];
		for (int c = 0; c < n; ++c) {
			std::snprintf(names[c], sizeof(names[c]), std::format("M{}{}", r + 1, c + 1).c_str());
			ptrs[c] = names[c];
		}
		const float w = CalcSplitDragWidth(ptrs, n);

		for (int c = 0; c < n; ++c) {
			if (c > 0)
				ImGui::SameLine();
			result |= ImDragFloat(std::format("M{}{}", r + 1, c + 1).c_str(), &v[r * n + c], v_speed, v_min, v_max, format, flags, w);
		}
		ImGui::PopID();
	}
	ImGui::PopID();
	return result;
}
} // namespace

bool ImDragMatrix2x2(const char* label, Matrix2x2& m, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	return ImDragMatrixImpl(label, &m.m[0][0], v_speed, v_min, v_max, format, 2, flags, width);
}
bool ImDragMatrix3x3(const char* label, Matrix3x3& m, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	return ImDragMatrixImpl(label, &m.m[0][0], v_speed, v_min, v_max, format, 3, flags, width);
}
bool ImDragMatrix4x4(const char* label, Matrix4x4& m, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width) {
	return ImDragMatrixImpl(label, &m.m[0][0], v_speed, v_min, v_max, format, 4, flags, width);
}

bool ImDragTransform(Transform& transform, float speed, const char* label) {
	bool result = false;
	ImGui::PushID(&transform); // テーブル ID もこれでユニークになる
	if (HasText(label))
		ImGui::TextUnformatted(label);

	if (ImGui::BeginTable("##propertyTable", 2, ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 4.0f);
		ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch, 6.0f);

		const struct {
			const char* name;
			Vector3* value;
		} rows[] = {
			{"Translate", &transform.translate},
			{"Rotate", &transform.rotate},
			{"Scale", &transform.scale},
		};

		for (const auto& row : rows) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(row.name);
			ImGui::TableSetColumnIndex(1);
			result |= ImDragVector3(row.name, *row.value, speed, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_None, 0.0f);
		}
		ImGui::EndTable();
	}
	ImGui::PopID();
	return result;
}
bool ImColorBox(float* color, const char* label, float buttonWidth) {
	bool result = false;

	ImGui::PushID(&color);

	if (!buttonWidth) {
		buttonWidth = std::max<float>(1.0f, ImGui::GetContentRegionAvail().x);
	}
	const ImVec4 preview(color[0], color[1], color[2], color[3]);
	if (ImGui::ColorButton("##preview", preview, ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(buttonWidth, 25.0f))) {
		ImGui::OpenPopup("##picker");
	}

	if (ImGui::BeginPopup("##picker")) {
		if (HasText(label)) {
			ImGui::TextUnformatted(label);
			ImGui::Separator();
		}
		result |= ImGui::ColorPicker4("##Picker", color, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);
		ImGui::EndPopup();
	}

	ImGui::PopID();
	return result;
}
bool ImColorBox(Vector4& color, const char* label, float buttonWidth) {
	return ImColorBox(&color.x, label, buttonWidth);
}


} // namespace Cake

#endif // USE_IMGUI
