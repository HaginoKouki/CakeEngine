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

uint32_t SrvManager::GetIndexFromCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) const {
	// 0 番の CPU ハンドル（heap_.GetCPUHandle(0)）の ptr を start として取る.
	auto start = heap_.GetCPUHandle(0).ptr;

	// handle.ptr が start より小さければ、このヒープのハンドルではないので kInvalidIndex を返す.
	if (handle.ptr < start) {
		return kInvalidIndex;
	}

	// (handle.ptr - start) を heap_.GetSize() で割って、番号を求める.
	const SIZE_T offset = handle.ptr - start;
	if (offset % heap_.GetSize() != 0) {
		return kInvalidIndex;
	}
	SIZE_T result = offset / heap_.GetSize();
	if (result >= kMaxSrvCount) {
		return kInvalidIndex;
	}
	return static_cast<uint32_t>(result);
}

} // namespace Cake
