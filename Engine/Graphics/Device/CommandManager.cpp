#include "CommandManager.h"

#include <cassert>
#include "Engine/Foundation/Debug/DebugLog.h" // AssertHRESULTL用

namespace Cake {
namespace {
constexpr const char* kLogCategory = "CommandManager";
}

CommandManager::~CommandManager() {
	if (fenceEvent_ != nullptr) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}
void CommandManager::Initialize(ID3D12Device* device) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);
	HRESULT hr;

	/*
	* コマンドキューの生成
	———————————————*/
	// コマンドキューを生成する.
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	AssertHRESULT(hr, "コマンドキューの生成");

	// コマンドアロケータを生成する.
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	AssertHRESULT(hr, "コマンドアロケータの生成");

	// コマンドリストを生成する.
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	AssertHRESULT(hr, "コマンドリストの生成");

	/*
	* FenceとEventを生成する
	———————————————*/
	// Fenceの生成.
	hr = device->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	AssertHRESULT(hr, "Fenceの生成");

	// FenceEventの生成.
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent_ == nullptr) {
		DebugLog::GetInstance().Log(
			LogLevel::Error,
			"CommandManager",
			"FenceEventの生成に失敗しました"
		);
		assert(false);
	}

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}
void CommandManager::ExecuteAndWait() {
	// コマンドリストをClose.
	HRESULT hr = commandList_->Close();
	AssertHRESULT(hr, "コマンドリストのクローズ");

	// GPUにコマンドリストを実行させる.
	ID3D12CommandList* commandLists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(1, commandLists);

	WaitForGPU();
}
void CommandManager::ResetForNextFrame() {
	HRESULT hr;
	hr = commandAllocator_->Reset();
	AssertHRESULT(hr, "コマンドアロケータのリセット");

	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	AssertHRESULT(hr, "コマンドリストのリセット");
}

void CommandManager::WaitForGPU() {
	// Fenceの値を更新してSignalを送る.
	fenceValue_++;
	commandQueue_->Signal(fence_.Get(), fenceValue_);

	// GPUが追いついていなければイベントで待つ.
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

} // namespace Cake
