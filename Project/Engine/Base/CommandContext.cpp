#include "CommandContext.h"
#include "GraphicsCore.h"
#include <cassert>

void CommandContext::Initialize()
{
	//デバイスを取得
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	//コマンドアロケータを作成する
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	//コマンドアロケーターの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを作成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	//コマンドリストの作成がうまくできなかったので起動できない
	assert(SUCCEEDED(hr));
}

void CommandContext::TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState)
{
	D3D12_RESOURCE_STATES oldState = resource.currentState_;

	if (oldState != newState)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource.GetResource();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = oldState;
		barrier.Transition.StateAfter = newState;
		resource.currentState_ = newState;
		commandList_->ResourceBarrier(1, &barrier);
	}
}

void CommandContext::SetRenderTargets(UINT num, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[])
{
	commandList_->OMSetRenderTargets(num, rtvHandles, false, nullptr);
}

void CommandContext::SetRenderTargets(UINT num, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[], D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
	commandList_->OMSetRenderTargets(num, rtvHandles, false, &dsvHandle);
}

void CommandContext::ClearColor(ColorBuffer& target)
{
	commandList_->ClearRenderTargetView(target.GetRTVHandle(), target.GetClearColor(), 0, nullptr);
}

void CommandContext::ClearDepth(DepthBuffer& target)
{
	commandList_->ClearDepthStencilView(target.GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void CommandContext::SetViewport(const D3D12_VIEWPORT& viewport)
{
	commandList_->RSSetViewports(1, &viewport);
}

void CommandContext::SetScissor(const D3D12_RECT& rect)
{
	commandList_->RSSetScissorRects(1, &rect);
}

void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* descriptorHeap)
{
	if (currentDescriptorHeaps_[type] != descriptorHeap)
	{
		currentDescriptorHeaps_[type] = descriptorHeap;
		BindDescriptorHeaps();
	}
}

void CommandContext::SetVertexBuffer(const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
{
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	commandList_->IASetPrimitiveTopology(primitiveTopology);
}

void CommandContext::SetConstantBuffer(UINT rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv)
{
	commandList_->SetGraphicsRootConstantBufferView(rootParameterIndex, cbv);
}

void CommandContext::SetDescriptorTable(UINT rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	commandList_->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHandle);
}

void CommandContext::SetRootSignature(const RootSignature& rootSignature)
{
	if (rootSignature.GetRootSignature() == currentRootSignature_)
	{
		return;
	}
	commandList_->SetGraphicsRootSignature(currentRootSignature_ = rootSignature.GetRootSignature());
}

void CommandContext::SetPipelineState(const PipelineState& pipelineState)
{
	if (pipelineState.GetPipelineState() == currentPipelineState_)
	{
		return;
	}
	commandList_->SetPipelineState(currentPipelineState_ = pipelineState.GetPipelineState());
}

void CommandContext::DrawInstanced(UINT vertexCount, UINT instanceCount)
{
	commandList_->DrawInstanced(vertexCount, instanceCount, 0, 0);
}

void CommandContext::Close()
{
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));
}

void CommandContext::Reset()
{
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));

	if (currentRootSignature_)
	{
		commandList_->SetGraphicsRootSignature(currentRootSignature_);
	}

	if (currentPipelineState_)
	{
		commandList_->SetPipelineState(currentPipelineState_);
	}

	BindDescriptorHeaps();
}

void CommandContext::BindDescriptorHeaps()
{
	UINT nonNullHeaps = 0;
	ID3D12DescriptorHeap* heapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		ID3D12DescriptorHeap* heapIter = currentDescriptorHeaps_[i];
		if (heapIter != nullptr)
		{
			heapsToBind[nonNullHeaps++] = heapIter;
		}
	}

	if (nonNullHeaps > 0)
	{
		commandList_->SetDescriptorHeaps(nonNullHeaps, heapsToBind);
	}
}