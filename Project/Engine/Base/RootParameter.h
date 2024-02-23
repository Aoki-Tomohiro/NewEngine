#pragma once
#include <d3d12.h>

class RootParameter
{
	friend class RootSignature;

public:
	void InitAsConstantBuffer(UINT registerNum, D3D12_SHADER_VISIBILITY shaderVisibility);

	void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT registerNum, UINT count, D3D12_SHADER_VISIBILITY shaderVisibility);

	void InitAsDescriptorTable(UINT rangeCount, D3D12_SHADER_VISIBILITY shaderVisibility);

	void SetTableRange(UINT rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE type, UINT registerNum, UINT count);

	const D3D12_ROOT_PARAMETER& operator()() const { return rootParameter_; };

private:
	D3D12_ROOT_PARAMETER rootParameter_{};
};

