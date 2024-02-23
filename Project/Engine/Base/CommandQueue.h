#pragma once
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

class CommandQueue
{
public:
	void Initialize();

	void ExecuteCommandList(ID3D12CommandList* commandList[]);

	void WaitForFence();

	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;

	uint64_t fenceValue_{};
};

