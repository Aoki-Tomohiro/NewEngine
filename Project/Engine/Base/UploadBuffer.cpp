#include "UploadBuffer.h"
#include "GraphicsCore.h"
#include <cassert>

void UploadBuffer::Create(size_t sizeInBytes)
{
	//デバイスの取得
	ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

	//バッファサイズの初期化
	bufferSize_ = sizeInBytes;

	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う

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
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource_));
	assert(SUCCEEDED(hr));

	//GpuVirtualAddressの初期化
	gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();
}

void* UploadBuffer::Map()
{
	void* memory;
	resource_->Map(0, nullptr, &memory);
	return memory;
}

void UploadBuffer::Unmap()
{
	resource_->Unmap(0, nullptr);
}