#include "Display.h"
#include <cassert>

void Display::Initialize(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue)
{
	//Applicationのインスタンスを取得
	application_ = Application::GetInstance();

	//スワップチェーンの作成
	CreateSwapChain(dxgiFactory, commandQueue);

	//リソースの作成
	CreateResources();
}

void Display::Present()
{
	//GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);
}

ColorBuffer& Display::GetCurrentBuffer()
{
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();
	return *swapChainResources_[backBufferIndex];
}

void Display::CreateSwapChain(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue)
{
	//スワップチェーンを作成する
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = application_->kClientWidth;//画面の幅。ウィンドウのクライアント領域と同じものにしておく
	swapChainDesc.Height = application_->kClientHeight;//画面の高さ。ウィンドウのクライアント領域と同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに映したら、中身を破棄
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, application_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void Display::CreateResources()
{
	for (uint32_t i = 0; i < 2; i++)
	{
		ID3D12Resource* resource = nullptr;
		HRESULT hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&resource));
		swapChainResources_[i] = std::make_unique<ColorBuffer>();
		swapChainResources_[i]->CreateFromSwapChain(resource);
	}
}