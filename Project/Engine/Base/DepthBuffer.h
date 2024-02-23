#pragma once
#include "GpuResource.h"
#include "DescriptorHandle.h"
#include <cstdint>

class DepthBuffer : public GpuResource
{
public:
	void Create(uint32_t width, uint32_t height, DXGI_FORMAT format);

	void Create(uint32_t width, uint32_t height, DXGI_FORMAT format, bool useAsTexture);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() const { return dsvHandle_; };

	const DescriptorHandle& GetSRVHandle() const { return srvHandle_; };

private:
	void CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};

	DescriptorHandle srvHandle_{};

	bool useAsTexture_ = false;
};

