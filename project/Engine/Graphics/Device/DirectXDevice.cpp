#include "DirectXDevice.h"

#include <cassert>
#include <format>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Utility/Convert.h"

namespace Cake {

namespace {
constexpr const char* kLogCategory = "DirectXDevice";
}

void DirectXDevice::Initialize() {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	EnableDebugLayer();
	CreateFactory();
	SelectAdapter();
	CreateDevice();
	SetupInfoQueue();

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

void DirectXDevice::EnableDebugLayer() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (SUCCEEDED(hr)) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
	} else {
		DebugLog::GetInstance().Log(
			LogLevel::Warn,
			kLogCategory,
			"デバッグレイヤーの有効化に失敗しました"
		);
		DebugLog::GetInstance().LogHRESULT(hr);
	}
#endif
}

void DirectXDevice::CreateFactory() {
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	AssertHRESULT(hr, "DXGIFactoryの生成");
}

void DirectXDevice::SelectAdapter() {
	for (UINT i = 0;
	     dxgiFactory_->EnumAdapterByGpuPreference(
			 i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			 IID_PPV_ARGS(&useAdapter_)
		 ) != DXGI_ERROR_NOT_FOUND;
	     ++i) {
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter_->GetDesc3(&adapterDesc);
		AssertHRESULT(hr, "アダプタの情報取得");

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			DebugLog::GetInstance().Log(
				LogLevel::Info,
				kLogCategory,
				ConvertString(std::format(L"Use Adapter:{}", adapterDesc.Description))
			);
			break;
		}
		useAdapter_ = nullptr;
	}
	assert(useAdapter_ != nullptr);
}

void DirectXDevice::CreateDevice() {
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		HRESULT hr = D3D12CreateDevice(
			useAdapter_.Get(),
			featureLevels[i],
			IID_PPV_ARGS(&device_)
		);
		if (SUCCEEDED(hr)) {
			DebugLog::GetInstance().Log(
				LogLevel::Info,
				kLogCategory,
				std::format("Feature Level: {}", featureLevelStrings[i])
			);
			break;
		}
	}
	assert(device_ != nullptr);
	DebugLog::GetInstance().Log(
		LogLevel::Info,
		kLogCategory,
		"Complete create D3D12Device!!!"
	);
}

void DirectXDevice::SetupInfoQueue() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる.
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージのID.
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ.
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制するレベル.
		D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する.
		infoQueue->PushStorageFilter(&filter);
	}
#endif
}

} // namespace Cake
