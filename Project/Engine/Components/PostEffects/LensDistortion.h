#pragma once
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"
#include <memory>

class LensDistortion
{
public:
	void Initialize();

	void Update();

	const UploadBuffer* GetConstBuffer() const { return constBuff_.get(); };

	const bool GetIsEnable() const { return isEnable_; };

	void SetIsEnable(const bool isEnable) { isEnable_ = isEnable; };

	const float GetTightness() const { return tightness_; };

	void SetTightness(const float tightness) { tightness_ = tightness; };

	const float GetStrength() const { return strength_; };

	void SetStrength(const float strength) { strength_ = strength; };

private:
	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

	bool isEnable_ = false;

	float tightness_ = 2.5f;

	float strength_ = -0.1f;
};

