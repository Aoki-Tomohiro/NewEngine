#pragma once
#include "RootSignature.h"

class PipelineState
{
public:
	void SetRootSignature(const RootSignature* rootSignature);

	void SetInputLayout(UINT numElements, const D3D12_INPUT_ELEMENT_DESC* inputElementDescs);

	void SetVertexShader(const void* binary, size_t size);

	void SetPixelShader(const void* binary, size_t size);

	void SetBlendState(const D3D12_BLEND_DESC& blendDesc);

	void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc);

	void SetRenderTargetFormats(UINT numRTVs, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat);

	void SetSampleMask(UINT sampleMask);

	void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType);

	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);

	void Finalize();

	ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc_{};

	const RootSignature* rootSignature_ = nullptr;
};

