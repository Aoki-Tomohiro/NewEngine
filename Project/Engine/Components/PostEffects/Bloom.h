#pragma once
#include "GaussianBlur.h"

class Bloom
{
public:
	static const int kMaxBlurCount = 4;

	void Initialize();

	void Update();

	void Apply(const DescriptorHandle& srvHandle);

	const uint32_t GetBlurCount() const { return blurCount_; };

	void SetBlurCount(const uint32_t blurCount) { blurCount_ = blurCount; };

	const bool GetIsEnable() const { return isEnable_; };

	void SetIsEnable(const bool isEnable) { isEnable_ = isEnable; };

	const float GetIntensity() const { return intensity_; };

	void SetIntensity(const float intensity) { intensity_ = intensity; };

	const float GetTextureWeight() const { return textureWeight_; };

	void SetTextureWeight(const float textureWeight) { textureWeight_ = textureWeight; };

	const float GetHighLumTextureWeight() const { return highLumTextureWeight_; };

	void SetHighLumTextureWeight(const float highLumTextureWeight) { highLumTextureWeight_ = highLumTextureWeight; };

	const float GetBlurTextureWeight(const uint32_t index) const { return blurTextureWeight_[index]; };

	void SetBlurTextureWeight(const uint32_t index, const float blurTextureWeight) { blurTextureWeight_[index] = blurTextureWeight; };

	GaussianBlur* GetGaussianBlur(uint32_t index) const { return gaussianBlurs_[index].get(); };

	const DescriptorHandle& GetDescriptorHandle() const { return bloomColorBuffer_->GetSRVHandle(); };

private:
	void CreateHighLumPipelineState();

	void CreateBloomPipelineState();

	void RenderHighLuminance(const DescriptorHandle& srvHandle);

	void ApplyGaussianBlur();

private:
	//RootSignature
	RootSignature highLumRootSignature_{};
	RootSignature bloomRootSignature_{};

	//PipelineState
	PipelineState highLumPipelineState_{};
	PipelineState bloomPipelineState_{};

	//ColorBuffer
	std::unique_ptr<ColorBuffer> highLumColorBuffer_ = nullptr;
	std::unique_ptr<ColorBuffer> bloomColorBuffer_ = nullptr;

	//GaussianBlur
	std::array<std::unique_ptr<GaussianBlur>, kMaxBlurCount> gaussianBlurs_{};

	//ConstBuffer
	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

	//調整項目
	int32_t isEnable_ = false;

	float intensity_ = 1.0f;

	float textureWeight_ = 1.0f;

	float highLumTextureWeight_ = 1.0f;

	float blurTextureWeight_[4] = { 1.0f,1.0f,1.0f,1.0f };

	uint32_t blurCount_ = kMaxBlurCount;
};

