#pragma once
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

class DirectionalLight
{
public:
	const Vector4& GetColor() const { return color_; };

	void SetColor(const Vector4& color) { color_ = color; };

	const Vector3& GetDirection() const { return direction_; };

	void SetDirection(const Vector3& direction) { direction_ = direction; };

	const float GetIntensity() const { return intensity_; };

	void SetIntensity(const float intensity) { intensity_ = intensity; };

	const bool GetIsEnable() const { return isEnable_; };

	void SetIsEnable(const bool isEnable) { isEnable_ = isEnable; };

private:
	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	Vector3 direction_ = { 0.0f,-1.0f,0.0f };

	float intensity_ = 1.0f;

	bool isEnable_ = true;
};

