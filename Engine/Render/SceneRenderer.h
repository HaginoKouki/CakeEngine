#pragma once
/*====================================
 *
 * シーンを1回描くためのパス列を保持し、順に実行するクラス。
 *
 * 描く相手がゲームカメラでもエディタカメラでも手順は変わらないため、
 * ここに一本化することで、視点を差し替えるだけで同じ絵が出ることを保証する。
 *
 * 【視点は受け取るだけ】
 * カメラの選択も射影の組み立ても呼び出し側の仕事。
 * エディタカメラはシーンに属さないため、視点の作り方まで抱えると分岐が生まれる。
 *
 * 【描画先も知らない】
 * どの RenderTarget へ描くかは Renderer::BeginScene 等で呼び出し側が決める。
 *
 * 【パス列の順序が絵の順序】
 * Initialize で組み立てた順にそのまま実行される。
 * 描画機能を足すときは、新しい RenderPass を作って列へ挿すこと。
 * このクラス自体を書き換える必要はない。
 *
 * ====================================*/
#include <cstdint>
#include <memory>
#include <vector>

#include "Engine/Render/CameraView.h"
#include "Engine/Render/Pass/RenderPass.h"

namespace Cake {

class Scene;
class Renderer;

class SceneRenderer {
private:
	Renderer* renderer_ = nullptr; // 非所有.

	// 実行順に並んだパス列。先頭から順に Execute される.
	std::vector<std::unique_ptr<RenderPass>> passes_;

public:
	// 既定のパス列（LightSetup → Skybox → Opaque）を組み立てる.
	void Initialize(Renderer* renderer);

	// 渡された視点でシーンを1回描く.
	void Render(Scene& scene, const CameraView& view, const RenderViewOptions& options = {});

	// シーン内の有効なカメラから視点を組み立てる.
	// 射影は出力先の解像度から作るため、描画先のサイズを渡すこと.
	// カメラが1つも無ければ false（out は触らない）.
	bool TryMakeMainCameraView(
		Scene& scene,
		uint32_t targetWidth,
		uint32_t targetHeight,
		CameraView& out
	);
};

} // namespace Cake
