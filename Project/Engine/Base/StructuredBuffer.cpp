#include "StructuredBuffer.h"
#include "GraphicsCore.h"
#include <cassert>

void StructuredBuffer::Create(uint32_t numElements, uint32_t elementSize)
{
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	elementCount_ = numElements;
	elementSize_ = elementSize;
	bufferSize_ = numElements * elementSize;

	currentState_ = D3D12_RESOURCE_STATE_GENERIC_READ;

	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	//リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = bufferSize_;
	//バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//作成
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, currentState_, nullptr,
		IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	CreateDerivedViews(device, numElements, elementSize);
}

void StructuredBuffer::CreateDerivedViews(ID3D12Device* device, uint32_t numElements, uint32_t elementSize)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = UINT(elementSize);
	srvHandle_ = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHandle_);
}

void* StructuredBuffer::Map()
{
	void* memory;
	resource_->Map(0, nullptr, &memory);
	return memory;
}

void StructuredBuffer::Unmap()
{
	resource_->Unmap(0, nullptr);
}