#pragma once
/*====================================
 *
 * Graphics 層と JobSystem の中の数（SRV、転送、解放待ち、ジョブ）を毎フレーム表示するデバッグウィンドウ。
 * 数が増え続けていないか、溜まったまま減らないかを目で確かめるために使う。
 *
 * ====================================*/
#include "Engine/Application/BuildConfig.h"

#ifdef ENABLE_EDITOR
namespace Cake {

class Graphics;
class JobSystem;

class GraphicsDebugWindow {
private:
	Graphics* graphics_ = nullptr;   // 非所有.
	JobSystem* jobSystem_ = nullptr; // 非所有.

public:
	void Initialize(Graphics* graphics, JobSystem* jobSystem);

	// 毎フレーム呼ぶ.
	void Draw(const char* title = "Graphics Debug");
};

} // namespace Cake
#endif
