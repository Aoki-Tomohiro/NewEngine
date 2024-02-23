#include "CommandQueue.h"
#include "GraphicsCore.h"
#include <cassert>

void CommandQueue::Initialize()
{
	//デバイスを取得
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	//コマンドキューを作成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	//コマンドキューの作成がうまくできなかったので起動できない
	assert(SUCCEEDED(hr));

	//フェンスの作成
	hr = device->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));
}

void CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList[])
{
	commandQueue_->ExecuteCommandLists(1, commandList);
}

void CommandQueue::WaitForFence()
{
	//Fenceの値を更新
	fenceValue_++;
	//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue_->Signal(fence_.Get(), fenceValue_);

	//Fenceの値が指定したSignal値にたどり着いているか確認する
	//GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		//FenceのSignalを待つためのイベントを作成する
		HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fenceEvent != nullptr);

		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent);

		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);

		//Fenceのイベントを閉じる
		CloseHandle(fenceEvent);
	}
}