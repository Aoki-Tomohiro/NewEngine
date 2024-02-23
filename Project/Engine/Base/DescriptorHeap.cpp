#include "DescriptorHeap.h"
#include "GraphicsCore.h"
#include <cassert>

void DescriptorHeap::Initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible)
{
    //デバイスの取得
    ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();

    //ディスクリプタヒープの作成
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = type;
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap_));
    assert(SUCCEEDED(hr));

    //ディスクリプタサイズの取得
    descriptorSize_ = device->GetDescriptorHandleIncrementSize(type);

    //ディスクリプタヒープの数の初期化
    numFreeDescriptors_ = numDescriptors;

    //ディスクリプタハンドルの初期化
    shaderVisible ? firstHandle_ = DescriptorHandle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorHeap_->GetGPUDescriptorHandleForHeapStart()) : firstHandle_ = DescriptorHandle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart());

    nextFreeHandle_ = firstHandle_;
}

DescriptorHandle DescriptorHeap::Allocate()
{
    assert(numFreeDescriptors_ >= 0);
    DescriptorHandle ret = nextFreeHandle_;
    nextFreeHandle_ += descriptorSize_;
    numFreeDescriptors_--;
    return ret;
}