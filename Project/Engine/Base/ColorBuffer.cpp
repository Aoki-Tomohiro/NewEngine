#include "ColorBuffer.h"
#include "GraphicsCore.h"
#include <cassert>

void ColorBuffer::CreateFromSwapChain(ID3D12Resource* baseResource)
{
	GraphicsCore* graphicsCore = GraphicsCore::GetInstance();

	ID3D12Device* device = graphicsCore->GetDevice();

	D3D12_RESOURCE_DESC resourceDesc = baseResource->GetDesc();

	resource_.Attach(baseResource);
	currentState_ = D3D12_RESOURCE_STATE_PRESENT;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvHandle_ = graphicsCore->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(resource_.Get(), &rtvDesc, rtvHandle_);
}

void ColorBuffer::Create(uint32_t width, uint32_t height, DXGI_FORMAT format)
{
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	currentState_ = D3D12_RESOURCE_STATE_COMMON;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor_[0];
	clearValue.Color[1] = clearColor_[1];
	clearValue.Color[2] = clearColor_[2];
	clearValue.Color[3] = clearColor_[3];

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, currentState_, &clearValue,
		IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	CreateDerivedViews(device, format);
}

void ColorBuffer::Create(uint32_t width, uint32_t height, DXGI_FORMAT format, float* clearColor)
{
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	currentState_ = D3D12_RESOURCE_STATE_COMMON;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];
	clearColor_[0] = clearColor[0];
	clearColor_[1] = clearColor[1];
	clearColor_[2] = clearColor[2];
	clearColor_[3] = clearColor[3];

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, currentState_, &clearValue,
		IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	CreateDerivedViews(device, format);
}

void ColorBuffer::CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = format;
	rtvHandle_ = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(resource_.Get(), &rtvDesc, rtvHandle_);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvHandle_ = GraphicsCore::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHandle_);
}