#pragma once
/*====================================
 *
 * マテリアル1つ分のインスペクタUIと、全マテリアルを並べる MaterialList ウィンドウ。
 *
 * シェーダーの切り替え・cbufferパラメータの編集・テクスチャスロットの割り当てを行い、
 * 編集結果をそのフレームのうちに Material::Apply() でGPUへ反映する。
 *
 * 【編集可否】
 * Material::IsEditable() が false（モデル埋め込みの既定マテリアル）の場合、
 * 全ウィジェットを無効化して表示のみにする。編集したい場合は
 * "Create Material Asset" で .mat として切り出す。
 *
 * 【保存】
 * ここでは保存しない。編集を検知したら MarkDirty() を立てるだけで、
 * 実際の書き戻しは Ctrl+S（MaterialManager::SaveDirtyMaterials）が行う。
 * 保存先は Material が持つ assetPath_ で、パスをここで組み立ててはいけない。
 *
 * ここは描画するだけで状態を持たない。どのマテリアルを描くかは呼び出し側が決める。
 * 同じ画面に複数並べる場合は、呼び出し側で ImGui::PushID してID衝突を避けること。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef USE_IMGUI

namespace Cake {

struct MaterialHandle;
class Material;

struct EditorDrawContext;

void DrawMaterialInspector(Material* material, EditorDrawContext& ctx);
void DrawMaterialInspector(MaterialHandle handle, EditorDrawContext& ctx);

void DrawMaterialList(EditorDrawContext& ctx);
void MaterialCreateButton(EditorDrawContext& ctx);

} // namespace Cake

#endif // USE_IMGUI
