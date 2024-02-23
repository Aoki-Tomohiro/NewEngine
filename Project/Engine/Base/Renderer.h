#pragma once
#include "Engine/3D/Lights/LightManager.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "PipelineState.h"
#include <vector>

enum DrawPass
{
	Opaque,
	Transparent,
	NumTypes,
};

enum BlendMode 
{
	//ブレンドなし
	kBlendModeNone,
	//通常aブレンド
	kBlendModeNormal,
	//加算
	kBlendModeAdd,
	//減算
	kBlendModeSubtract,
	//乗算
	kBlendModeMultiply,
	//スクリーン
	kBlendModeScreen,
	//利用してはいけない
	kCountOfBlendMode,
};

class Renderer
{
public:
	enum RootBindings 
	{
		//マテリアル
		kMaterial,
		//ワールドトランスフォーム
		kWorldTransform,
		//カメラ
		kCamera,
		//テクスチャ
		kTexture,
		//ライト
		kDirectionalLight,
	};

	static Renderer* GetInstance();

	static void Destroy();

	void Initialize();

	void AddObject(D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
		D3D12_GPU_VIRTUAL_ADDRESS materialCBV,
		D3D12_GPU_VIRTUAL_ADDRESS worldTransformCBV,
		D3D12_GPU_VIRTUAL_ADDRESS cameraCBV,
		D3D12_GPU_DESCRIPTOR_HANDLE textureSRV,
		UINT vertexCount,
		DrawPass drawPass);

	void Render();

	void ClearRenderTarget();

	void ClearDepthBuffer();

	void PreDraw();

	void PostDraw();

	void PreDrawSprites(BlendMode blendMode);

	void PostDrawSprites();

	void PreDrawParticles();

	void PostDrawParticles();

	const DescriptorHandle& GetSceneColorDescriptorHandle() const { return sceneColorBuffer_->GetSRVHandle(); };

	const DescriptorHandle& GetLinearDepthDescriptorHandle() const { return linearDepthColorBuffer_->GetSRVHandle(); };

private:
	Renderer() = default;
	~Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void CreateModelPipelineState();

	void CreateSpritePipelineState();

	void CreateParticlePipelineState();

	void Sort();

private:
	struct SortObject {
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_GPU_VIRTUAL_ADDRESS materialCBV;
		D3D12_GPU_VIRTUAL_ADDRESS worldTransformCBV;
		D3D12_GPU_VIRTUAL_ADDRESS cameraCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE textureSRV;
		UINT vertexCount;
		DrawPass type;
	};

	static Renderer* instance_;

	std::vector<SortObject> sortObjects_{};

	std::unique_ptr<ColorBuffer> sceneColorBuffer_ = nullptr;

	std::unique_ptr<ColorBuffer> linearDepthColorBuffer_ = nullptr;

	std::unique_ptr<DepthBuffer> sceneDepthBuffer_ = nullptr;

	LightManager* lightManager_ = nullptr;

	RootSignature modelRootSignature_{};

	RootSignature spriteRootSignature_{};

	RootSignature particleRootSignature_{};

	std::vector<PipelineState> modelPipelineStates_{};

	std::vector<PipelineState> spritePipelineStates_{};

	std::vector<PipelineState> particlePipelineStates_{};
};

