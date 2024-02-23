#pragma once
#include "Application.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include <array>
#include <memory>
#include <dxgi1_6.h>

class Display
{
public:
	void Initialize(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue);

	void Present();

	ColorBuffer& GetCurrentBuffer();

private:
	void CreateSwapChain(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue);

	void CreateResources();

private:
	Application* application_ = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

	std::array<std::unique_ptr<ColorBuffer>, 2> swapChainResources_;
};

