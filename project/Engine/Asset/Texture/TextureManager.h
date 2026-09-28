#pragma once
/*====================================
 *
 * テクスチャファイルの読み込みと SRV（ShaderResourceView）の生成・管理を行うマネージャ。
 * ファイルパスをキーにテクスチャをキャッシュし、同じテクスチャの重複読み込みを防ぐ。
 *
 * 【ハンドル方式（MaterialManager / ModelManager と統一）】
 * テクスチャ実体は pool_（std::vector）に連続配置し、外へは TextureHandle を払い出す。
 *   - index + generation … 3マネージャ共通の作法。将来アンロードで generation が効く（予約）。
 *   - gpuHandle 併せ持ち … テクスチャは GPU ディスクリプタが主役で、値で持ち回れて安定なので、
 *                          ハンドルに gpuHandle を同梱する。これにより描画側は Resolve 不要で
 *                          そのまま SetGraphicsRootDescriptorTable に渡せる（使い勝手優先）。
 *
 * 注意：index（pool_ の添字）と srvIndex（SRVヒープ上の位置）は別物。
 *   - index    … pool_ での配列添字。ハンドルの本体。
 *   - srvIndex … ディスクリプタヒープ上の位置。
 *                TextureData 側が保持する。
 *
 * 読み込み失敗・無効ハンドル時は errorTexture_（マゼンタ/黒チェッカー）を返し、
 * エンジンを落とさず「欠損が目に見える」状態で描画を継続する（MaterialManager のエラーマテリアルと対称）。
 *
 * ====================================*/
#include <string>
#include <vector>
#include <unordered_map>
#include <wrl.h>
#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"

#include "Engine/Graphics/Descriptor/DescriptorHeap.h"
#include "Engine/Asset/Texture/TextureHandle.h"

namespace Cake {

class CommandManager;

struct TextureData {
	TextureHandle handle;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_GPU_DESCRIPTOR_HANDLE previewGpuHandle{}; // ImGui::Image用（デコードなし）
	std::string filePath{};
	uint32_t srvIndex = 0; // SRVヒープ上の位置（index とは別物）.
	uint32_t textureWidth = 0;
	uint32_t textureHeight = 0;
};

class TextureManager {
private:
	ID3D12Device* device_ = nullptr;
	DescriptorHeap* srvHeap_ = nullptr;
	CommandManager* commandManager_ = nullptr;

	// テクスチャ実体を連続配置で所有する.
	std::vector<TextureData> pool_;
	// pool_ と同じ添字。スロットの世代（将来アンロードでインクリメント）.
	std::vector<uint32_t> generations_;
	// パス → ハンドル。重複ロード検出・パス引き用の索引.
	std::unordered_map<std::string, TextureHandle> byPath_;

	uint32_t nextSrvIndex_ = Reserved::kSrvReservedCount;

public:
	TextureHandle defaultTexture_{}; // 1x1 白（未割り当てスロット用）.
	TextureHandle errorTexture_{};   // マゼンタ/黒チェッカー（読込失敗・無効時）.

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
		ID3D12Resource* texture,
		const DirectX::ScratchImage& mipImages
	);

	void CreateDefaultTexture();
	void CreateErrorTexture();

	// image から SRV を作り、pool_ に登録してハンドルを払い出す共通処理.
	TextureHandle RegisterTexture(const std::string& filePath, const DirectX::ScratchImage& mipImages);

	// ハンドルが今有効か（index 範囲内 かつ generation 一致）.
	bool IsAlive(TextureHandle handle) const;

public:
	void Initialize(
		ID3D12Device* device,
		DescriptorHeap* srvHeap,
		CommandManager* commandManager
	);

	// テクスチャを読み込んでSRVを生成、ハンドルを返す。
	// 読み込みに失敗した場合は errorTexture_ を返す（Assert では落とさない）。
	TextureHandle Load(const std::string& filePath);

	TextureHandle GetDefaultTexture() const { return defaultTexture_; }
	TextureHandle GetErrorTexture() const { return errorTexture_; }

	// --- ハンドル解決（MaterialManager と同じ作法）---
	// 有効なら実体を返す。無効・破棄済みなら nullptr.
	const TextureData* Resolve(TextureHandle handle) const;
	// 無効時に errorTexture_ の実体を返す（必ず non-null）。表示・情報取得を落とさない.
	const TextureData* ResolveOrError(TextureHandle handle) const;

	// パスからハンドルを引く（見つからなければ無効ハンドル）.
	TextureHandle FindHandle(const std::string& filePath) const;

	// 読み込み済みテクスチャのファイルパス一覧（ImGuiのテクスチャ選択用、ソート済み）.
	std::vector<std::string> GetLoadedPaths() const;

	// 後方互換：srvIndex から TextureData を引く（線形探索を廃し索引化）。
	const TextureData* GetTextureData(uint32_t srvIndex) const;
	// 後方互換：ハンドルから TextureData を引く（= Resolve）。
	const TextureData* GetTextureData(const TextureHandle& handle) const { return Resolve(handle); }
};

} // namespace Cake
