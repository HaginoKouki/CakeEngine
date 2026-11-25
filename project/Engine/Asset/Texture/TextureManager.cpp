#include "TextureManager.h"

#include <algorithm>
#include <cassert>
#include <format>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Utility/Convert.h"
#include "Engine/Graphics/Buffer/GPUResourceUtility.h"
#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Graphics/Upload/UploadQueue.h"

#include "externals/DirectXTex/d3dx12.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "TextureManager";
}

void TextureManager::Initialize(
	ID3D12Device* device,
	SrvManager* srvManager,
	UploadQueue* uploadQueue
) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	srvManager_ = srvManager;
	uploadQueue_ = uploadQueue;

	pool_.reserve(64);
	generations_.reserve(64);

	CreateWhiteTexture(); // 白.
	CreateErrorTexture();   // マゼンタ/黒チェッカー.

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
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

	return RegisterTexture(filePath, std::move(mipImages), false);
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

void TextureManager::CreateWhiteTexture() {
	DirectX::ScratchImage image;
	HRESULT hr = image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, 1, 1, 1);
	AssertHRESULT(hr, "デフォルトテクスチャの初期化");
	uint8_t* px = image.GetPixels();
	px[0] = 255;
	px[1] = 255;
	px[2] = 255;
	px[3] = 255; // 白.

	whiteTexture_ = RegisterTexture("__default_white__", std::move(image), true);
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

	errorTexture_ = RegisterTexture("__error_magenta__", std::move(image), true);
}

TextureHandle TextureManager::RegisterTexture(const std::string& filePath, DirectX::ScratchImage mipImages, bool resident) {
	const DirectX::TexMetadata metadata = mipImages.GetMetadata();

#pragma region Srvの番号を貰う
	uint32_t srvIndex = srvManager_->Allocate();
	uint32_t previewSrvIndex = SrvManager::kInvalidIndex;

	if (srvIndex == SrvManager::kInvalidIndex) {
		if (IsAlive(errorTexture_)) {
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::format("SRV の番号を確保できず、エラーテクスチャで代替: {}", filePath));
			return errorTexture_;
		} else {
			assert(false && "SRV の番号が尽きて、既定のテクスチャを作れません。SrvManager::kMaxSrvCount を確かめてください");
			return {};
		}
	}
#ifdef ENABLE_EDITOR
	previewSrvIndex = srvManager_->Allocate();
	if (previewSrvIndex == SrvManager::kInvalidIndex) {
		srvManager_->Free(srvIndex);
		if (IsAlive(errorTexture_)) {
			DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, std::format("プレビュー用 SRV の番号を確保できず、エラーテクスチャで代替: {}", filePath));
			return errorTexture_;
		} else {
			assert(false && "SRV の番号が尽きて、既定のテクスチャを作れません。SrvManager::kMaxSrvCount を確かめてください");
			return {};
		}
	}
#endif
#pragma endregion

	// リソースを作る.
	TextureData td;
	td.filePath = filePath;
	td.resource = CreateTextureResource(metadata); // COPY_DEST の状態で作られる.

	// 画像を shared_ptr に引っ越させる.
	auto image = std::make_shared<DirectX::ScratchImage>(std::move(mipImages));

	// 転送するデータ（画像の各ミップマップ）の置き場所を作る.
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	HRESULT hr = DirectX::PrepareUpload(device_, image->GetImages(), image->GetImageCount(), image->GetMetadata(), subresources);
	AssertHRESULT(hr, "転送データの準備");

	// pool_ に載せる位置を決めて、番号とハンドルを設定する.
	const uint32_t index = static_cast<uint32_t>(pool_.size());
	td.srvIndex = srvIndex;
	td.textureWidth = static_cast<uint32_t>(metadata.width);
	td.textureHeight = static_cast<uint32_t>(metadata.height);
	td.handle.index = index;
	td.handle.generation = 1;
	td.handle.gpuHandle = srvManager_->GetGPUHandle(srvIndex);
#ifdef ENABLE_EDITOR
	td.previewGpuHandle = srvManager_->GetGPUHandle(previewSrvIndex);
#endif

	if (resident) {
		// 常駐のテクスチャなら本物を書く.
		// Initialize で作るので、1フレーム目の先頭で、描画より前に転送される.
		WriteSrvs(td.resource.Get(), srvIndex, previewSrvIndex);
	} else {
		// 常駐のテクスチャでないなら、本物が転送されるまで白テクスチャを書いておく.
		WriteSrvs(pool_[whiteTexture_.index].resource.Get(), srvIndex, previewSrvIndex);
	}

	// 転送を予約する。常駐のテクスチャでなければ、転送が終わったら本物の SRV に書き直す処理（onComplete）を付ける
	UploadRequest request;
	request.destination = td.resource;
	request.subresources = std::move(subresources);
	request.owner = image;
	request.stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	if (!resident) {
		request.onComplete = [this, index, srvIndex, previewSrvIndex]() {
			// 本物の SRV を書き直す.
			WriteSrvs(pool_[index].resource.Get(), srvIndex, previewSrvIndex);
		};
	}
	uploadQueue_->Enqueue(std::move(request));

	// TextureData を pool_ に、ハンドルを byPath_ に登録してから返す.
	TextureHandle result = td.handle;
	pool_.push_back(std::move(td));
	generations_.push_back(1);
	byPath_[filePath] = result;
	return result;
}

void TextureManager::WriteSrvs(ID3D12Resource* resource, uint32_t srvIndex, uint32_t previewSrvIndex) {
	// リソースの情報を取る.
	const D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();

	// SrvDescを作る.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resourceDesc.Format; // 例: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

	// 3D描画用のSRV.
	device_->CreateShaderResourceView(resource, &srvDesc, srvManager_->GetCPUHandle(srvIndex));

#ifdef ENABLE_EDITOR
	// ImGuiプレビュー用（UNORM、デコードなし・素通し）
	D3D12_SHADER_RESOURCE_VIEW_DESC previewDesc = srvDesc;
	previewDesc.Format = DirectX::MakeSRGB(resourceDesc.Format) == resourceDesc.Format
	                         ? DirectX::MakeLinear(resourceDesc.Format) // _SRGBを剥がしたUNORM相当
	                         : resourceDesc.Format;

	device_->CreateShaderResourceView(resource, &previewDesc, srvManager_->GetCPUHandle(previewSrvIndex));
#endif
}

bool TextureManager::IsAlive(TextureHandle handle) const {
	if (!handle.IsValid() || handle.index >= pool_.size()) {
		return false;
	}
	return generations_[handle.index] == handle.generation;
}

} // namespace Cake
