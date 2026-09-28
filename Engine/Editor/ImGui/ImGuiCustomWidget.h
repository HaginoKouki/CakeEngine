#pragma once
/*====================================
 *
 * ====================================*/
#ifdef USE_IMGUI

#include <string>
#include <format>

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/IconFontCppHeaders/IconsFontAwesome7.h"

#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Foundation/Identity/LocalId.h"
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Foundation/Math/Transform.h"

#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Asset/Database/AssetRef.h"

namespace Cake {

bool ImCheckBox(bool& value, const char* label = nullptr);

bool ImDragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImSliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);

bool ImStringField(std::string& str, const char* label = nullptr, float width = 0.0f);

bool ImDragVector2(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragVector2(const char* label, Vector2& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragVector3(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragVector3(const char* label, Vector3& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragVector4(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragVector4(const char* label, Vector4& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);

bool ImDragMatrix2x2(const char* label, Matrix2x2& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragMatrix3x3(const char* label, Matrix3x3& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);
bool ImDragMatrix4x4(const char* label, Matrix4x4& v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags, float width = 0.0f);

bool ImDragTransform(Transform& transform, float speed = 0.01f, const char* label = nullptr);
bool ImColorBox(float* color, const char* label = nullptr, float buttonWidth = 0.0f);
bool ImColorBox(Vector4& color, const char* label = nullptr, float buttonWidth = 0.0f);


template <class HandleT>
bool ImAssetRefCombo(const char* label, AssetRef<HandleT>* v, AssetDatabase& database) {
	// 選択中のアセットのパスをプレビューに出す。解決できない場合は未選択にする.
	const AssetEntry* current = database.Find(v->guid);
	const std::string preview = (current != nullptr) ? current->path : std::string("(none)");

	bool changed = false;
	if (ImGui::BeginCombo(std::format("##{}", label).c_str(), preview.c_str())) {
		// 未選択枠.
		if (ImGui::Selectable("(none)", !v->guid.IsValid())) {
			v->Clear();
			changed = true;
		}
		// アセット一覧を出す。選択中のものは選択済み表示にする.
		for (const AssetEntry* entry : database.GetEntriesOfType(v->Type())) {
			const bool selected = (entry->guid == v->guid);
			if (ImGui::Selectable(entry->path.c_str(), selected) && !selected) {
				v->Set(entry->guid);
				v->Resolve(database); // 選んだ時点で読み込んでおく.
				changed = true;
			}
		}
		ImGui::EndCombo();
	}

	// GUIDはあるのに解決できていない場合（アセットが消えた等）を目立たせる.
	if (v->guid.IsValid() && current == nullptr) {
		ImGui::SameLine();
		ImGui::TextDisabled("(missing)");
	}
	return changed;
}

} // namespace Cake

#endif // USE_IMGUI
