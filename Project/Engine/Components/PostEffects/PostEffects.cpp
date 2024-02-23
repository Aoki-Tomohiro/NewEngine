#include "PostEffects.h"
#include "Engine/Base/GraphicsCore.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Utilities/ShaderCompiler.h"

PostEffects* PostEffects::instance_ = nullptr;

PostEffects* PostEffects::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new PostEffects();
	}
	return instance_;
}

void PostEffects::Destroy()
{
	if (instance_ != nullptr)
	{
		delete instance_;
		instance_ = nullptr;
	}
	//GaussianBlurの開放
	GaussianBlur::Destroy();
}

void PostEffects::Initialize()
{
	//GaussianBlurの静的初期化
	GaussianBlur::StaticInitialize();

	//VertexBufferの作成
	CreateVertexBuffer();

	//PipelineStateの作成
	CreatePipelineState();

	//DepthOfFieldの初期化
	depthOfField_ = std::make_unique<DepthOfField>();
	depthOfField_->Initialize();

	//Bloomの初期化
	bloom_ = std::make_unique<Bloom>();
	bloom_->Initialize();

	//LensDistortionの初期化
	lensDistortion_ = std::make_unique<LensDistortion>();
	lensDistortion_->Initialize();

	//Vignetteの初期化
	vignette_ = std::make_unique<Vignette>();
	vignette_->Initialize();

	//Fogの初期化
	fog_ = std::make_unique<Fog>();
	fog_->Initialize();
}

void PostEffects::Update()
{
	//DepthOfFieldの更新
	depthOfField_->Update();

	//Bloomの更新
	bloom_->Update();

	//Fogの更新
	fog_->Update();

	//LensDistortionの更新
	lensDistortion_->Update();

	//Vignetteの更新
	vignette_->Update();
}

void PostEffects::Apply()
{
	//コマンドリストを取得
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();

	//VertexBufferViewを設定
	commandContext->SetVertexBuffer(vertexBufferView_);

	//形状を設定
	commandContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ポストエフェクトが無効なら処理を飛ばす
	if (!isEnable_)
	{
		return;
	}

	//DepthOfFieldを適用
	depthOfField_->Apply(Renderer::GetInstance()->GetSceneColorDescriptorHandle());

	//Bloomを適用
	bloom_->Apply(depthOfField_->GetDescriptorHandle());

	//Fogを適用
	fog_->Apply(bloom_->GetDescriptorHandle());
}

void PostEffects::Draw()
{
	//コマンドリストを取得
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();

	//RootSignatureを設定
	commandContext->SetRootSignature(rootSignature_);

	//PipelineStateを設定
	commandContext->SetPipelineState(pipelineState_);

	//DescriptorTableを設定
	commandContext->SetDescriptorTable(0, Renderer::GetInstance()->GetSceneColorDescriptorHandle());
	commandContext->SetDescriptorTable(1, fog_->GetDescriptorHandle());

	//CBVを設定
	commandContext->SetConstantBuffer(2, lensDistortion_->GetConstBuffer()->GetGpuVirtualAddress());
	commandContext->SetConstantBuffer(3, vignette_->GetConstBuffer()->GetGpuVirtualAddress());

	//DrawCall
	commandContext->DrawInstanced(6, 1);
}

void PostEffects::CreateVertexBuffer()
{
	//頂点の作成
	vertices_[0] = VertexDataPosUV{ {-1.0f,-1.0f,1.0,1.0f},{0.0f,1.0f} };
	vertices_[1] = VertexDataPosUV{ {-1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f} };
	vertices_[2] = VertexDataPosUV{ {1.0f,-1.0f,1.0f,1.0f},{1.0f,1.0f} };
	vertices_[3] = VertexDataPosUV{ {-1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f} };
	vertices_[4] = VertexDataPosUV{ {1.0f,1.0f,1.0f,1.0f},{1.0f,0.0f} };
	vertices_[5] = VertexDataPosUV{ {1.0f,-1.0f,1.0f,1.0f},{1.0f,1.0f} };

	//頂点リソースを作る
	vertexBuffer_ = std::make_unique<UploadBuffer>();
	vertexBuffer_->Create(sizeof(VertexDataPosUV) * 6);
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGpuVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexDataPosUV) * 6;
	//頂点1つあたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexDataPosUV);

	//頂点バッファにデータを書き込む
	VertexDataPosUV* vertexData = static_cast<VertexDataPosUV*>(vertexBuffer_->Map());
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexDataPosUV) * vertices_.size());
	vertexBuffer_->Unmap();
}

void PostEffects::CreatePipelineState()
{
	//RootSignatureの作成
	rootSignature_.Create(4, 1);

	//RootParameterの設定
	rootSignature_[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature_[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature_[2].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature_[3].InitAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_PIXEL);

	//StaticSamplerを設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	rootSignature_.InitStaticSampler(0, staticSamplers[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature_.Finalize();

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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = ShaderCompiler::CompileShader(L"PostEffects.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = ShaderCompiler::CompileShader(L"PostEffects.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//書き込むRTVの情報
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//PSOを作成する
	pipelineState_.SetRootSignature(&rootSignature_);
	pipelineState_.SetInputLayout(2, inputElementDescs);
	pipelineState_.SetVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize());
	pipelineState_.SetPixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize());
	pipelineState_.SetBlendState(blendDesc);
	pipelineState_.SetRasterizerState(rasterizerDesc);
	pipelineState_.SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
	pipelineState_.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pipelineState_.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	pipelineState_.SetDepthStencilState(depthStencilDesc);
	pipelineState_.Finalize();
}