#include "SceneInspector.h"

#ifdef USE_IMGUI

#include <string>
#include <vector>
#include <algorithm>

#include "externals/imgui/imgui.h"

#include "Engine/Platform/File/FileDialog.h"
#include "Engine/Asset/Database/AssetDatabase.h"

#include "Engine/Editor/ImGui/Icons.h"
#include "Engine/Editor/ImGui/ImGuiCustomWidget.h"
#include "Engine/Editor/Inspector/PropertyDrawer.h"
#include "Engine/Editor/Inspector/InspectorMaterialSection.h"
#include "Engine/Editor/EditorDrawContext.h"

#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"

#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/System/CloneSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Serialize/SceneSerializer.h"
#include "Engine/Scene/SceneManager.h"

namespace Cake {
namespace {
struct PendingReparent {
	GameObjectId child;
	GameObjectId parent; // 無効なら root へ移す.
	bool requested = false;
};

void DrawHierarchyNode(Scene& scene, GameObjectId id, GameObjectId& selected, GameObjectId& pendingDestroy, PendingReparent& pendingReparent, GameObjectId& pendingDuplicate) {
	const GameObject* object = scene.Find(id);
	if (object == nullptr) {
		return;
	}

	// 描画中に子の生成・破棄が起きるとリストへの参照が壊れるので、先にコピーしておく.
	const std::vector<GameObjectId> children = object->GetTransform().GetChildren();
	const std::string name = object->GetName();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (children.empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (id == selected) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	ImGui::PushID(static_cast<int>(id.index));
	const bool opened = ImGui::TreeNodeEx((kGameObjectIcon + " " + name).c_str(), flags);

	// 開閉の三角をクリックしただけのときは選択を変えない.
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		selected = id;
	}

	// 右クリックでコンテキストメニューを出す.
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Create Empty Child")) {
			selected = scene.CreateGameObject("GameObject", id);
		}
		if (ImGui::MenuItem("Duplicate")) {
			// 走査中に生成すると再帰が壊れるので、後で適用する.
			pendingDuplicate = id;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Delete")) {
			pendingDestroy = id;
		}
		ImGui::EndPopup();
	}

	// ドラッグ元.
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("HierarchyObject", &id, sizeof(GameObjectId));
		ImGui::TextUnformatted(name.c_str());
		ImGui::EndDragDropSource();
	}

	// ドロップ先。ここへ落とされたら自分の子にする.
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyObject")) {
			const GameObjectId dropped = *static_cast<const GameObjectId*>(payload->Data);
			// 走査中に children_ を書き換えると再帰が壊れるので、後で適用する.
			pendingReparent = {dropped, id, true};
		}
		ImGui::EndDragDropTarget();
	}

	if (opened) {
		for (GameObjectId child : children) {
			DrawHierarchyNode(scene, child, selected, pendingDestroy, pendingReparent, pendingDuplicate);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

} // namespace

void DrawHierarchyWindow(Scene& scene, GameObjectId& selected, SceneManager& sceneManager) {
	ImGui::Begin("Hierarchy");
	const std::string label = sceneManager.HasPath() ? sceneManager.GetCurrentPath() : std::string("(new scene)");

	GameObjectId pendingDestroy;
	PendingReparent pendingReparent;
	GameObjectId pendingDuplicate;

	// ショートカット。Hierarchy にフォーカスがあるときだけ効かせる.
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && selected.IsValid()) {
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
			pendingDuplicate = selected;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
			pendingDestroy = selected;
		}
	}

	// ## 以降は表示されず ID にだけ使われる。パスが変わっても開閉状態を保つ.
	if (ImGui::CollapsingHeader((label + "###SceneRoot").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		// roots_ は走査中に変化しうるのでコピーしてから回す.
		const std::vector<GameObjectId> roots = scene.GetRoots();
		for (GameObjectId root : roots) {
			DrawHierarchyNode(scene, root, selected, pendingDestroy, pendingReparent, pendingDuplicate);
		}

		// ツリーの下の空き領域.
		// ルートへのドロップ先と、ルート直下への生成メニューを兼ねる.
		const float dropHeight = (std::max)(ImGui::GetContentRegionAvail().y, 24.0f);
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, dropHeight));

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyObject")) {
				const GameObjectId dropped = *static_cast<const GameObjectId*>(payload->Data);
				pendingReparent = {dropped, GameObjectId{}, true};
			}
			ImGui::EndDragDropTarget();
		}

		// 空き領域の右クリック。ルート直下に作る.
		if (ImGui::BeginPopupContextItem("HierarchyContext")) {
			if (ImGui::MenuItem("Create Empty")) {
				selected = scene.CreateGameObject("GameObject");
			}
			ImGui::EndPopup();
		}
	}
	// 親子の張り替えを適用する。破棄より先に行う（破棄されたIDへ張り替えないため）.
	if (pendingReparent.requested) {
		scene.SetParent(pendingReparent.child, pendingReparent.parent, false);
	}

	// 複製は同じ親の下に作る.
	if (pendingDuplicate.IsValid()) {
		GameObjectId parent;
		if (const GameObject* object = scene.Find(pendingDuplicate)) {
			parent = object->GetTransform().GetParent();
		}
		const GameObjectId created = DuplicateGameObject(scene, pendingDuplicate, parent);
		if (created.IsValid()) {
			selected = created; // 作った直後に Inspector で編集できるようにする.
		}
	}

	// 破棄はツリーを描き終えてから行う（描画中に消すとImGuiのID対応が崩れる）.
	if (pendingDestroy.IsValid()) {
		if (selected == pendingDestroy) {
			selected = GameObjectId{};
		}
		scene.DestroyGameObject(pendingDestroy);
	}

	ImGui::End();
}

void DrawInspectorWindow(Scene& scene, GameObjectId& selected, EditorDrawContext& ctx) {
	ImGui::Begin("Inspector");

	GameObject* object = scene.Find(selected);
	if (object == nullptr) {
		ImGui::TextDisabled("(no selection)");
		ImGui::End();
		return;
	}

	// --- 名前とアクティブ状態 ---
	bool active = object->IsActive();
	if (ImCheckBox(active)) {
		object->SetActive(active);
	}
	ImGui::SameLine(0.0f, 4.0f);
	std::string name = object->GetName();
	if (ImStringField(name)) {
		object->SetName(name);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// --- Transform ---
	// 組み込みフィールドでリフレクション対象外なので、ここだけ個別に描く.
	if (ImGui::CollapsingHeader((kTransformIcon + " " + "Transform").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent();
		ImGui::Spacing();
		DrawTransformProperty("", &object->GetTransform().GetLocalMutable());
		ImGui::Spacing();
		ImGui::Unindent();
	}

	// --- コンポーネント ---
	// ここから下はコンポーネントの型を一切知らない。型情報を引いて回すだけ.
	TypeRegistry& registry = TypeRegistry::GetInstance();

	ComponentTypeId pendingRemove = 0;
	bool hasPendingRemove = false;

	// 走査中に追加・削除が起きるのでコピーしてから回す.
	const std::vector<ComponentRef> refs = object->GetComponentRefs();
	for (const ComponentRef& ref : refs) {
		ImGui::PushID(static_cast<int>(ref.type));

		const ComponentTypeInfo* info = registry.Find(ref.type);
		if (info == nullptr) {
			// RegisterAllComponents への追加漏れ。黙って消えると気付けないので表示する.
			ImGui::TextDisabled("(unregistered component type: %u)", ref.type);
			ImGui::PopID();
			continue;
		}

		const bool opened = ImGui::CollapsingHeader(info->type.name, ImGuiTreeNodeFlags_DefaultOpen);
		if (ImGui::BeginPopupContextItem()) {
			// 他のコンポーネントから必要とされている型は消させない（Unity の RequireComponent と同じ）.
			const ComponentTypeInfo* dependent = registry.FindDependent(scene, selected, ref.type);
			if (ImGui::MenuItem("Remove Component", nullptr, false, dependent == nullptr)) {
				pendingRemove = ref.type;
				hasPendingRemove = true;
			}
			if (dependent != nullptr && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("%s が依存しているため削除できません", dependent->type.name);
			}
			ImGui::EndPopup();
		}

		if (opened) {
			// 型を知らないまま実体を取り出す.
			void* instance = info->ops.get(scene, selected);
			if (instance != nullptr) {
				ImGui::Indent();
				DrawProperties(instance, info->type, scene, *ctx.assetDatabase);
				ImGui::Unindent();
			}
		}
		ImGui::PopID();
	}

	if (hasPendingRemove) {
		// メニューで弾いているが、適用直前にも確かめる（ops.remove は依存を見ない生の操作のため）.
		const ComponentTypeInfo* info = registry.Find(pendingRemove);
		if (info != nullptr && registry.FindDependent(scene, selected, pendingRemove) == nullptr) {
			info->ops.remove(scene, selected);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	{
		// 現在の全体の残りの幅を取得
		float totalWidth = ImGui::GetContentRegionAvail().x;
		// スタイルから間隔の設定を取得
		float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
		float buttonWidth = totalWidth - itemSpacing * 2.0f; // ボタンの幅を計算
		if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0))) {
			ImGui::OpenPopup("AddComponentPopup");
		}
	}
	if (ImGui::BeginPopup("AddComponentPopup")) {
		// 登録済みの型が自動的に並ぶ。コンポーネントを増やしてもここは触らない.
		for (const ComponentTypeInfo* info : registry.GetAll()) {
			if (ImGui::MenuItem(info->type.name)) {
				registry.AddComponent(scene, selected, *info); // 依存先も一緒に付く.
			}
		}
		ImGui::EndPopup();
	}

	// --- マテリアル（最下部） ---
	// 型固有の知識は InspectorMaterialSection 側に閉じている.
	DrawInspectorMaterialSection(scene, selected, ctx);

	ImGui::End();
}

} // namespace Cake

#endif // USE_IMGUI
