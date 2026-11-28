#pragma once
/*====================================
 *
 * GPU への転送を予約で受け付け、フレームの先頭で上限まで、コピーの命令をコマンドリストに積む
 * 一時バッファと転送先は DeferredReleaseQueue に預け、GPU がコピーを終えるまで生かす
 * （呼ぶ側が予約の直後に転送先を手放しても、コピーの途中で解放されないようにするため）
 * 完了処理は、GPU が転送を終えた次のフレームの先頭で呼ぶ
 * メインスレッド専用。終了のときは、残った予約と完了処理を実行せずに捨てる
 *
 * ====================================*/
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <vector>
#include <wrl.h>
#include <d3d12.h>

namespace Cake {

class DeferredReleaseQueue;

// ▼ 予約1件を表す struct UploadRequest を作る。メンバーは次のとおり.
struct UploadRequest {
	// 転送先。COPY_DEST の状態で作っておくこと.
	Microsoft::WRL::ComPtr<ID3D12Resource> destination;
	// 転送するデータの場所.
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	// データの持ち主。Record で写したら手放す.
	std::shared_ptr<const void> owner;
	// 転送後の状態.
	D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	// GPU が転送を終えたあとに、メインスレッドで呼ぶ.
	std::function<void()> onComplete;
};

class UploadQueue {
private:
	// 予約と、Enqueue のときに計算した一時バッファの大きさ.
	struct Pending {
		UploadRequest request;
		uint64_t bufferSize;
	};
	std::deque<Pending> pending_;

	// フェンスの番号ごとの、GPU に送った転送の完了処理.
	struct Batch {
		uint64_t fenceValue;
		std::vector<std::function<void()>> completions;
	};
	// このフレームで積んだ完了処理（まだ番号が付いていない）.
	std::vector<std::function<void()>> recorded_;
	// フェンスの番号が付いた完了処理.
	std::deque<Batch> batches_;

	ID3D12Device* device_ = nullptr;
	DeferredReleaseQueue* deferredReleaseQueue_ = nullptr;

	uint64_t bytesLastFrame_ = 0; // 直前の Record で転送した量.

public:
	void Initialize(ID3D12Device* device, DeferredReleaseQueue* deferredReleaseQueue);

	// 予約を積む。一時バッファの大きさは、ここで GetRequiredIntermediateSize で計算する.
	void Enqueue(UploadRequest request);

	// 上限まで予約を取り出し、コピーとバリアを積む.
	// フレームの先頭で呼ぶ.
	void Record(ID3D12GraphicsCommandList* commandList);

	// このフレームで積んだ完了処理に番号を付ける.
	// フレームの終わり（ExecuteAndWait の後）に呼ぶ.
	void CloseFrame(uint64_t fenceValue);

	// 番号が completedValue 以下の完了処理を呼ぶ.
	// フレームの先頭（Record より前）に呼ぶ.
	void ProcessCompleted(uint64_t completedValue);

	size_t GetPendingCount() const { return pending_.size(); }
	uint64_t GetBytesLastFrame() const { return bytesLastFrame_; }
};

} // namespace Cake
