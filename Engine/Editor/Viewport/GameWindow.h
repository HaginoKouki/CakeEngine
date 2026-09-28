#pragma once
/*====================================
 *
 * ゲームカメラ視点のプレビューウィンドウ（ImGui 上の "Game"）。
 *
 * 持つのは「自分の描画先（SceneRenderTarget）」と「自分のフォーカス状態」だけ。
 * 選択中オブジェクトや再生状態といったエディタ全体の状態は持たない。
 *
 * 【1フレームの呼び出し順】
 *   ConsumePendingResize / ResizeRenderTarget … Editor が両ビューぶんまとめて行う。
 *                                               GPUアイドルを1回に抑えるため、ここでは待たない。
 *   Render                                    … RT へ描画コマンドを積む。
 *   Draw                                      … ImGui へ結果を出し、次フレームぶんのリサイズ要求を積む。
 *
 * 【レターボックス】
 * ウィンドウの形に関わらず、ProjectSettings のアスペクト比で描く。
 * 余白は ImGui のウィンドウ背景がそのまま黒帯になる。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR

#include <cstdint>

#include "Engine/Graphics/RenderTarget/SceneRenderTarget.h"

namespace Cake {

class Graphics;
class Renderer;
class SceneRenderer;
class SceneManager;
struct ProjectSettings;

class GameWindow {
private:
	SceneRenderTarget rt_;
	bool focused_ = false;

	/* 非所有。寿命は Editor と同じ */
	Graphics* graphics_ = nullptr;
	Renderer* renderer_ = nullptr;
	SceneRenderer* sceneRenderer_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	ProjectSettings* settings_ = nullptr;

public:
	void Initialize(
		Graphics& graphics, Renderer& renderer, SceneRenderer& sceneRenderer,
		SceneManager& sceneManager, ProjectSettings& settings
	);

	// ゲームカメラ視点をRTへ描く。メインカメラが無ければ何も描かない.
	void Render();

	// ImGui ウィンドウを出す.
	void Draw();

	// 今フレームのフォーカス。Draw の後に読むこと.
	bool IsFocused() const { return focused_; }

	/* --- RT の遅延リサイズ。GPUアイドルの都合で Editor が順番を握る --- */
	bool ConsumePendingResize(uint32_t& width, uint32_t& height) {
		return rt_.ConsumePendingResize(width, height);
	}
	void ResizeRenderTarget(uint32_t width, uint32_t height) {
		rt_.Resize(width, height);
	}
	uint32_t GetRenderTargetWidth() const { return rt_.GetWidth(); }
	uint32_t GetRenderTargetHeight() const { return rt_.GetHeight(); }
};

} // namespace Cake

#endif // ENABLE_EDITOR
