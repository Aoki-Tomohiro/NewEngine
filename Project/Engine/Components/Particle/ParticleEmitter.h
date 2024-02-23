#pragma once
#include "Engine/Math/AABB.h"
#include "Particle.h"
#include <string>
#include <memory>
#include <list>

struct AccelerationField
{
	Vector3 acceleration;//加速度
	AABB area;//範囲
	bool isEnable;//フラグ
};

struct GravityField
{
	Vector3 center;//中心点
	AABB area;//範囲
	float strength;//重力の強さ
	float stopDistance;//動きを止める距離
	bool isEnable;//フラグ
};

class ParticleEmitter
{
public:
	struct MinMaxStructVector4
	{
		Vector4 min;
		Vector4 max;
	};

	struct MinMaxStructVector3
	{
		Vector3 min;
		Vector3 max;
	};

	struct MinMaxStructFloat
	{
		float min;
		float max;
	};

	void Update();

	std::list<std::unique_ptr<Particle>>& GetParticles() { return particles_; };

	const std::string& GetName() const { return name_; };

	void SetName(const std::string& name) { name_ = name; };

	const bool GetIsDead() const { return isActive_; };

	void SetIsDead() { isActive_ = true; };

	const float GetDeleteTime() const { return deleteTime_; };

	void SetDeleteTime(float deleteTime) { deleteTime_ = deleteTime; };

	const Vector3& GetTranslation() const { return translation_; };

	void SetTranslation(const Vector3& translation) { translation_ = translation; };

	const Vector3& GetMinPopArea() const { return popArea_.min; };

	const Vector3& GetMaxPopArea() const { return popArea_.max; };

	void SetPopArea(const Vector3& min, const Vector3& max) { popArea_ = { min,max }; };

	const Vector3& GetMinPopRotation() const { return popRotation_.min; };

	const Vector3& GetMaxPopRotation() const { return popRotation_.max; };

	void SetPopRotation(const Vector3& min, const Vector3& max) { popRotation_ = { min,max }; };

	const Quaternion& GetPopQuaternion() const { return popQuaternion_; };

	void SetPopQuaternion(const Quaternion& quaternion) { popQuaternion_ = quaternion; };

	const Vector3& GetMinPopScale() const { return popScale_.min; };

	const Vector3& GetMaxPopScale() const { return popScale_.max; };

	void SetPopScale(const Vector3& min, const Vector3& max) { popScale_ = { min,max }; };

	const float GetMinPopAzimuth() const { return popAzimuth.min; };

	const float GetMaxPopAzimuth() const { return popAzimuth.max; };

	void SetPopAzimuth(float min, float max) { popAzimuth = { min,max }; };

	const float GetMinPopElevation() const { return popElevation.min; };

	const float GetMaxPopElevation() const { return popElevation.max; };

	void SetPopElevation(float min, float max) { popElevation = { min,max }; };

	const Vector3& GetMinPopVelocity() const { return popArea_.min; };

	const Vector3& GetMaxPopVelocity() const { return popArea_.max; };

	void SetPopVelocity(const Vector3& min, const Vector3& max) { popArea_ = { min,max }; };

	const Vector4& GetMinPopColor() const { return popColor_.min; };

	const Vector4& GetMaxPopColor() const { return popColor_.max; };

	void SetPopColor(const Vector4& min, const Vector4& max) { popColor_ = { min,max }; };

	const float GetMinPopLifeTime() const { return popLifeTime_.min; };

	const float GetMaxPopLifeTime() const { return popLifeTime_.max; };

	void SetPopLifeTime(float min, float max) { popLifeTime_ = { min,max }; };

	const uint32_t GetPopCount() const { return popCount_; };

	void SetPopCount(uint32_t count) { popCount_ = count; };

	const float GetPopFrequency() const { return popFrequency_; };

	void SetPopFrequency(float frequency) { popFrequency_ = frequency; };

	const AccelerationField& GetAccelerationField() const { return accelerationField_; };

	void SetAccelerationField(const AccelerationField& accelerationField) { accelerationField_ = accelerationField; };

	const GravityField& GetGravityField() const { return gravityField_; };

	void SetGravityField(const GravityField& gravityField) { gravityField_ = gravityField; };

private:
	void Pop();

	bool IsCollision(const AABB& aabb, const Vector3& point);

private:
	std::list<std::unique_ptr<Particle>> particles_{};

	std::string name_ = "nameless";

	MinMaxStructVector3 popArea_ = { {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	MinMaxStructVector3 popRotation_ = { {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	MinMaxStructVector3 popScale_ = { {1.0f,1.0f,1.0f},{1.0f,1.0f,1.0f} };

	MinMaxStructFloat popAzimuth = { 0.0f,360.0f };

	MinMaxStructFloat popElevation = { 0.0f,180.0f };

	MinMaxStructVector3 popVelocity_ = { {1.0f,1.0f,1.0f},{1.0f,1.0f,1.0f} };

	MinMaxStructVector4 popColor_ = { {1.0f,1.0f,1.0f,1.0f},{1.0f,1.0f,1.0f,1.0f} };

	MinMaxStructFloat popLifeTime_ = { 0.5f,1.0f };

	Quaternion popQuaternion_ = { 0.0f,0.0f,0.0f,1.0f };

	Vector3 translation_ = { 0.0f,0.0f,0.0f };

	uint32_t popCount_ = 1;

	float popFrequency_ = 0.1f;

	float frequencyTime_ = popFrequency_;

	float deleteTime_ = 10.0f;

	float deleteTimer_ = 0.0f;

	bool isActive_ = false;

	bool spawnFinished_ = false;

	AccelerationField accelerationField_{};

	GravityField gravityField_{};

	friend class ParticleEmitterBuilder;
};

