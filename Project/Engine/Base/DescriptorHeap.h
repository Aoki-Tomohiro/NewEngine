#pragma once
#include "DescriptorHandle.h"
#include <cstdint>
#include <wrl.h>

class DescriptorHeap
{
public:
	void Initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

	DescriptorHandle Allocate();

	uint32_t GetDescriptorSize() const { return descriptorSize_; }

	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap_.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;

	UINT descriptorSize_ = 0;

	uint32_t numFreeDescriptors_ = 0;

	DescriptorHandle firstHandle_;

	DescriptorHandle nextFreeHandle_;
};

