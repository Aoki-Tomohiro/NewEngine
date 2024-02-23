#pragma once
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Quaternion.h"

class Particle
{
public:
	void Initialize(const Vector3& translation, const Vector3& rotation, const Quaternion& quaternion, const Vector3& scale, const Vector3& velocity, const Vector4& color, float lifeTime);

	void Update();

	const Vector3& GetTranslation() { return translation_; };

	void SetTranslation(const Vector3& translation) { translation_ = translation; };

	const Vector3& GetRotation() { return rotation_; };

	void SetRotation(const Vector3& rotation) { rotation_ = rotation; };

	const Quaternion& GetQuaternion() const { return quaternion_; };

	void SetQuaternion(const Quaternion& quaternion) { quaternion_ = quaternion; };

	const Vector3& GetScale() { return scale_; };

	void SetScale(const Vector3& scale) { scale_ = scale; };

	const Vector3& GetVelocity() { return velocity_; };

	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; };

	const Vector4& GetColor() { return color_; };

	void SetColor(const Vector4& color) { color_ = color; };

	const bool IsDead() const { return isDead_; };

	void SetIsDead(const bool isDead) { isDead_ = isDead; };

private:
	Vector3 translation_{};

	Vector3 rotation_{};

	Vector3 scale_{};

	Vector3 velocity_{};

	Vector4 color_{};

	Quaternion quaternion_{ 0.0f,0.0f,0.0f,1.0f };

	float lifeTime_ = 0.0f;

	float currentTime_ = 0.0f;

	bool isDead_ = false;

	float alpha_ = 0.0f;
};

