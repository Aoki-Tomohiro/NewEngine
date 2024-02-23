#pragma once
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"

class CommandContext
{
public:
	void Initialize();

	void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState);

	void SetRenderTargets(UINT num, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[]);

	void SetRenderTargets(UINT num, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[], D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

	void ClearColor(ColorBuffer& target);

	void ClearDepth(DepthBuffer& target);

	void SetViewport(const D3D12_VIEWPORT& viewport);

	void SetScissor(const D3D12_RECT& rect);

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* descriptorHeap);

	void SetVertexBuffer(const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);

	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);

	void SetConstantBuffer(UINT rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv);

	void SetDescriptorTable(UINT rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void SetRootSignature(const RootSignature& rootSignature);

	void SetPipelineState(const PipelineState& pipelineState);

	void DrawInstanced(UINT vertexCount, UINT instanceCount);

	void Close();

	void Reset();

	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); };

private:
	void BindDescriptorHeaps();

protected:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

	ID3D12RootSignature* currentRootSignature_ = nullptr;

	ID3D12PipelineState* currentPipelineState_ = nullptr;

	ID3D12DescriptorHeap* currentDescriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

