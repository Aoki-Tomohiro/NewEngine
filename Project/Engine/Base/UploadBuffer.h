#pragma once
#include "GpuResource.h"

class UploadBuffer : public GpuResource
{
public:
	void Create(size_t sizeInBytes);

	void* Map();

	void Unmap();

	size_t GetBufferSize() const { return bufferSize_; };

private:
	size_t bufferSize_ = 0;
};

