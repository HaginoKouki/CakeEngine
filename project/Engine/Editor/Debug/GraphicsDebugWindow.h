#pragma once
/*====================================
 *
 * Graphics 層と JobSystem の中の数（SRV、転送、解放待ち、ジョブ）を毎フレーム表示するデバッグウィンドウ。
 * 数が増え続けていないか、溜まったまま減らないかを目で確かめるために使う。
 *
 * ====================================*/
#include <cstdint>
#include <memory>
#include <vector>
#include <functional>
#include <wrl.h>
#include <d3d12.h>

#include "Engine/Graphics/Descriptor/SrvManager.h"
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR
namespace Cake {

class Graphics;
class JobSystem;

class GraphicsDebugWindow {
private:
	// テスト用のテクスチャ1枚.
	struct TestTexture {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex = SrvManager::kInvalidIndex;
		uint32_t width = 0;
		uint32_t height = 0;
	};

	using Pixels = std::shared_ptr<std::vector<uint8_t>>;

	Graphics* graphics_ = nullptr;   // 非所有.
	JobSystem* jobSystem_ = nullptr; // 非所有.

	// ワーカーで作ったテクスチャ.
	TestTexture workerTexture_;

	// 転送が終わって、表示してよいか.
	bool workerTextureReady_ = false;
	// 何枚目のテクスチャか。古い完了処理を見分けるのに使う.
	uint32_t workerTextureSerial_ = 0;
	// ワーカーで作っている最中か（ボタンの連打で積みすぎないため）.
	bool workerJobRunning_ = false;

	// 今のフレームで作ったテクスチャ.
	TestTexture stressTexture_;
	// あと何回、作成と破棄を繰り返すか.
	uint32_t stressRemaining_ = 0;

public:
	~GraphicsDebugWindow();
	void Initialize(Graphics* graphics, JobSystem* jobSystem);

	// 毎フレーム呼ぶ.
	void Draw(const char* title = "Graphics Debug");

private:
	TestTexture CreateTestTexture(uint32_t width, uint32_t height, Pixels pixels, std::function<void()> onComplete);
	void ReleaseTestTexture(TestTexture& texture);
	void DrawWorkerTextureTest();
	void DrawStressTest();
};

} // namespace Cake
#endif
