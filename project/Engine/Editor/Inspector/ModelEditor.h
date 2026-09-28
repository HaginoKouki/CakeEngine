#pragma once
/*====================================
 *
 * モデル1つ分のインスペクタUIと、全モデルを並べる ModelList ウィンドウ。
 *
 * メッシュごとの頂点数・サブメッシュ構成を表示し、マテリアルスロットの差し替えと、
 * 割り当て中マテリアルの編集（MaterialEditor へ委譲）を行う。
 *
 * 差し替え先は共有アセットである ModelData のスロットなので、
 * ここでの変更は同じモデルを参照する全オブジェクトに効く。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef USE_IMGUI

#include <d3d12.h>

#include "Engine/Asset/Model/ModelHandle.h"

namespace Cake {

struct EditorDrawContext;

void DrawModelInspector(Cake::ModelHandle model, EditorDrawContext ctx);

void DrawModelList(EditorDrawContext ctx);

} // namespace Cake

#endif // USE_IMGUI
