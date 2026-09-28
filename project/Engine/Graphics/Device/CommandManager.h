#pragma once

#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

namespace Cake {

class CommandManager {
private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;

	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;

public:
	~CommandManager();
	void Initialize(ID3D12Device* device);

	void ExecuteAndWait();
	void ResetForNextFrame();

	// Getter
	ID3D12GraphicsCommandList* GetCommandList() const {
		return commandList_.Get();
	}
	ID3D12CommandQueue* GetCommandQueue() const {
		return commandQueue_.Get();
	}

	void WaitForGPU();
};

} // namespace Cake
