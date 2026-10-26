#include "SrvManager.h"

#include <cassert>

namespace Cake {
void SrvManager::Initialize(ID3D12Device* device) {
	heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSrvCount, true);
}

// 次の番号を1つ払い出す。上限を超えたら assert で止める.
uint32_t SrvManager::Allocate() {
	assert(nextIndex_ < kMaxSrvCount);
	return nextIndex_++;
}

} // namespace Cake
