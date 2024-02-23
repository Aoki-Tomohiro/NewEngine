#pragma once
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"

class PointLight
{
public:
	const Vector4& GetColor() const { return color_; };

	void SetColor(const Vector4& color) { color_ = color; };

	const Vector3& GetPosition() const { return position_; };

	void SetPosition(const Vector3& position) { position_ = position; };

	const float GetIntensity() const { return intensity_; };

	void SetIntensity(const float intensity) { intensity_ = intensity; };

	const float GetRadius() const { return radius_; };
	
	void SetRadius(const float radius) { radius_ = radius; };

	const float GetDecay() const { return decay_; };

	void SetDecay(const float decay) { decay_ = decay; };

	const bool GetIsEnable() const { return isEnable_; };

	void SetIsEnable(const bool isEnable) { isEnable_ = isEnable; };

private:
	Vector4 color_{ 1.0f,1.0f,1.0f,1.0f };

	Vector3 position_ = { 0.0f,2.0f,0.0f };

	float intensity_ = 1.0f;

	float radius_ = 10.0f;

	float decay_ = 1.0f;

	bool isEnable_ = false;
};

