#include "ParticleEmitterBuilder.h"

ParticleEmitterBuilder::ParticleEmitterBuilder()
{
	particleEmitter_ = new ParticleEmitter();
}

ParticleEmitterBuilder::~ParticleEmitterBuilder()
{

}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetEmitterName(const std::string& name)
{
	particleEmitter_->name_ = name;
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetTranslation(const Vector3& translation)
{
	particleEmitter_->translation_ = translation;
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopArea(const Vector3& min, const Vector3& max)
{
	particleEmitter_->popArea_ = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopRotation(const Vector3& min, const Vector3& max)
{
	particleEmitter_->popRotation_ = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopQuaternion(const Quaternion& quaternion)
{
	particleEmitter_->popQuaternion_ = quaternion;
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopScale(const Vector3& min, const Vector3& max)
{
	particleEmitter_->popScale_ = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopAzimuth(float min, float max)
{
	particleEmitter_->popAzimuth = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopElevation(float min, float max)
{
	particleEmitter_->popElevation = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopVelocity(const Vector3& min, const Vector3& max)
{
	particleEmitter_->popVelocity_ = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopColor(const Vector4& min, const Vector4& max)
{
	particleEmitter_->popColor_ = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopLifeTime(float min, float max)
{
	particleEmitter_->popLifeTime_ = { min,max };
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopCount(uint32_t count)
{
	particleEmitter_->popCount_ = count;

	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetPopFrequency(float frequency)
{
	particleEmitter_->popFrequency_ = frequency;
	particleEmitter_->frequencyTime_ = frequency;
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetDeleteTime(float deleteTime)
{
	particleEmitter_->deleteTime_ = deleteTime;
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetAccelerationField(const AccelerationField& accelerationField)
{
	particleEmitter_->accelerationField_ = accelerationField;
	return *this;
}

ParticleEmitterBuilder& ParticleEmitterBuilder::SetGravityField(const GravityField& gravityField)
{
	particleEmitter_->gravityField_ = gravityField;
	return *this;
}

ParticleEmitter* ParticleEmitterBuilder::Build()
{
	return particleEmitter_;
}