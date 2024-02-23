#pragma once
#include "Engine/Base/PipelineState.h"
#include "Engine/Base/ColorBuffer.h"
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"
#include <array>
#include <memory>

class GaussianBlur
{
public:
	enum BlurDirection
	{
		kHorizontal,
		kVertical,
		kCountOfBlurDirection
	};

	static void StaticInitialize();

	static void Destroy();

	void Initialize(uint32_t width, uint32_t height);

	void Update();

	void PreBlur(uint32_t index);

	void PostBlur(uint32_t index);

	const uint32_t GetTextureWidth() const { return textureWidth_; };

	void SetTextureWidth(const uint32_t textureWidth) { textureWidth_ = textureWidth; };

	const uint32_t GetTextureHeight() const { return textureHeight_; };

	void SetTextureHeight(const uint32_t textureHeight) { textureHeight_ = textureHeight; };

	const float GetSigma() const { return sigma_; };

	void SetSigma(const float sigma) { sigma_ = sigma; };

	const UploadBuffer* GetConstantBuffer() const { return constBuff_.get(); };

	const DescriptorHandle& GetDescriptorHandle(uint32_t index) const { return blurBuffers_[index]->GetSRVHandle(); };

private:
	static std::unique_ptr<RootSignature> rootSignature_;

	static std::array<std::unique_ptr<PipelineState>, kCountOfBlurDirection> pipelineStates_;

	std::array<std::unique_ptr<ColorBuffer>, kCountOfBlurDirection> blurBuffers_{};

	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

	uint32_t textureWidth_ = 0;

	uint32_t textureHeight_ = 0;

	float sigma_ = 5.0f;
};

