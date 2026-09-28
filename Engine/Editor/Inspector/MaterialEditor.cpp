#include "MaterialEditor.h"

#ifdef USE_IMGUI

#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include "externals/imgui/imgui.h"

#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Material/MaterialManager.h"
#include "Engine/Asset/Texture/TextureManager.h"

#include "Engine/Graphics/Shader/ShaderDefinition.h"
#include "Engine/Graphics/Shader/ShaderLibrary.h"

#include "Engine/Editor/ImGui/Icons.h"
#include "Engine/Editor/ImGui/ImGuiCustomWidget.h"
#include "Engine/Editor/EditorDrawContext.h"

namespace Cake {
namespace {

// 新規 .mat の置き場所。AssetDatabase のルート（Assets）配下でないと索引に載らない.
constexpr const char* kMaterialAssetDir = "Assets/Materials";

// シェーダー切替コンボ。切り替えたら true.
bool DrawShaderCombo(Material* material, EditorDrawContext& ctx) {
	const ShaderDefinition* shader = material->GetShader();
	const std::string currentName = (shader != nullptr) ? shader->name : std::string("(none)");

	// GetAllNames は unordered_map 由来で順序不定。ソートしないと毎フレーム並びが踊る.
	std::vector<std::string> names = ctx.shaderLibrary->GetAllNames();
	std::sort(names.begin(), names.end());

	bool changed = false;
	ImGui::TextUnformatted(std::format("{} Shader", kMaterialIcon).c_str());
	if (ImGui::BeginCombo("##Shader", currentName.c_str())) {
		for (const std::string& name : names) {
			const ShaderDefinition* next = ctx.shaderLibrary->Find(name);
			if (next == nullptr || next->hiddenInEditor) {
				continue; // ギズモ等の内部シェーダーは選ばせない.
			}
			const bool selected = (name == currentName);
			if (ImGui::Selectable(name.c_str(), selected) && !selected) {
				material->SetShader(ctx.device, next);
				// 新シェーダーが要求するテクスチャスロットの未割り当てを白ダミーで埋める.
				const TextureHandle white = ctx.textureManager->GetDefaultTexture();
				for (const TextureSlotDesc& slot : next->textures) {
					if (material->GetTexture(slot.name).gpuHandle.ptr == 0) {
						material->SetTexture(slot.name, white);
					}
				}
				changed = true;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	return changed;
}

// cbuffer パラメータ編集。1つでも動いたら true.
bool DrawParams(Material* material) {
	const ShaderDefinition* shader = material->GetShader();
	if (shader == nullptr) {
		ImGui::TextDisabled("(no shader)");
		return false;
	}
	if (shader->params.empty()) {
		ImGui::TextDisabled("(no parameters)");
		return false;
	}

	bool changed = false;
	for (const ShaderParamDesc& p : shader->params) {
		void* ptr = material->GetParamPtr(p.name);
		if (ptr == nullptr) {
			continue;
		}
		float* f = static_cast<float*>(ptr);
		const std::string id = std::format("##{}", p.name);

		// uiMin == uiMax は「UIから触らせない」の意味.
		ImGui::BeginDisabled(p.uiMax - p.uiMin == 0.0f);
		ImGui::TextUnformatted(p.name.c_str());
		switch (p.type) {
			case ShaderParamType::Float:
				changed |= ImGui::SliderFloat(id.c_str(), f, p.uiMin, p.uiMax);
				break;
			case ShaderParamType::Float2:
				changed |= ImGui::SliderFloat2(id.c_str(), f, p.uiMin, p.uiMax);
				break;
			case ShaderParamType::Float3:
				changed |= ImGui::SliderFloat3(id.c_str(), f, p.uiMin, p.uiMax);
				break;
			case ShaderParamType::Float4:
				changed |= ImGui::SliderFloat4(id.c_str(), f, p.uiMin, p.uiMax);
				break;
			case ShaderParamType::Color:
				changed |= ImGui::ColorEdit4(id.c_str(), f);
				break;
			case ShaderParamType::Int:
				if (p.uiMin == 0.0f && p.uiMax == 1.0f) {
					changed |= ImGui::Checkbox(id.c_str(), reinterpret_cast<bool*>(ptr));
				} else {
					changed |= ImGui::SliderInt(
						id.c_str(), reinterpret_cast<int*>(ptr),
						static_cast<int>(p.uiMin), static_cast<int>(p.uiMax)
					);
				}
				break;
		}
		ImGui::EndDisabled();
	}
	return changed;
}

// テクスチャスロット割り当て。差し替えたら true.
bool DrawTextures(Material* material, EditorDrawContext& ctx) {
	const ShaderDefinition* shader = material->GetShader();
	if (shader == nullptr || shader->textures.empty()) {
		return false;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::TextUnformatted(std::format("{} Textures", kTextureIcon).c_str());

	bool changed = false;
	for (const TextureSlotDesc& slot : shader->textures) {
		ImGui::PushID(slot.name.c_str()); // スロット間のID衝突を防ぐ.

		const TextureHandle cur = material->GetTexture(slot.name);
		const TextureData* td = ctx.textureManager->ResolveOrError(cur); // 必ず non-null.

		// プレビュー画像（64x64）とホバー時の拡大表示.
		ImGui::Image((ImTextureID)td->previewGpuHandle.ptr, ImVec2(64, 64));
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			const float maxSize = 256.0f;
			const float w = static_cast<float>(td->textureWidth);
			const float h = static_cast<float>(td->textureHeight);

			ImVec2 size(maxSize, maxSize); // サイズ不明時は正方形へフォールバック.
			if (w > 0.0f && h > 0.0f) {
				const float scale = maxSize / (w > h ? w : h); // 長辺を基準に縮小率を決める.
				size = ImVec2(w * scale, h * scale);
			}
			ImGui::Image((ImTextureID)td->previewGpuHandle.ptr, size);
			ImGui::EndTooltip();
		}

		ImGui::SameLine();

		// Material はスロットを TextureHandle でしか持たないので、
		// 現在値からその場で AssetRef を組み立ててコンボへ渡す（GUIDはパスから逆引き）.
		// 未登録パス（1x1白などの内蔵テクスチャ）は無効GUIDになり、(none) 表示になる.
		AssetRef<TextureHandle> ref;
		ref.guid = ctx.assetDatabase->GetGuid(td->filePath);
		ref.handle = cur;

		ImGui::BeginGroup();
		ImGui::TextUnformatted(slot.name.c_str()); // ImAssetRefCombo はラベルを表示しない.
		if (ImAssetRefCombo(slot.name.c_str(), &ref, *ctx.assetDatabase)) {
			// 未選択・解決失敗のときは白ダミーへ戻す。
			// 無効ハンドルのままだと gpuHandle が 0 で描画コマンドに渡ってしまう.
			material->SetTexture(
				slot.name,
				ref.IsResolved() ? ref.handle : ctx.textureManager->GetDefaultTexture()
			);
			changed = true;
		}
		ImGui::EndGroup();

		ImGui::PopID();
	}
	return changed;
}

// 保存ボタン列。読み取り専用なら「.mat として切り出す」ボタンに変わる.
// 【注意】この関数を抜けた後に material を触らないこと。
// CreateAssetFrom が MaterialManager の pool_ を再確保し、ポインタが無効になる.
void DrawSaveRow(Material* material, EditorDrawContext& ctx) {
	if (!material->IsEditable()) {
		ImGui::BeginDisabled(!ctx.allowAssetEditing);
		if (ImGui::Button(std::format("{} Create Material Asset", kMaterialIcon).c_str())) {
			const MaterialHandle source = ctx.materialManager->FindHandle(material->GetName());
			const std::string path = std::format("{}/{}.mat", kMaterialAssetDir, material->GetName());

			if (ctx.materialManager->CreateAssetFrom(source, path).IsValid()) {
				ctx.assetDatabase->ImportAsset(path); // 索引へ載せてコンボに出す.
			}
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("モデル既定の内容を .mat として複製し、編集できるようにします");
		}
		return;
	}

	const bool canSave = material->IsDirty() && material->HasAssetPath() && ctx.allowAssetEditing;
	ImGui::BeginDisabled(!canSave);
	if (ImGui::Button(std::format("{} Save (Ctrl+S)", kSaveIcon).c_str())) {
		ctx.materialManager->SaveDirtyMaterials();
	}
	ImGui::EndDisabled();

	if (!material->HasAssetPath()) {
		ImGui::SameLine();
		ImGui::TextDisabled("(no asset / changes are not saved)");
	}
}

} // namespace

void DrawMaterialInspector(Material* material, EditorDrawContext& ctx) {
	if (material == nullptr) {
		ImGui::TextDisabled("(no material)");
		return;
	}

	// --- ヘッダ：名前・出自・未保存マーク ---
	ImGui::TextUnformatted(
		std::format("{} {}{}", kMaterialIcon, material->GetName(), material->IsDirty() ? " *" : "").c_str()
	);

	const bool readOnly = !material->IsEditable();
	if (readOnly) {
		ImGui::SameLine();
		ImGui::TextDisabled("(model default / read-only)");
	} else if (material->HasAssetPath()) {
		ImGui::TextDisabled("%s", material->GetAssetPath().c_str());
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Play 中はアセットを触らせない（PlayModeController のスナップショット対象外で、
	// Stop しても巻き戻らないため）.
	ImGui::BeginDisabled(readOnly || !ctx.allowAssetEditing);

	bool changed = false;
	changed |= DrawShaderCombo(material, ctx);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	changed |= DrawParams(material);
	changed |= DrawTextures(material, ctx);

	ImGui::EndDisabled();

	// 編集があった分だけ dirty を立てる。書き戻しは Ctrl+S に任せる.
	if (changed) {
		material->MarkDirty();
	}
	material->Apply(); // 編集結果をGPUへ反映.

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawSaveRow(material, ctx);
}

void DrawMaterialInspector(MaterialHandle handle, EditorDrawContext& ctx) {
	DrawMaterialInspector(ctx.materialManager->ResolveOrError(handle), ctx);
}

void DrawMaterialList(EditorDrawContext& ctx) {
	ImGui::Begin(std::format("{} MaterialList", kMaterialIcon).c_str());

	MaterialCreateButton(ctx);
	ImGui::Separator();

	// 名前→ハンドル→Resolve で mutable に取り直す（GetPool() は const のため）.
	const std::vector<std::string> names = ctx.materialManager->GetAllNames();

	ImGui::Indent();
	if (ImGui::BeginTabBar("Materials", ImGuiTabBarFlags_FittingPolicyScroll)) {
		for (const std::string& name : names) {
			Material* material = ctx.materialManager->Resolve(ctx.materialManager->FindHandle(name));
			if (material == nullptr) {
				continue;
			}
			ImGui::PushID(name.c_str());
			if (ImGui::BeginTabItem(std::format("{}{}", name, material->IsDirty() ? " *" : "").c_str())) {
				DrawMaterialInspector(material, ctx);
				ImGui::EndTabItem();
			}
			ImGui::PopID();
		}
		ImGui::EndTabBar();
	}
	ImGui::Unindent();

	ImGui::End();
}

void MaterialCreateButton(EditorDrawContext& ctx) {
	// 入力欄の状態はこのウィンドウでしか使わないので static で持つ.
	static char nameBuffer[64] = "NewMaterial";
	static std::string shaderName = "Standard";

	ImGui::BeginDisabled(!ctx.allowAssetEditing);

	ImGui::SetNextItemWidth(160.0f);
	ImGui::InputText("##NewMaterialName", nameBuffer, sizeof(nameBuffer));
	ImGui::SameLine();

	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::BeginCombo("##NewMaterialShader", shaderName.c_str())) {
		std::vector<std::string> names = ctx.shaderLibrary->GetAllNames();
		std::sort(names.begin(), names.end());
		for (const std::string& name : names) {
			const ShaderDefinition* def = ctx.shaderLibrary->Find(name);
			if (def == nullptr || def->hiddenInEditor) {
				continue;
			}
			if (ImGui::Selectable(name.c_str(), name == shaderName)) {
				shaderName = name;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();

	if (ImGui::Button(std::format("{} Create", kMaterialIcon).c_str()) && nameBuffer[0] != '\0') {
		const std::string path = std::format("{}/{}.mat", kMaterialAssetDir, nameBuffer);
		if (ctx.materialManager->CreateAsset(nameBuffer, shaderName, path).IsValid()) {
			ctx.assetDatabase->ImportAsset(path);
		}
	}

	ImGui::EndDisabled();
}

} // namespace Cake

#endif // USE_IMGUI
