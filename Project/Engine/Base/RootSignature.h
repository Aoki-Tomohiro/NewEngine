#pragma once
#include "RootParameter.h"
#include <cassert>
#include <wrl.h>
#include <memory>

class RootSignature
{
public:
	void Create(UINT numParameters, UINT numSamplers);

	void Finalize();

	void InitStaticSampler(UINT registerNum, const D3D12_STATIC_SAMPLER_DESC& nonStaticSamplerDesc, D3D12_SHADER_VISIBILITY shaderVIsibility);

	RootParameter& operator[] (size_t entryIndex)
	{
		assert(entryIndex < numRootParameters_);
		return rootParameters_.get()[entryIndex];
	}

	const RootParameter& operator[](size_t entryIndex) const
	{
		assert(entryIndex < numStaticSamplers_);
		return rootParameters_.get()[entryIndex];
	}

	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	std::unique_ptr<RootParameter[]> rootParameters_ = nullptr;

	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> staticSamplers_ = nullptr;

	UINT numRootParameters_ = 0;

	UINT numStaticSamplers_ = 0;

	UINT numInitializedStaticSamplers_ = 0;
};

