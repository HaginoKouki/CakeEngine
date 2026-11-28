#pragma once
#include <wrl.h>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

namespace Cake {

struct DirectXDeviceDesc {
	bool enableDebugLayer = false;    // デバッグレイヤーを有効にする.
	bool enableGpuValidation = false; // GPU ベースの検証も有効にする（とても遅い）.
};

class DirectXDevice {
private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

public:
	void Initialize(const DirectXDeviceDesc& desc);

	ID3D12Device* GetD3D12Device() const { return device_.Get(); }
	IDXGIFactory7* GetFactory() const { return dxgiFactory_.Get(); }

private:
	void EnableDebugLayer(const DirectXDeviceDesc& desc);
	void CreateFactory();
	void SelectAdapter();
	void CreateDevice();
	void SetupInfoQueue(const DirectXDeviceDesc& desc);
};

} // namespace Cake
