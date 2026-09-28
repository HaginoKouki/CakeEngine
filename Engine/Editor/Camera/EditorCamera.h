#pragma once
/*====================================
 *
 * エディタのシーンビュー用カメラ。
 * 右ドラッグ中の WASD + QE で移動し、マウス移動で視点を回す。
 *
 * シーンには属さないため GameObject も CameraComponent も持たない。
 * 位置と回転を自前で持ち、要求されたときに CameraView を組み立てて返す。
 *
 * 【アスペクト比は持たない】
 * 射影は GetCameraView() の引数（出力先の解像度）から毎回作る。
 * シーンビューはレターボックスせず、ウィンドウの形をそのまま使う。
 *
 * 【行列は自作しない】
 * ゲームのカメラとまったく同じ経路を通すため、view / projection の組み立ては
 * MakeCameraView に任せる。ここが分岐すると規約のずれに気づけなくなる。
 *
 * ====================================*/
#include <cstdint>
#include <numbers>

#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Math/Matrix.h"
#include "Engine/Render/CameraView.h"

namespace Cake {

class InputManager;

class EditorCamera {
private:
	Vector3 translate_ = {0.0f, 1.0f ,10.0f};
	Vector3 rotation_ = {0.0f, std::numbers::pi_v<float>, 0.0f};

	InputManager* inputManager_ = nullptr;
	bool active_ = true; // falseの間は入力を受け付けない.

	float fovYDegree_ = 60.0f;
	float nearClip_ = 0.1f;
	float farClip_ = 1000.0f;

	float moveSpeed_ = 3.0f;       // 秒あたりの移動量.
	float rotationSpeed_ = 0.005f; // マウス移動1ピクセルあたりのラジアン.

public:
	void Initialize(InputManager* inputManager);
	void Reset();

	// 入力を受けて位置と回転だけを更新する。行列はここでは作らない.
	void Update(float deltaTime);

	void SetActive(bool b) { active_ = b; }

	// 出力先の解像度から射影を作り、描画に使う視点を返す.
	CameraView GetCameraView(uint32_t targetWidth, uint32_t targetHeight) const;

	Vector3 GetTranslate() const { return translate_; }
	Vector3 GetRotation() const { return rotation_; }
	void SetTranslate(const Vector3& translate) { translate_ = translate; }
	void SetRotation(const Vector3& rotation) { rotation_ = rotation; }
};

} // namespace Cake
