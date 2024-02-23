#include "GraphicsCore.h"
#include "Engine/Utilities/Log.h"
#include "Engine/Utilities/ShaderCompiler.h"
#include <cassert>

//実体定義
GraphicsCore* GraphicsCore::instance_ = nullptr;

GraphicsCore* GraphicsCore::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new GraphicsCore();
	}
	return instance_;
}

void GraphicsCore::Destroy()
{
	if (instance_)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

void GraphicsCore::Initialize()
{
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	//DXGIファクトリーの作成
	//HRESULTはWindows系のエラーコードであり、//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));


	//良い順にアダプタを読む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) !=
		DXGI_ERROR_NOT_FOUND; ++i)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter_->GetDesc3(&adapterDesc);
		//取得できなかったら止める
		assert(SUCCEEDED(hr));
		//ソフトウェアアダプタでなければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//採用したアダプタの情報をログに出力
			MyUtility::Log(MyUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		//ソフトウェアアダプタの場合は見なかったことにする
		useAdapter_ = nullptr;
	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter_ != nullptr);


	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		//採用したアダプターでデバイス作成
		hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr))
		{
			//生成できたのでログ出力を行ってループを抜ける
			MyUtility::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくできなかったので起動できない
	assert(device_ != nullptr);
	//初期化完了のログを出す
	MyUtility::Log("Complete Create D3D12Device!!!\n");


#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に泊まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に泊まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] =
		{
			//Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			//https://stackoverflow.com/questions/69805245/directx-12-application-is-craching-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
	}
#endif

	//コマンドコンテキストの初期化
	commandContext_ = std::make_unique<CommandContext>();
	commandContext_->Initialize();

	//コマンドキューの生成
	commandQueue_ = std::make_unique<CommandQueue>();
	commandQueue_->Initialize();

	//DescriptorHeapの作成
	for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		descriptorHeaps_[i] = std::make_unique<DescriptorHeap>();
		bool shaderVisible = (i == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? true : false;
		descriptorHeaps_[i]->Initialize(D3D12_DESCRIPTOR_HEAP_TYPE(i), kNumDescriptors_[i], shaderVisible);
	}

	//ディスプレイの生成
	display_ = std::make_unique<Display>();
	display_->Initialize(dxgiFactory_.Get(), commandQueue_->GetCommandQueue());

	//深度バッファの生成
	depthBuffer_ = std::make_unique<DepthBuffer>();
	depthBuffer_->Create(Application::kClientWidth, Application::kClientHeight, DXGI_FORMAT_D24_UNORM_S8_UINT);

	//FrameRateControllerの初期化
	frameRateController_ = std::make_unique<FrameRateController>();
	frameRateController_->Initialize();

	//シェーダーコンパイラーの初期化
	ShaderCompiler::Initialize();
}

void GraphicsCore::PreDraw()
{
	//バックバッファを取得
	ColorBuffer& currentBackBuffer = display_->GetCurrentBuffer();

	//リソースの状態遷移
	commandContext_->TransitionResource(currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//レンダーターゲットとデプスバッファを設定
	commandContext_->SetRenderTargets(1, &currentBackBuffer.GetRTVHandle(), depthBuffer_->GetDSVHandle());

	//レンダーターゲットをクリア
	ClearRenderTarget();

	//デプスバッファをクリア
	ClearDepthBuffer();

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = Application::kClientWidth;
	viewport.Height = Application::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//ビューポートを設定
	commandContext_->SetViewport(viewport);

	//シザー矩形
	D3D12_RECT scissorRect{};
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = Application::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = Application::kClientHeight;
	//シザーを設定
	commandContext_->SetScissor(scissorRect);
}

void GraphicsCore::PostDraw()
{
	//バックバッファを取得
	ColorBuffer& currentBackBuffer = display_->GetCurrentBuffer();

	//リソースの状態遷移
	commandContext_->TransitionResource(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT);

	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	commandContext_->Close();

	//GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandContext_->GetCommandList() };
	commandQueue_->ExecuteCommandList(commandLists);

	//GPUとOSに画面の交換を行うよう通知する
	display_->Present();

	//GPUの処理が完了するのを待つ
	commandQueue_->WaitForFence();

	//FPS固定
	frameRateController_->Update();

	//次のフレーム用のコマンドリストを準備
	commandContext_->Reset();
}

void GraphicsCore::ClearRenderTarget()
{
	//バックバッファを取得
	ColorBuffer currentBackBuffer = display_->GetCurrentBuffer();

	//指定した色で画面全体をクリア
	commandContext_->ClearColor(currentBackBuffer);
}

void GraphicsCore::ClearDepthBuffer()
{
	//深度値を書き込めるようにする
	commandContext_->TransitionResource(*depthBuffer_, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	//デプスバッファをクリア
	commandContext_->ClearDepth(*depthBuffer_);
}

DescriptorHandle GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	return descriptorHeaps_[type]->Allocate();
}