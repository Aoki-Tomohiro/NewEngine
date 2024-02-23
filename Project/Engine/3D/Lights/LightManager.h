#pragma once
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include <array>
#include <memory>

class LightManager
{
public:
	//DirectionalLightの数
	static const int kNumDirectionalLight = 1;
	//PointLightの数
	static const int kNumPointLight = 1;
	//SpotLightの数
	static const int kNumSpotLight = 1;
	//定数バッファ用構造体
	struct ConstBuffDataLight
	{
		std::array<ConstBuffDataDirectionalLight, kNumDirectionalLight> directionalLights{};
		std::array<ConstBuffDataPointLight, kNumPointLight> pointLights{};
		std::array<ConstBuffDataSpotLight, kNumSpotLight> spotLights{};
	};

	static LightManager* GetInstance();

	static void Destroy();

	void Initialize();

	void Update();

	UploadBuffer* GetConstantBuffer() const { return constBuff_.get(); };

	DirectionalLight& GetDirectionalLight(uint32_t index) { return directionalLights_[index]; };

	PointLight& GetPointLight(uint32_t index) { return pointLights_[index]; };

	SpotLight& GetSpotLight(uint32_t index) { return spotLights_[index]; };

private:
	LightManager() = default;
	~LightManager() = default;
	LightManager(const LightManager&) = delete;
	LightManager& operator=(const LightManager&) = delete;

private:
	static LightManager* instance_;

	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

	std::array<DirectionalLight, kNumDirectionalLight> directionalLights_{};

	std::array<PointLight, kNumPointLight> pointLights_{};

	std::array<SpotLight, kNumSpotLight> spotLights_{};
};

