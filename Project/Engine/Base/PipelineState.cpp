#include "PipelineState.h"
#include "GraphicsCore.h"
#include <cassert>

void PipelineState::SetRootSignature(const RootSignature* rootSignature)
{
	rootSignature_ = rootSignature;
}

void PipelineState::SetInputLayout(UINT numElements, const D3D12_INPUT_ELEMENT_DESC* inputElementDescs)
{
	pipelineStateDesc_.InputLayout.pInputElementDescs = inputElementDescs;
	pipelineStateDesc_.InputLayout.NumElements = numElements;
}

void PipelineState::SetVertexShader(const void* binary, size_t size)
{
	pipelineStateDesc_.VS = { binary,size };
}

void PipelineState::SetPixelShader(const void* binary, size_t size)
{
	pipelineStateDesc_.PS = { binary,size };
}

void PipelineState::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
{
	pipelineStateDesc_.BlendState = blendDesc;
}

void PipelineState::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc)
{
	pipelineStateDesc_.RasterizerState = rasterizerDesc;
}

void PipelineState::SetRenderTargetFormats(UINT numRTVs, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat)
{
	pipelineStateDesc_.NumRenderTargets = numRTVs;
	pipelineStateDesc_.DSVFormat = dsvFormat;
	pipelineStateDesc_.SampleDesc.Count = 1;
	pipelineStateDesc_.SampleDesc.Quality = 0;
	for (uint32_t i = 0; i < pipelineStateDesc_.NumRenderTargets; ++i)
	{
		pipelineStateDesc_.RTVFormats[i] = rtvFormats[i];
	}
}

void PipelineState::SetSampleMask(UINT sampleMask)
{
	pipelineStateDesc_.SampleMask = sampleMask;
}

void PipelineState::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType)
{
	pipelineStateDesc_.PrimitiveTopologyType = primitiveTopologyType;
}

void PipelineState::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc)
{
	pipelineStateDesc_.DepthStencilState = depthStencilDesc;
}

void PipelineState::Finalize()
{
	pipelineStateDesc_.pRootSignature = rootSignature_->GetRootSignature();
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();
	HRESULT hr = device->CreateGraphicsPipelineState(&pipelineStateDesc_, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));
}