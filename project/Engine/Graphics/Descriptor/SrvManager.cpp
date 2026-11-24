#include "SrvManager.h"

#include <format>
#include "Engine/Foundation/Debug/DebugLog.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "SrvManager";
}
void SrvManager::Initialize(ID3D12Device* device) {
	heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSrvCount, true);
	indices_.Initialize(kMaxSrvCount);
}

uint32_t SrvManager::Allocate() {
	uint32_t index = indices_.Allocate();
	if (index == kInvalidIndex) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::format("SRV の番号が尽きました（容量：{}）", kMaxSrvCount));
	}
	return index;
}

void SrvManager::Free(uint32_t index) {
	if (!indices_.Free(index)) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::format("SRV の二重解放または、払い出していない番号の解放です（番号：{}）", index));
	}
}

} // namespace Cake
