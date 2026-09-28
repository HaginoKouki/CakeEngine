#include "PlayModeController.h"

#include "Engine/Platform/Time/Time.h"
#include "Engine/Foundation/Debug/DebugLog.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Serialize/SceneSerializer.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "PlayModeController";
}

bool PlayModeController::Play(Scene& scene) {
	if (state_ != PlayModeState::Edit) {
		return false;
	}

	snapshot_ = SceneSerializer::SaveSceneToString(scene);
	if (snapshot_.empty()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "スナップショットの作成に失敗しました");
		return false;
	}

	DebugLog::GetInstance().Log(
		LogLevel::Info, kLogCategory,
		"Play開始 (スナップショット " + std::to_string(snapshot_.size()) + " バイト)"
	);
	state_ = PlayModeState::Play;
	return true;
}

bool PlayModeController::Stop(Scene& scene, AssetDatabase& assetDatabase) {
	if (state_ != PlayModeState::Play && state_ != PlayModeState::Paused) {
		return false;
	}

	if (!SceneSerializer::LoadSceneFromString(scene, snapshot_, assetDatabase)) {
		// 復元できなかった場合、控えは捨てない。ここで捨てると編集内容が完全に失われる.
		DebugLog::GetInstance().Log(
			LogLevel::Error, kLogCategory,
			"復元に失敗しました。Play状態のままにします"
		);
		return false;
	}

	state_ = PlayModeState::Edit;
	snapshot_.clear();
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, "Stop: シーンを復元しました");
	return true;
}
void PlayModeController::Pause() {
	if (state_ == PlayModeState::Play) {
		state_ = PlayModeState::Paused;
	}
}
void PlayModeController::Resume() {
	if (state_ == PlayModeState::Paused) {
		state_ = PlayModeState::Play;
	}
}
void PlayModeController::RequestStep() {
	if (state_ == PlayModeState::Paused) {
		stepRequested_ = true;
	}
}

void PlayModeController::Tick(Time& time) {
	if (state_ == PlayModeState::Paused && stepRequested_) {
		stepRequested_ = false;
		time.SetPaused(false); // このフレームだけ時間を通す.
		return;
	}
	time.SetPaused(state_ != PlayModeState::Play);
}

} // namespace Cake
