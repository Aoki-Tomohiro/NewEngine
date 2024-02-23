#include "LensDistortion.h"

void LensDistortion::Initialize()
{
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataLensDistortion));
	Update();
}

void LensDistortion::Update()
{
	strength_ = std::min<float>(-0.1f, strength_);
	strength_ = std::max<float>(0.1f, strength_);
	ConstBuffDataLensDistortion* lensDistortionData = static_cast<ConstBuffDataLensDistortion*>(constBuff_->Map());
	lensDistortionData->isEnable = isEnable_;
	lensDistortionData->tightness = tightness_;
	lensDistortionData->strength = strength_;
	constBuff_->Unmap();
}