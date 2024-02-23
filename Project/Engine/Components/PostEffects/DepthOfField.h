#pragma once
#include "Engine/Base/PipelineState.h"
#include "Engine/Base/ColorBuffer.h"
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"

class DepthOfField
{
public:
	void Initialize();

	void Update();

	void Apply(const DescriptorHandle& srvHandle);

	const bool GetIsEnable() const { return isEnable_; };

	void SetIsEnable(const bool isEnable) { isEnable_ = isEnable; };

	const float GetFocusDepth() const { return focusDepth_; };

	void SetFocusDepth(const float focusDepth) { focusDepth_ = focusDepth; };

	const float GetNFocusWidth() const { return nFocusWidth_; };

	void SetNFocusWidth(const float nFocusWidth) { nFocusWidth_ = nFocusWidth; };

	const float GetFFocusWidth() const { return fFocusWidth_; };

	void SetFFocusWidth(const float fFocusWidth) { fFocusWidth_ = fFocusWidth; };

	const DescriptorHandle& GetDescriptorHandle() const { return colorBuffer_->GetSRVHandle(); };

private:
	void CreatePipelineState();

private:
	//RootSignature
	RootSignature rootSignature_{};

	//PipelineState
	PipelineState pipelineState_{};

	//ColorBuffer
	std::unique_ptr<ColorBuffer> colorBuffer_ = nullptr;

	//ConstBuffer
	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

	//調整項目
	int32_t isEnable_ = false;

	float focusDepth_ = 0.02f;

	float nFocusWidth_ = 0.005f;

	float fFocusWidth_ = 0.01f;
};

