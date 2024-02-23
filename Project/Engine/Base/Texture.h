#pragma once
#include "GpuResource.h"
#include "DescriptorHandle.h"
#include "Engine/Externals/DirectXTex/DirectXTex.h"
#include "Engine/Externals/DirectXTex/d3dx12.h"
#include <cstdint>
#include <string>

class Texture : public GpuResource
{
public:
	void Create(const DirectX::ScratchImage& mipImages);

	const DescriptorHandle& GetSRVHandle() const { return srvHandle_; }

	const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resourceDesc_; };

private:
	void CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format);

	void UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

private:
	D3D12_RESOURCE_DESC resourceDesc_{};

	DescriptorHandle srvHandle_{};
};

