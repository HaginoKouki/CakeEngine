#include "externals/doctest/doctest.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>


TEST_CASE("WARP: D3D12 のデバイスを作れる" * doctest::may_fail()) {
	using Microsoft::WRL::ComPtr;
	ComPtr<IDXGIFactory7> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device> device;

	// ▼ CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)) で、IDXGIFactory4 を作る.
	//    REQUIRE(SUCCEEDED(hr)) で確かめる（REQUIRE は失敗したら、そこでテストを止める）.
	HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	REQUIRE(SUCCEEDED(hr));

	// ▼ factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)) で、WARP のアダプターを受け取る.
	//    これも REQUIRE(SUCCEEDED(hr)) で確かめる.
	hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
	REQUIRE(SUCCEEDED(hr));

	// ▼ D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)) で、デバイスを作る.
	//    エンジン（DirectXDevice::CreateDevice）が最低限求めるのが 12.0 なので、それに合わせる.
	//    CHECK(SUCCEEDED(hr)) で確かめる.
	hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
	REQUIRE(SUCCEEDED(hr));

	// ▼ 作れたら、MESSAGE("WARP でデバイスを作れました"); で、結果に1行出しておく.
	//    MESSAGE は、テストの出力に文を出す doctest のマクロ。CI のログで、通ったことが目で見てわかる.
	MESSAGE("WARP でデバイスを作れました");
}
