#pragma once
#include "Bloom.h"
#include "DepthOfField.h"
#include "Fog.h"
#include "LensDistortion.h"
#include "Vignette.h"

class PostEffects
{
public:
	static PostEffects* GetInstance();

	static void Destroy();

	void Initialize();

	void Update();

	void Draw();

	void Apply();

	const bool GetIsEnable() const { return isEnable_; };

	void SetIsEnable(const bool isEnable) { isEnable_ = isEnable; };

	DepthOfField* GetDepthOfField() const { return depthOfField_.get(); };

	Bloom* GetBloom() const { return bloom_.get(); };

	Fog* GetFog() const { return fog_.get(); };

	LensDistortion* GetLensDistortion() const { return lensDistortion_.get(); };

	Vignette* GetVignette() const { return vignette_.get(); };

private:
	PostEffects() = default;
	~PostEffects() = default;
	PostEffects(const PostEffects&) = delete;
	PostEffects& operator=(const PostEffects&) = delete;

	void CreateVertexBuffer();

	void CreatePipelineState();

private:
	static PostEffects* instance_;

	std::unique_ptr<UploadBuffer> vertexBuffer_ = nullptr;

	std::array<VertexDataPosUV, 6> vertices_{};

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	RootSignature rootSignature_{};

	PipelineState pipelineState_{};

	std::unique_ptr<DepthOfField> depthOfField_ = nullptr;

	std::unique_ptr<Bloom> bloom_ = nullptr;

	std::unique_ptr<Fog> fog_ = nullptr;

	std::unique_ptr<LensDistortion> lensDistortion_ = nullptr;

	std::unique_ptr<Vignette> vignette_ = nullptr;

	bool isEnable_ = false;
};

