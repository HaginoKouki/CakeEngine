#pragma once
#include <wrl.h>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

namespace Cake {

class DirectXDevice {
private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

private:
	void EnableDebugLayer();
	void CreateFactory();
	void SelectAdapter();
	void CreateDevice();
	void SetupInfoQueue();

public:
	void Initialize();

	ID3D12Device* GetD3DDevice() const { return device_.Get(); }
	IDXGIFactory7* GetFactory() const { return dxgiFactory_.Get(); }
};

} // namespace Cake
