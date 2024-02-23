#include "GaussianBlur.h"
#include "Engine/Base/GraphicsCore.h"
#include "Engine/Utilities/ShaderCompiler.h"

//実態定義
std::unique_ptr<RootSignature> GaussianBlur::rootSignature_ = nullptr;
std::array<std::unique_ptr<PipelineState>, GaussianBlur::kCountOfBlurDirection> GaussianBlur::pipelineStates_{};

void GaussianBlur::StaticInitialize()
{
	//RootSignatureの作成
	rootSignature_ = std::make_unique<RootSignature>();
	rootSignature_->Create(2, 1);

	//RootParameterの設定
	(*rootSignature_)[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	(*rootSignature_)[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);

	//StaticSamplerを設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	rootSignature_->InitStaticSampler(0, staticSamplers[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature_->Finalize();

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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob[2];
	vertexShaderBlob[0] = ShaderCompiler::CompileShader(L"HorizontalBlur.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob[0] != nullptr);
	vertexShaderBlob[1] = ShaderCompiler::CompileShader(L"VerticalBlur.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob[1] != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob[2];
	pixelShaderBlob[0] = ShaderCompiler::CompileShader(L"HorizontalBlur.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob[0] != nullptr);
	pixelShaderBlob[1] = ShaderCompiler::CompileShader(L"VerticalBlur.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob[1] != nullptr);

	//書き込むRTVの情報
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//PSOを作成する
	for (uint32_t i = 0; i < pipelineStates_.size(); ++i)
	{
		pipelineStates_[i] = std::make_unique<PipelineState>();
		pipelineStates_[i]->SetRootSignature(rootSignature_.get());
		pipelineStates_[i]->SetInputLayout(2, inputElementDescs);
		pipelineStates_[i]->SetVertexShader(vertexShaderBlob[i]->GetBufferPointer(), vertexShaderBlob[i]->GetBufferSize());
		pipelineStates_[i]->SetPixelShader(pixelShaderBlob[i]->GetBufferPointer(), pixelShaderBlob[i]->GetBufferSize());
		pipelineStates_[i]->SetBlendState(blendDesc);
		pipelineStates_[i]->SetRasterizerState(rasterizerDesc);
		pipelineStates_[i]->SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);
		pipelineStates_[i]->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pipelineStates_[i]->SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		pipelineStates_[i]->Finalize();
	}
}

void GaussianBlur::Destroy()
{
	if (rootSignature_)
	{
		rootSignature_.reset();
	}

	for (uint32_t i = 0; i < pipelineStates_.size(); ++i)
	{
		if (pipelineStates_[i])
		{
			pipelineStates_[i].reset();
		}
	}
}

void GaussianBlur::Initialize(uint32_t width, uint32_t height)
{
	textureWidth_ = width;
	textureHeight_ = height;

	//ぼかし用のリソースを作成
	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };
	for (uint32_t i = 0; i < kCountOfBlurDirection; ++i)
	{
		blurBuffers_[i] = std::make_unique<ColorBuffer>();
		blurBuffers_[i]->Create(width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, clearColor);
	}

	//ConstBufferの作成
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataGaussianBlur));
	Update();
}

void GaussianBlur::Update()
{
	ConstBuffDataGaussianBlur* gaussianBlurData = static_cast<ConstBuffDataGaussianBlur*>(constBuff_->Map());
	gaussianBlurData->textureWidth = textureWidth_;
	gaussianBlurData->textureHeight = textureHeight_;
	float total = 0.0f;
	for (int i = 0; i < 8; i++) {
		gaussianBlurData->weight[i] = expf(-(i * i) / (2 * sigma_ * sigma_));
		total += gaussianBlurData->weight[i];
	}
	total = total * 2.0f - 1.0f;
	//最終的な合計値で重みをわる
	for (int i = 0; i < 8; i++) {
		gaussianBlurData->weight[i] /= total;
	}
	constBuff_->Unmap();
}

void GaussianBlur::PreBlur(uint32_t index)
{
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();

	//リソースの状態遷移
	commandContext->TransitionResource(*blurBuffers_[index], D3D12_RESOURCE_STATE_RENDER_TARGET);

	//RenderTargetを設定
	commandContext->SetRenderTargets(1, &blurBuffers_[index]->GetRTVHandle());

	//RenderTargetをクリア
	commandContext->ClearColor(*blurBuffers_[index]);

	//RootSignatureを設定
	commandContext->SetRootSignature(*rootSignature_);

	//PipelineStateを設定
	commandContext->SetPipelineState(*pipelineStates_[index]);

	//CBVを設定
	commandContext->SetConstantBuffer(1, constBuff_->GetGpuVirtualAddress());

	//ビューポート
	D3D12_VIEWPORT viewport{};
	viewport.Width = FLOAT(textureWidth_);
	viewport.Height = FLOAT(textureHeight_);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	commandContext->SetViewport(viewport);

	//シザー矩形
	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = textureWidth_;
	scissorRect.top = 0;
	scissorRect.bottom = textureHeight_;
	commandContext->SetScissor(scissorRect);
}

void GaussianBlur::PostBlur(uint32_t index)
{
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();
	commandContext->TransitionResource(*blurBuffers_[index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}