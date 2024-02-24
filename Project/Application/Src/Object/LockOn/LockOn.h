#pragma once
#include "Engine/Components/Input/Input.h"
#include "Engine/Base/Application.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/3D/Camera/Camera.h"
#include "Engine/2D/Sprite.h"
#include "Engine/Math/MathFunction.h"
#include "Application/Src/Object/Boss/Boss.h"

class LockOn
{
public:
	void Initialize();

	void Update(const Boss* boss, const Camera& camera);

	void Draw();

	Vector3 GetTargetPosition() const;

	bool ExistTarget() const { return target_ ? true : false; };

private:
	bool InRange(const Camera& camera);

	void SearchLockOnTarget(const Boss* boss, const Camera& camera);

private:
	Input* input_ = nullptr;

	std::unique_ptr<Sprite> lockOnMark_ = nullptr;

	const Boss* target_ = nullptr;

	float minDistance_ = 10.0f;

	float maxDistance_ = 80.0f;

	const float kDegreeToRadian = 3.14159265358979323846f / 180.0f;
	float angleRange_ = 20.0f * kDegreeToRadian;
};

