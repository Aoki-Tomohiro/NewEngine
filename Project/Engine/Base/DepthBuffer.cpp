#include "DepthBuffer.h"
#include "GraphicsCore.h"
#include <cassert>

void DepthBuffer::Create(uint32_t width, uint32_t height, DXGI_FORMAT format)
{
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.Format = format;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
		IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	currentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	CreateDerivedViews(device, format);
}

void DepthBuffer::Create(uint32_t width, uint32_t height, DXGI_FORMAT format, bool useAsTexture)
{
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.Format = format;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
		IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	useAsTexture_ = useAsTexture;
	currentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	CreateDerivedViews(device, format);
}

void DepthBuffer::CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	dsvHandle_ = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(resource_.Get(), &dsvDesc, dsvHandle_);

	if (useAsTexture_)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = (format == DXGI_FORMAT_D32_FLOAT) ? DXGI_FORMAT_R32_FLOAT : format;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvHandle_ = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHandle_);
	}
}