#include "ModelEditor.h"

#ifdef USE_IMGUI

#include <string>
#include <format>
#include <vector>

#include "externals/imgui/imgui.h"

#include "Engine/Asset/Texture/TextureManager.h"

#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialManager.h"

#include "Engine/Asset/Model/ModelManager.h"
#include "Engine/Asset/Database/AssetDatabase.h"

#include "Engine/Graphics/Shader/ShaderLibrary.h"
#include "Engine/Graphics/Shader/ShaderDefinition.h"

#include "Engine/Editor/imgui/Icons.h"
#include "Engine/Editor/Inspector/MaterialEditor.h"
#include "Engine/Editor/EditorDrawContext.h"


namespace Cake {

namespace {

bool DrawMaterialSlotCombo(
	Mesh& mesh,
	uint32_t slot,
	const std::vector<std::string>& materialNames,
	MaterialManager* materialManager
) {
	const MaterialHandle currentHandle = mesh.materialSlots[slot];
	const Material* current = materialManager->ResolveOrError(currentHandle);

	// プレビュー文字列を実体で保持する（一時オブジェクト参照回避）.
	const std::string currentName = (current != nullptr) ? current->GetName() : std::string("(none)");
	const std::string label = std::format("##MaterialSlot{}", slot);

	bool changed = false;
	ImGui::Text(std::format("{} MaterialSlot {}", kMaterialIcon, slot).c_str());
	if (ImGui::BeginCombo(label.c_str(), currentName.c_str())) {
		for (const std::string& name : materialNames) {
			const bool selected = (name == currentName);
			if (ImGui::Selectable(name.c_str(), selected) && !selected) {
				const MaterialHandle next = materialManager->FindHandle(name);
				if (next.IsValid()) {
					mesh.materialSlots[slot] = next; // 共有アセット側のスロットをハンドルで差し替える.
					changed = true;
				}
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	return changed;
}


} // namespace

void DrawModelInspector(Cake::ModelHandle model, EditorDrawContext ctx) {
	// コンボの選択肢はモデル全体で共通なので、1回だけ取得する.
	const std::vector<std::string> materialNames = ctx.materialManager->GetAllNames();

	const ModelData* modelData = ctx.modelManager->Resolve(model);
	if (modelData == nullptr) {
		ImGui::TextDisabled("(invalid model handle)");
		return;
	}

	const uint32_t meshCount = static_cast<uint32_t>(modelData->meshes.size());
	for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		// この Mesh への安定参照.
		const MeshHandle meshHandle{model, meshIndex};

		// 表示用に const 参照を取る（modelData は Resolve 済みなのでこのフレーム内で有効）.
		const Mesh& mesh = modelData->meshes[meshIndex];

		ImGui::PushID(static_cast<int>(meshIndex)); // メッシュ間のID衝突を防ぐ.

		if (ImGui::CollapsingHeader(std::format("{} {}", kMeshIcon, mesh.name != "" ? mesh.name.c_str() : "(mesh name empty)").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent();

			// 頂点情報.
			if (ImGui::CollapsingHeader(std::format("{} Vertex", kVertexIcon).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();
				uint32_t vertexCount = 0;
				for (const SubMesh& subMesh : mesh.subMeshes) {
					vertexCount += subMesh.vertexCount;
				}
				ImGui::TextUnformatted(std::format("{} total vertices : {}", kVertexIcon, vertexCount).c_str());
				ImGui::TextUnformatted(std::format("{} submeshes : {} / {} material slots : {}", kSubMeshIcon, mesh.subMeshes.size(), kMaterialIcon, mesh.GetSlotCount()).c_str());
				ImGui::Unindent();
			}

			// マテリアルの設定.
			if (ImGui::CollapsingHeader(std::format("{} Materials", kMaterialIcon).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();

				for (uint32_t slot = 0; slot < mesh.GetSlotCount(); ++slot) {
					ImGui::PushID(static_cast<int>(slot));

					// マテリアル差し替えUI：編集のため Mesh を mutable に解決する.
					Mesh* mutableMesh = ctx.modelManager->ResolveMeshMutable(meshHandle);
					if (mutableMesh != nullptr) {
						DrawMaterialSlotCombo(*mutableMesh, slot, materialNames, ctx.materialManager);
					}

					ImGui::Indent();
					// 選択中マテリアルの中身を編集する（無効ならエラーマテリアル）.
					Material* material = ctx.materialManager->ResolveOrError(mesh.materialSlots[slot]);
					if (material != nullptr) {
						DrawMaterialInspector(material, ctx);
					} else {
						ImGui::TextDisabled("(no material)");
					}
					ImGui::Unindent();

					ImGui::PopID();
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
				}

				ImGui::Unindent();
			}
			ImGui::Unindent();
		}
		ImGui::PopID();
	}
}

void DrawModelList(ModelManager* modelManager) {
	ImGui::Begin(std::format("{} ModelList", kModelIcon).c_str());

	const std::vector<ModelData>& models = modelManager->GetPool();
	if (ImGui::BeginTabBar(std::format("Models").c_str(), ImGuiTabBarFlags_FittingPolicyScroll)) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(models.size()); ++i) {
			const ModelData& model = models[i];
			ImGui::PushID(static_cast<int>(i));
			if (ImGui::BeginTabItem(model.name.c_str())) {
				ImGui::Text(std::format("{} ModelName:", kModelIcon).c_str());
				ImGui::Indent();
				ImGui::TextUnformatted(model.name.c_str());
				ImGui::Unindent();
				ImGui::Spacing();

				ImGui::Text(std::format("{} FilePath:", kFilePathIcon).c_str());
				ImGui::Indent();
				ImGui::TextUnformatted(model.filePath.c_str());
				ImGui::Unindent();
				ImGui::Spacing();

				for (const Mesh& mesh : model.meshes) {
					ImGui::Text(std::format("{} Mesh: {}", kMeshIcon, mesh.name).c_str());
					ImGui::Indent();
					for (const SubMesh& subMesh : mesh.subMeshes) {
						ImGui::Text(std::format("{} sub {}", kSubMeshIcon, subMesh.materialSlot).c_str());
						ImGui::Indent();
						ImGui::Text(std::format("{} vertex: {}", kVertexIcon, subMesh.vertexCount).c_str());
						ImGui::Unindent();
					}
					ImGui::Unindent();
					ImGui::Spacing();
				}
				ImGui::EndTabItem();
			}
			ImGui::PopID();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

} // namespace Cake

#endif // USE_IMGUI
