#include "Bloom.h"
#include "Engine/Base/GraphicsCore.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Utilities/ShaderCompiler.h"

void Bloom::Initialize()
{
	//GaussianBlurの生成
	for (uint32_t i = 0; i < kMaxBlurCount; ++i)
	{
		gaussianBlurs_[i] = std::make_unique<GaussianBlur>();
		gaussianBlurs_[i]->Initialize(Application::kClientWidth / (i + 1), Application::kClientHeight / (i + 1));
	}

	//ColorBufferの生成
	highLumColorBuffer_ = std::make_unique<ColorBuffer>();
	highLumColorBuffer_->Create(Application::kClientWidth, Application::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	bloomColorBuffer_ = std::make_unique<ColorBuffer>();
	bloomColorBuffer_->Create(Application::kClientWidth, Application::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	//ConstBufferの生成
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataBloom));
	Update();

	//高輝度用のPipelineStateを作成
	CreateHighLumPipelineState();

	//Bloom用のPipelineStateを作成
	CreateBloomPipelineState();
}

void Bloom::Update()
{
	ConstBuffDataBloom* bloomData = static_cast<ConstBuffDataBloom*>(constBuff_->Map());
	bloomData->isEnable = isEnable_;
	bloomData->intensity = intensity_;
	bloomData->textureWeight = textureWeight_;
	bloomData->highLumTextureWeight = highLumTextureWeight_;
	for (uint32_t i = 0; i < blurCount_; ++i)
	{
		bloomData->blurTextureWeight[i] = blurTextureWeight_[i];
	}
	constBuff_->Unmap();
}

void Bloom::Apply(const DescriptorHandle& srvHandle)
{
	//コマンドリストを取得
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();

	//高輝度を描画
	RenderHighLuminance(srvHandle);

	//GaussianBlurを適用
	ApplyGaussianBlur();

	//リソースの状態遷移
	commandContext->TransitionResource(*bloomColorBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//RenderTargetを設定
	commandContext->SetRenderTargets(1, &bloomColorBuffer_->GetRTVHandle());

	//RenderTargetをクリア
	commandContext->ClearColor(*bloomColorBuffer_);

	//RootSignatureを設定
	commandContext->SetRootSignature(bloomRootSignature_);

	//PipelineStateを設定
	commandContext->SetPipelineState(bloomPipelineState_);

	//DescriptorTableを設定
	commandContext->SetDescriptorTable(0, srvHandle);
	commandContext->SetDescriptorTable(1, highLumColorBuffer_->GetSRVHandle());
	commandContext->SetDescriptorTable(2, gaussianBlurs_[0]->GetDescriptorHandle(GaussianBlur::kVertical));
	commandContext->SetDescriptorTable(3, gaussianBlurs_[1]->GetDescriptorHandle(GaussianBlur::kVertical));
	commandContext->SetDescriptorTable(4, gaussianBlurs_[2]->GetDescriptorHandle(GaussianBlur::kVertical));
	commandContext->SetDescriptorTable(5, gaussianBlurs_[3]->GetDescriptorHandle(GaussianBlur::kVertical));

	//CBVを設定
	commandContext->SetConstantBuffer(6, constBuff_->GetGpuVirtualAddress());

	//ビューポート
	D3D12_VIEWPORT viewport{ 0.0f, 0.0f, Application::kClientWidth, Application::kClientHeight, 0.0f, 1.0f };
	commandContext->SetViewport(viewport);

	//シザー矩形を設定
	D3D12_RECT scissorRect{ 0, 0, Application::kClientWidth, Application::kClientHeight };
	commandContext->SetScissor(scissorRect);

	//DrawCall
	commandContext->DrawInstanced(6, 1);

	//リソースの状態遷移
	commandContext->TransitionResource(*bloomColorBuffer_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Bloom::CreateHighLumPipelineState()
{
	//RootSignatureの作成
	highLumRootSignature_.Create(1, 1);

	//RootParameterの設定
	highLumRootSignature_[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	//StaticSamplerを設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	highLumRootSignature_.InitStaticSampler(0, staticSamplers[0], D3D12_SHADER_VISIBILITY_PIXEL);
	highLumRootSignature_.Finalize();

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//シェーダーをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	vertexShaderBlob = ShaderCompiler::CompileShader(L"HighLum.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	pixelShaderBlob = ShaderCompiler::CompileShader(L"HighLum.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//書き込むRTVの情報
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//PSOを作成する
	highLumPipelineState_.SetRootSignature(&highLumRootSignature_);
	highLumPipelineState_.SetInputLayout(2, inputElementDescs);
	highLumPipelineState_.SetVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize());
	highLumPipelineState_.SetPixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize());
	highLumPipelineState_.SetBlendState(blendDesc);
	highLumPipelineState_.SetRasterizerState(rasterizerDesc);
	highLumPipelineState_.SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);
	highLumPipelineState_.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	highLumPipelineState_.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	highLumPipelineState_.Finalize();
}

void Bloom::CreateBloomPipelineState()
{
	//RootSignatureの作成
	bloomRootSignature_.Create(7, 1);

	//RootParameterの設定
	bloomRootSignature_[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_[5].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_[6].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);

	//StaticSamplerを設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	bloomRootSignature_.InitStaticSampler(0, staticSamplers[0], D3D12_SHADER_VISIBILITY_PIXEL);
	bloomRootSignature_.Finalize();

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//シェーダーをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	vertexShaderBlob = ShaderCompiler::CompileShader(L"Bloom.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	pixelShaderBlob = ShaderCompiler::CompileShader(L"Bloom.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//書き込むRTVの情報
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//PSOを作成する
	bloomPipelineState_.SetRootSignature(&bloomRootSignature_);
	bloomPipelineState_.SetInputLayout(2, inputElementDescs);
	bloomPipelineState_.SetVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize());
	bloomPipelineState_.SetPixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize());
	bloomPipelineState_.SetBlendState(blendDesc);
	bloomPipelineState_.SetRasterizerState(rasterizerDesc);
	bloomPipelineState_.SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);
	bloomPipelineState_.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	bloomPipelineState_.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	bloomPipelineState_.Finalize();
}

void Bloom::RenderHighLuminance(const DescriptorHandle& srvHandle)
{
	//コマンドリストを取得
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();

	//リソースの状態遷移
	commandContext->TransitionResource(*highLumColorBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//RenderTargetを設定
	commandContext->SetRenderTargets(1, &highLumColorBuffer_->GetRTVHandle());

	//RenderTargetをクリア
	commandContext->ClearColor(*highLumColorBuffer_);

	//RootSignatureを設定
	commandContext->SetRootSignature(highLumRootSignature_);

	//PipelineStateを設定
	commandContext->SetPipelineState(highLumPipelineState_);

	//DescriptorTableを設定
	commandContext->SetDescriptorTable(0, srvHandle);

	//DrawCall
	commandContext->DrawInstanced(6, 1);

	//リソースの状態遷移
	commandContext->TransitionResource(*highLumColorBuffer_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Bloom::ApplyGaussianBlur()
{
	//コマンドリストを取得
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();

	for (uint32_t i = 0; i < kMaxBlurCount; ++i)
	{
		//横ぼかし
		gaussianBlurs_[i]->PreBlur(GaussianBlur::kHorizontal);

		if (i < blurCount_)
		{
			//DescriptorTableを設定
			commandContext->SetDescriptorTable(0, (i == 0) ? highLumColorBuffer_->GetSRVHandle() : gaussianBlurs_[i - 1]->GetDescriptorHandle(GaussianBlur::kVertical));

			//DrawCall
			commandContext->DrawInstanced(6, 1);
		}

		gaussianBlurs_[i]->PostBlur(GaussianBlur::kHorizontal);

		//縦ぼかし
		gaussianBlurs_[i]->PreBlur(GaussianBlur::kVertical);

		if (i < blurCount_)
		{
			//DescriptorTableを設定
			commandContext->SetDescriptorTable(0, gaussianBlurs_[i]->GetDescriptorHandle(GaussianBlur::kHorizontal));

			//DrawCall
			commandContext->DrawInstanced(6, 1);
		}

		gaussianBlurs_[i]->PostBlur(GaussianBlur::kVertical);
	}
}