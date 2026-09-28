#include "SceneManager.h"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Scene/Serialize/SceneSerializer.h"
#include "Engine/Scene/System/ComponentUpdateSystem.h"
#include "Engine/Scene/System/RenderSystem.h"
#include "Engine/Scene/System/CollisionSystem.h"
#include "Engine/Scene/System/UpdateContext.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "SceneManager";
}

void SceneManager::Initialize(Platform* platform, AssetDatabase* assetDatabase) {
	assetDatabase_ = assetDatabase;
	platform_ = platform;
	input_ = &platform->GetInput();

		// 未設定だとコンポーネントが一切動かず、原因が分かりにくいので明示的に落とす.
	if (input_ == nullptr) {
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			"InputManager が未設定です。コンポーネントの Update が走りません。"
		);
	}
}

void SceneManager::Update(float deltaTime, float unscaledDeltaTime) {
	// dt=0 を渡すだけだと入力に即応するコンポーネントが動くため、呼び出しごと飛ばす.
	if (deltaTime > 0.0f && input_ != nullptr) {
		const UpdateContext ctx{
			.scene = scene_,
			.input = *input_,
			.time = platform_->GetTime(),
			.sound = platform_->GetSound(),
			.deltaTime = deltaTime,
			.unscaledDeltaTime = unscaledDeltaTime,
		};
		RunComponentUpdates(ctx);
	}
	// インスペクタでの編集を反映するため、こちらは常に走らせる.
	scene_.UpdateTransforms();

	if (deltaTime > 0.0f) {
		RunCollisionDetection(scene_);
	}
}

void SceneManager::CreateEmptyScene() {
	scene_.Clear();
	currentPath_.clear();
}

bool SceneManager::SaveScene() {
	if (currentPath_.empty()) {
		return false; // 保存先が未確定。SaveSceneAs を使うこと.
	}
	return SceneSerializer::SaveScene(scene_, currentPath_);
}

bool SceneManager::SaveSceneAs(const std::string& path) {
	if (!SceneSerializer::SaveScene(scene_, path)) {
		return false; // 失敗したパスを保存先にしない.
	}
	currentPath_ = path;
	return true;
}

bool SceneManager::LoadScene(const std::string& path) {
	if (!SceneSerializer::LoadScene(scene_, path, *assetDatabase_)) {
		return false;
	}
	// 初回の解決に失敗した参照を拾い直す（解決済みは素通りする）.
	ResolveSceneAssets(scene_, *assetDatabase_);
	currentPath_ = path;
	return true;
}

} // namespace Cake
