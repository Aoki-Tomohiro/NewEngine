#include "RootParameter.h"

void RootParameter::InitAsConstantBuffer(UINT registerNum, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter_.ShaderVisibility = shaderVisibility;
	rootParameter_.Descriptor.ShaderRegister = registerNum;
}

void RootParameter::InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT registerNum, UINT count, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	InitAsDescriptorTable(1, shaderVisibility);
	SetTableRange(0, type, registerNum, count);
}

void RootParameter::InitAsDescriptorTable(UINT rangeCount, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter_.ShaderVisibility = shaderVisibility;
	rootParameter_.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount]{};
	rootParameter_.DescriptorTable.NumDescriptorRanges = rangeCount;
}

void RootParameter::SetTableRange(UINT rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE type, UINT registerNum, UINT count)
{
	D3D12_DESCRIPTOR_RANGE* descriptorRange = const_cast<D3D12_DESCRIPTOR_RANGE*>(rootParameter_.DescriptorTable.pDescriptorRanges + rangeIndex);
	descriptorRange->BaseShaderRegister = registerNum;
	descriptorRange->NumDescriptors = count;
	descriptorRange->RangeType = type;
	descriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算
}