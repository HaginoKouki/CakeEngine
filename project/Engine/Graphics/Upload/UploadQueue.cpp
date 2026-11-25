#include "UploadQueue.h"

#include "externals/DirectXTex/d3dx12.h"

#include "Engine/Graphics/Upload/UploadBudget.h"
#include "Engine/Graphics/Device/DeferredReleaseQueue.h"
#include "Engine/Graphics/Buffer/GPUResourceUtility.h"

namespace Cake {

void UploadQueue::Initialize(ID3D12Device* device, DeferredReleaseQueue* deferredReleaseQueue) {
	// ▼ 2つのポインタを控える.
	device_ = device;
	deferredReleaseQueue_ = deferredReleaseQueue;
}

void UploadQueue::Enqueue(UploadRequest request) {
	// 一時バッファの大きさを求める.
	size_t bufferSize = GetRequiredIntermediateSize(request.destination.Get(), 0, static_cast<UINT>(request.subresources.size()));
	// ▼ Pending に request（std::move する）と大きさを入れて、pending_ の末尾に入れる.
	pending_.push_back({std::move(request), bufferSize});
}

void UploadQueue::Record(ID3D12GraphicsCommandList* commandList) {
	bytesLastFrame_ = 0;
	if (pending_.empty()) {
		return;
	}

	// 件数を求める.
	std::vector<uint64_t> sizes;
	for (auto& pending : pending_) {
		sizes.push_back(pending.bufferSize);
	}
	size_t uploadCount = CountUploadsThisFrame(sizes, kUploadBytesPerFrame);

	for (size_t i = 0; i < uploadCount; i++) {
		// pending_ の先頭から要素を取り出す.
		Pending pending = std::move(pending_.front());
		pending_.pop_front();

		// CreateBufferResource で、一時バッファを作る.
		auto bufferResource = CreateBufferResource(device_, pending.bufferSize);

		// UpdateSubresources(commandList, 転送先, 一時バッファ, 0, 0, subresources の数, subresources の先頭) で、データを写してコピーの命令を積む.
		UpdateSubresources(
			commandList,
			pending.request.destination.Get(),
			bufferResource.Get(),
			0, 0,
			static_cast<UINT>(pending.request.subresources.size()),
			pending.request.subresources.data()
		);

		// 転送先を COPY_DEST から stateAfter へ移すバリアを積む（今の UploadTextureData のバリアと同じ形）.
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pending.request.destination.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = pending.request.stateAfter;
		commandList->ResourceBarrier(1, &barrier);

		// 一時バッファを DeferredReleaseQueue に預ける。一時バッファを値でキャプチャするだけの、空のラムダを Enqueue すればよい.
		//  ラムダが解放されるときに、キャプチャした ComPtr も解放される.
		deferredReleaseQueue_->Enqueue([bufferResource]() {});

		// onComplete を recorded_ の末尾に入れる（空の std::function なら入れない）.
		if (pending.request.onComplete) {
			recorded_.push_back(std::move(pending.request.onComplete));
		}

		// bytesLastFrame_ に大きさを足す.
		bytesLastFrame_ += pending.bufferSize;
	}
}

void UploadQueue::CloseFrame(uint64_t fenceValue) {
	// 完了処理が積まれていないなら何もしない.
	if (recorded_.empty()) {
		return;
	}

	// Batch を作り、fenceValue と、recorded_ を入れて、batches_ の末尾に入れる.
	Batch batch;
	batch.fenceValue = fenceValue;
	batch.completions = std::move(recorded_);
	batches_.push_back(std::move(batch));

	// pending_ を空にする.
	recorded_.clear();
}

void UploadQueue::ProcessCompleted(uint64_t completedValue) {
	for (auto it = batches_.begin(); it != batches_.end();) {
		if (it->fenceValue <= completedValue) {
			// 先頭の fenceValue が completedValue 以下なら、その completions を全部呼んでから pop_front する.
			for (auto& completion : it->completions) {
				completion();
			}
			it = batches_.erase(it);
		} else {
			// completedValue より大きければ、そこで止める（後ろはもっと新しいので、まだ解放できない）.
			break;
		}
	}
}

} // namespace Cake
