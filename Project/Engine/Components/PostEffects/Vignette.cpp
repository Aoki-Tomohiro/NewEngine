#include "Vignette.h"

void Vignette::Initialize()
{
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataVignette));
	Update();
}

void Vignette::Update()
{
	ConstBuffDataVignette* vignetteData = static_cast<ConstBuffDataVignette*>(constBuff_->Map());
	vignetteData->isEnable = isEnable_;
	vignetteData->intensity = intensity_;
	constBuff_->Unmap();
}