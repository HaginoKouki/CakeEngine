#include "GPUResourceUtility.h"

#include <numbers>
#include <cmath>

#include <fstream>
#include <sstream>

#include "Engine/Foundation/Debug/DebugLog.h"

#include "Engine/Asset/Material/Material.h"

namespace Cake {

// バッファリソースを生成する関数.
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// 頂点リソース用のヒープの設定.
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う.
	// 頂点リソースの設定.
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする.
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes; // リソースのサイズ。今回はVector4を3頂点分.
	// バッファの場合は1にする決まり.
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり.
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;

	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&newResource));
	AssertHRESULT(hr, "バッファリソースの生成");

	return newResource;
}

} // namespace Cake
