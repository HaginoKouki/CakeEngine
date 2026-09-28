#include "LightManager.h"

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Buffer/GPUResourceUtility.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "LightManager";
}

void LightManager::Initialize(ID3D12Device* device) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	directionalResource_ = CreateBufferResource(device, sizeof(DirectionalLight));
	directionalResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalData_));

	// 既定値（SetDirectional が呼ばれるまでの保険）.
	directionalData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalData_->intensity = 1.0f;

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

} // namespace Cake
