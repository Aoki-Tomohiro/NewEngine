#pragma once
#include "Application.h"
#include "CommandContext.h"
#include "CommandQueue.h"
#include "Display.h"
#include "DescriptorHeap.h"
#include "FrameRateController.h"
#include <array>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

class GraphicsCore
{
public:
	static GraphicsCore* GetInstance();

	static void Destroy();

	void Initialize();

	void PreDraw();

	void PostDraw();

	void ClearRenderTarget();

	void ClearDepthBuffer();

	DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

	ID3D12DescriptorHeap* GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) const { return descriptorHeaps_[type]->GetDescriptorHeap(); }

	ID3D12Device* GetDevice() const { return device_.Get(); };

	CommandContext* GetCommandContext() const { return commandContext_.get(); };

	CommandQueue* GetCommandQueue() const { return commandQueue_.get(); };

private:
	GraphicsCore() = default;
	~GraphicsCore() = default;
	GraphicsCore(const GraphicsCore&) = delete;
	GraphicsCore& operator=(const GraphicsCore&) = delete;

private:
	static GraphicsCore* instance_;

	Application* app_ = nullptr;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;

	std::unique_ptr<CommandContext> commandContext_ = nullptr;

	std::unique_ptr<CommandQueue> commandQueue_ = nullptr;

	std::array<std::unique_ptr<DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> descriptorHeaps_{};

	std::array<const uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> kNumDescriptors_ = { 256, 256, 256, 256, };

	std::unique_ptr<Display> display_ = nullptr;

	std::unique_ptr<DepthBuffer> depthBuffer_ = nullptr;

	std::unique_ptr<FrameRateController> frameRateController_ = nullptr;
};

