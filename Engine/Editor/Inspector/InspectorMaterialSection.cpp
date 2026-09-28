#include "InspectorMaterialSection.h"

#ifdef USE_IMGUI

#include <cstdint>
#include <format>
#include <unordered_set>

#include "externals/imgui/imgui.h"

#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialManager.h"
#include "Engine/Asset/Model/Mesh.h"
#include "Engine/Asset/Model/ModelManager.h"

#include "Engine/Scene/Component/MeshRendererComponent.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Editor/ImGui/Icons.h"
#include "Engine/Editor/Inspector/MaterialEditor.h"

namespace Cake {

void DrawInspectorMaterialSection(Scene& scene, GameObjectId selected, EditorDrawContext& ctx) {
	MeshRendererComponent* renderer = scene.GetComponent<MeshRendererComponent>(selected);
	if (renderer == nullptr) {
		return; // 描くものが無いオブジェクトでは何も出さない.
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushID("InspectorMaterialSection");

	// --- 1) 上書きマテリアルが設定されていれば、それだけを描く ---
	if (renderer->material.IsResolved()) {
		if (ImGui::CollapsingHeader(
				std::format("{} Material (Override)", kMaterialIcon).c_str(),
				ImGuiTreeNodeFlags_DefaultOpen
			)) {
			ImGui::Indent();
			DrawMaterialInspector(renderer->material.handle, ctx);
			ImGui::Unindent();
		}
		ImGui::PopID();
		return;
	}

	// --- 2) 未設定ならモデル既定を並べる（Embedded なので読み取り専用になる） ---
	const ModelData* model = ctx.modelManager->Resolve(renderer->model.handle);
	if (model == nullptr) {
		ImGui::TextDisabled("(no model)");
		ImGui::PopID();
		return;
	}

	if (!ImGui::CollapsingHeader(
			std::format("{} Materials (Model Default)", kMaterialIcon).c_str(),
			ImGuiTreeNodeFlags_DefaultOpen
		)) {
		ImGui::PopID();
		return;
	}

	ImGui::Indent();

	std::unordered_set<uint32_t> seen; // 同じマテリアルを何度も描かないための重複除去.
	int drawn = 0;
	for (const Mesh& mesh : model->meshes) {
		for (const MaterialHandle& slot : mesh.materialSlots) {
			if (!slot.IsValid() || !seen.insert(slot.index).second) {
				continue;
			}
			ImGui::PushID(static_cast<int>(slot.index));
			DrawMaterialInspector(slot, ctx);
			ImGui::PopID();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			++drawn;
		}
	}
	if (drawn == 0) {
		ImGui::TextDisabled("(no material)");
	}

	ImGui::Unindent();
	ImGui::PopID();
}

} // namespace Cake

#endif // USE_IMGUI
