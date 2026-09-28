#pragma once
/*====================================
 *
 * 1フレームの絵を作る工程を分割した単位。
 * SceneRenderer が保持するパス列を順に Execute することで1枚の絵が完成する。
 *
 * 【なぜ分けるか】
 * 影・半透明・ポストエフェクトを足すたびに Renderer が肥大するのを防ぐため。
 * 描画機能の追加は「パスを1つ作って列に挿す」だけで済み、
 * Renderer も SceneRenderer も触らずに済む。
 *
 * 【描画先は持たない】
 * どの RenderTarget へ描くかは呼び出し側が Renderer::BeginScene で決める。
 * パスは「今バインドされている描画先へ描く」だけ。
 * 視点は RenderContext で受け取るため、ゲームカメラ視点とエディタカメラ視点で
 * 同じパス列をそのまま使い回せる。
 *
 * 【DX12 の型を持ち込まないこと】
 * ID3D12GraphicsCommandList 等を RenderContext へ入れると全パスが DX12 へ直結し、
 * 将来 RHI を挟む際の書き換え対象がパスの数だけ増える。
 * 低レベルな描画は必ず Renderer 経由で行うこと。
 *
 * ====================================*/
#include "Engine/Render/CameraView.h"

namespace Cake {

class Scene;
class Renderer;
struct GizmoSettings;


// 視点ごとに変えたい描画設定。ゲームビューとエディタビューの差はここへ集める.
struct RenderViewOptions {
	// 非nullのときだけギズモを描く。ゲームビューでは常に nullptr.
	const GizmoSettings* gizmos = nullptr;
};
// 1回の Execute に必要な情報。パスはここから全てを受け取る.
struct RenderContext {
	Scene* scene = nullptr;
	const CameraView* view = nullptr;
	Renderer* renderer = nullptr;
	const RenderViewOptions* options = nullptr;
};

class RenderPass {
public:
	virtual ~RenderPass() = default;

	// プロファイラ表示やエディタでの一覧に使う識別名.
	virtual const char* GetName() const = 0;

	// このパスの処理を実行する.
	virtual void Execute(RenderContext& ctx) = 0;
};

} // namespace Cake
