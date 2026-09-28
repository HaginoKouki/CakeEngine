// Engine/Graphics/GPUResourceUtility.h
#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace Cake {

// Uploadヒープ上にバッファリソースを1つ生成する（頂点/インデックス/定数バッファ共通の下請け）.
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

} // namespace Cake
