#pragma once
#include "GpuResource.h"
#include "DescriptorHandle.h"
#include <cstdint>

class ColorBuffer : public GpuResource
{
public:
	void CreateFromSwapChain(ID3D12Resource* baseResource);

	void Create(uint32_t width, uint32_t height, DXGI_FORMAT format);

	void Create(uint32_t width, uint32_t height, DXGI_FORMAT format, float* clearColor);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTVHandle() const { return rtvHandle_; };

	const DescriptorHandle& GetSRVHandle() const { return srvHandle_; };

	const float* GetClearColor() const { return clearColor_; };

private:
	void CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format);

private:
	DescriptorHandle srvHandle_{};

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};

	float clearColor_[4]{ 0.1f, 0.25f, 0.5f, 1.0f };
};

