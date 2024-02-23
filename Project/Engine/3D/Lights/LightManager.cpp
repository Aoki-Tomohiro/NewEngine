#include "LightManager.h"
#include "Engine/Math/MathFunction.h"

//実態定義
LightManager* LightManager::instance_ = nullptr;

LightManager* LightManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new LightManager();
	}
	return instance_;
}

void LightManager::Destroy()
{
	if (instance_ != nullptr)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

void LightManager::Initialize()
{
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataLight));
	Update();
}

void LightManager::Update()
{
	ConstBuffDataLight* lightData = static_cast<ConstBuffDataLight*>(constBuff_->Map());
	for (uint32_t i = 0; i < kNumDirectionalLight; ++i)
	{
		lightData->directionalLights[i].color = directionalLights_[i].GetColor();
		lightData->directionalLights[i].direction = Mathf::Normalize(directionalLights_[i].GetDirection());
		lightData->directionalLights[i].intensity = directionalLights_[i].GetIntensity();
		lightData->directionalLights[i].isEnable = directionalLights_[i].GetIsEnable();
	}
	for (uint32_t i = 0; i < kNumPointLight; ++i)
	{
		lightData->pointLights[i].color = pointLights_[i].GetColor();
		lightData->pointLights[i].position = pointLights_[i].GetPosition();
		lightData->pointLights[i].intensity = pointLights_[i].GetIntensity();
		lightData->pointLights[i].radius = pointLights_[i].GetRadius();
		lightData->pointLights[i].decay = pointLights_[i].GetDecay();
		lightData->pointLights[i].isEnable = pointLights_[i].GetIsEnable();
	}
	for (uint32_t i = 0; i < kNumSpotLight; ++i)
	{
		lightData->spotLights[i].color = spotLights_[i].GetColor();
		lightData->spotLights[i].position = spotLights_[i].GetPosition();
		lightData->spotLights[i].intensity = spotLights_[i].GetIntensity();
		lightData->spotLights[i].direction = Mathf::Normalize(spotLights_[i].GetDirection());
		lightData->spotLights[i].distance = spotLights_[i].GetDistance();
		lightData->spotLights[i].decay = spotLights_[i].GetDecay();
		lightData->spotLights[i].cosFalloffStart = spotLights_[i].GetCosFalloffStart();
		lightData->spotLights[i].cosAngle = spotLights_[i].GetCosAngle();
		lightData->spotLights[i].isEnable = spotLights_[i].GetIsEnable();
	}
	constBuff_->Unmap();
}