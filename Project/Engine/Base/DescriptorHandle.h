#pragma once
#include <d3d12.h>

class DescriptorHandle
{
public:
	DescriptorHandle() = default;

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) : cpuHandle_(cpuHandle), gpuHandle_(gpuHandle) {};

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) : cpuHandle_(cpuHandle) {};

	void operator+=(UINT descriptorSize)
	{
		cpuHandle_.ptr += descriptorSize;
		gpuHandle_.ptr += descriptorSize;
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &cpuHandle_; }

	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return cpuHandle_; };

	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return gpuHandle_; };

private:
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_{};

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_{};
};

