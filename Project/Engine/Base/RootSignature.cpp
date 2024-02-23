#include "RootSignature.h"
#include "GraphicsCore.h"
#include "Engine/Utilities/Log.h"

void RootSignature::Create(UINT numRootParameters, UINT numStaticSamplers)
{
	if (numRootParameters > 0)
	{
        rootParameters_.reset(new RootParameter[numRootParameters]{});
        numRootParameters_ = numRootParameters;
	}

	if (numStaticSamplers > 0)
	{
        staticSamplers_.reset(new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers]{});
        numStaticSamplers_ = numStaticSamplers;
	}
}

void RootSignature::Finalize()
{
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    descriptionRootSignature.NumParameters = numRootParameters_;
    descriptionRootSignature.pParameters = (const D3D12_ROOT_PARAMETER*)rootParameters_.get();
    descriptionRootSignature.NumStaticSamplers = numStaticSamplers_;
    descriptionRootSignature.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)staticSamplers_.get();

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        MyUtility::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元に生成
    ID3D12Device* device = GraphicsCore::GetInstance()->GetDevice();
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void RootSignature::InitStaticSampler(UINT registerNum, const D3D12_STATIC_SAMPLER_DESC& nonStaticSamplerDesc, D3D12_SHADER_VISIBILITY shaderVIsibility)
{
    assert(numInitializedStaticSamplers_ < numStaticSamplers_);
    D3D12_STATIC_SAMPLER_DESC& staticSamplerDesc = staticSamplers_[numInitializedStaticSamplers_++];
    staticSamplerDesc.Filter = nonStaticSamplerDesc.Filter;
    staticSamplerDesc.AddressU = nonStaticSamplerDesc.AddressU;
    staticSamplerDesc.AddressV = nonStaticSamplerDesc.AddressV;
    staticSamplerDesc.AddressW = nonStaticSamplerDesc.AddressW;
    staticSamplerDesc.ComparisonFunc = nonStaticSamplerDesc.ComparisonFunc;
    staticSamplerDesc.MaxLOD = nonStaticSamplerDesc.MaxLOD;
    staticSamplerDesc.ShaderRegister = registerNum;
    staticSamplerDesc.ShaderVisibility = shaderVIsibility;
}