#include "TextureManager.h"

#include <algorithm>
#include <cassert>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Utility/Convert.h"
#include "Engine/Graphics/Buffer/GPUResourceUtility.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/Device/CommandManager.h"

#include "externals/DirectXTex/d3dx12.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "TextureManager";
}

void TextureManager::Initialize(
	ID3D12Device* device,
	DescriptorHeap* srvHeap,
	CommandManager* commandManager
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	srvHeap_ = srvHeap;
	commandManager_ = commandManager;

	pool_.reserve(64);
	generations_.reserve(64);

	CreateDefaultTexture(); // 白.
	CreateErrorTexture();   // マゼンタ/黒チェッカー.

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

bool TextureManager::IsAlive(TextureHandle handle) const {
	if (!handle.IsValid() || handle.index >= pool_.size()) {
		return false;
	}
	return generations_[handle.index] == handle.generation;
}

// image から SRV を作り pool_ に登録してハンドルを払い出す共通処理.
TextureHandle TextureManager::RegisterTexture(const std::string& filePath, const DirectX::ScratchImage& mipImages) {
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	TextureData td;
	td.filePath = filePath;
	td.resource = CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediate = UploadTextureData(td.resource.Get(), mipImages);

	// 既存：3D描画用（_SRGB、デコードあり）
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format; // 例: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	const uint32_t srvIndex = nextSrvIndex_++;
	device_->CreateShaderResourceView(td.resource.Get(), &srvDesc, srvHeap_->GetCPUHandle(srvIndex));

	// 追加：ImGuiプレビュー用（UNORM、デコードなし・素通し）
	D3D12_SHADER_RESOURCE_VIEW_DESC previewDesc = srvDesc;
	previewDesc.Format = DirectX::MakeSRGB(metadata.format) == metadata.format
	                         ? DirectX::MakeLinear(metadata.format) // _SRGBを剥がしたUNORM相当
	                         : metadata.format;

	const uint32_t previewSrvIndex = nextSrvIndex_++;
	device_->CreateShaderResourceView(td.resource.Get(), &previewDesc, srvHeap_->GetCPUHandle(previewSrvIndex));
	td.previewGpuHandle = srvHeap_->GetGPUHandle(previewSrvIndex);

	// pool_ に載せる位置が index。generation は 1 始まり.
	const uint32_t index = static_cast<uint32_t>(pool_.size());

	td.srvIndex = srvIndex;
	td.textureWidth = static_cast<uint32_t>(metadata.width);
	td.textureHeight = static_cast<uint32_t>(metadata.height);
	td.handle.index = index;
	td.handle.generation = 1;
	td.handle.gpuHandle = srvHeap_->GetGPUHandle(srvIndex);

	commandManager_->ExecuteAndWait();
	commandManager_->ResetForNextFrame();

	TextureHandle result = td.handle;
	pool_.push_back(std::move(td));
	generations_.push_back(1);
	byPath_[filePath] = result;
	return result;
}

TextureHandle TextureManager::Load(const std::string& filePath) {
	// キャッシュ検索（索引 O(1)）.
	auto it = byPath_.find(filePath);
	if (it != byPath_.end()) {
		return it->second;
	}

	// ファイル読み込み＆ミップマップ生成。失敗したらエラーテクスチャを返す（Assert では落とさない）.
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(
		filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image
	);
	if (FAILED(hr)) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "テクスチャ読み込み失敗、エラーテクスチャで代替: " + filePath);
		return errorTexture_;
	}

	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB, 0, mipImages
	);
	if (FAILED(hr)) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "ミップマップ生成失敗、エラーテクスチャで代替: " + filePath);
		return errorTexture_;
	}

	return RegisterTexture(filePath, mipImages);
}

const TextureData* TextureManager::Resolve(TextureHandle handle) const {
	if (!IsAlive(handle)) {
		return nullptr;
	}
	return &pool_[handle.index];
}

const TextureData* TextureManager::ResolveOrError(TextureHandle handle) const {
	if (const TextureData* td = Resolve(handle)) {
		return td;
	}
	// errorTexture_ は Initialize で必ず作られているので有効.
	return Resolve(errorTexture_);
}

TextureHandle TextureManager::FindHandle(const std::string& filePath) const {
	auto it = byPath_.find(filePath);
	if (it == byPath_.end()) {
		return {};
	}
	return it->second;
}

std::vector<std::string> TextureManager::GetLoadedPaths() const {
	std::vector<std::string> paths;
	paths.reserve(pool_.size());
	for (const TextureData& td : pool_) {
		paths.push_back(td.filePath);
	}
	std::sort(paths.begin(), paths.end()); // 一覧の並びを安定させる.
	return paths;
}

const TextureData* TextureManager::GetTextureData(uint32_t srvIndex) const {
	// srvIndex は index とは別物なので、pool_ を srvIndex で引く.
	// （呼び出し頻度が低ければ線形で十分。高頻度化するなら srvIndex→index の索引を足す）.
	for (const TextureData& td : pool_) {
		if (td.srvIndex == srvIndex) {
			return &td;
		}
	}
	return nullptr;
}

void TextureManager::CreateDefaultTexture() {
	DirectX::ScratchImage image;
	HRESULT hr = image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, 1, 1, 1);
	AssertHRESULT(hr, "デフォルトテクスチャの初期化");
	uint8_t* px = image.GetPixels();
	px[0] = 255;
	px[1] = 255;
	px[2] = 255;
	px[3] = 255; // 白.

	defaultTexture_ = RegisterTexture("__default_white__", image);
}

void TextureManager::CreateErrorTexture() {
	// 16x16 のマゼンタ/黒チェッカー。いかにも「欠損」とわかる絵にする.
	const size_t kSize = 16;
	DirectX::ScratchImage image;
	HRESULT hr = image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kSize, kSize, 1, 1);
	AssertHRESULT(hr, "エラーテクスチャの初期化");

	uint8_t* px = image.GetPixels();
	for (size_t y = 0; y < kSize; ++y) {
		for (size_t x = 0; x < kSize; ++x) {
			const bool magenta = ((x / 4) + (y / 4)) % 2 == 0; // 4x4 マスのチェッカー.
			uint8_t* p = px + (y * kSize + x) * 4;
			p[0] = magenta ? 255 : 0; // R.
			p[1] = 0;                 // G.
			p[2] = magenta ? 255 : 0; // B.
			p[3] = 255;               // A.
		}
	}

	errorTexture_ = RegisterTexture("__error_magenta__", image);
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	AssertHRESULT(hr, "TextureResourceの生成");
	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(
	ID3D12Resource* texture,
	const DirectX::ScratchImage& mipImages
) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(
		device_, mipImages.GetImages(), mipImages.GetImageCount(),
		mipImages.GetMetadata(), subresources
	);

	uint64_t intermediateSize = GetRequiredIntermediateSize(
		texture, 0, UINT(subresources.size())
	);

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = Cake::CreateBufferResource(device_, intermediateSize);

	UpdateSubresources(
		commandManager_->GetCommandList(),
		texture, intermediateResource.Get(),
		0, 0, UINT(subresources.size()), subresources.data()
	);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandManager_->GetCommandList()->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

} // namespace Cake
