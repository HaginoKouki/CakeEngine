#include "EditorCamera.h"

#include <algorithm>
#include <cmath>

#include "Engine/Platform/Input/InputManager.h"

namespace Cake {
namespace {
// 真上・真下を越えると視界が反転するので、手前で止める（89度）.
constexpr float kPitchLimit = 1.55334f;
constexpr float kTwoPi = std::numbers::pi_v<float> * 2.0f;
} // namespace

void EditorCamera::Initialize(InputManager* inputManager) {
	inputManager_ = inputManager;
	Reset();
}

void EditorCamera::Reset() {
	translate_ = {0.0f, 1.0f, 10.0f};
	rotation_ = {0.0f, std::numbers::pi_v<float>, 0.0f};
}

void EditorCamera::Update(float deltaTime) {
	if (!active_) {
		return;
	}
	if (!IsPressed(inputManager_->GetMouseButtonState(MouseButton::Right))) {
		return;
	}

	// 回転を先に反映すると、同じフレームの移動が新しい向きに沿う.
	const Vector2 mouseVelocity = inputManager_->GetMouseVelocity();
	if (mouseVelocity.x != 0.0f || mouseVelocity.y != 0.0f) {
		// 横に振る＝Y軸まわり（yaw）、縦に振る＝X軸まわり（pitch）.
		rotation_.y += mouseVelocity.x * rotationSpeed_;
		rotation_.x += mouseVelocity.y * rotationSpeed_;

		// yaw は一周（2π）で畳む。pitch は畳まず、真上・真下の手前で止める.
		rotation_.y = std::fmod(rotation_.y, kTwoPi);
		rotation_.x = std::clamp(rotation_.x, -kPitchLimit, kPitchLimit);
	}

	const Matrix4x4 rotationMatrix = Matrix4x4::MakeRotationMatrix(rotation_);
	const Vector3 right = {rotationMatrix.m[0][0], rotationMatrix.m[0][1], rotationMatrix.m[0][2]};
	const Vector3 up = {rotationMatrix.m[1][0], rotationMatrix.m[1][1], rotationMatrix.m[1][2]};
	const Vector3 forward = {rotationMatrix.m[2][0], rotationMatrix.m[2][1], rotationMatrix.m[2][2]};

	const float distance = moveSpeed_ * deltaTime;
	if (IsPressed(inputManager_->GetRawKeyState(KeyCode::W))) {
		translate_ += forward * distance;
	}
	if (IsPressed(inputManager_->GetRawKeyState(KeyCode::S))) {
		translate_ -= forward * distance;
	}
	if (IsPressed(inputManager_->GetRawKeyState(KeyCode::D))) {
		translate_ += right * distance;
	}
	if (IsPressed(inputManager_->GetRawKeyState(KeyCode::A))) {
		translate_ -= right * distance;
	}
	if (IsPressed(inputManager_->GetRawKeyState(KeyCode::E))) {
		translate_ += up * distance;
	}
	if (IsPressed(inputManager_->GetRawKeyState(KeyCode::Q))) {
		translate_ -= up * distance;
	}
}

CameraView EditorCamera::GetCameraView(uint32_t targetWidth, uint32_t targetHeight) const {
	// GameObject を持たないので world 行列だけここで作り、以降の組み立てはゲームのカメラと同じ MakeCameraView に通す.
	const Matrix4x4 world = Matrix4x4::MakeRotationMatrix(rotation_) * Matrix4x4::MakeTranslateMatrix(translate_);

	return MakeCameraView(world, fovYDegree_, nearClip_, farClip_, targetWidth, targetHeight);
}

} // namespace Cake
