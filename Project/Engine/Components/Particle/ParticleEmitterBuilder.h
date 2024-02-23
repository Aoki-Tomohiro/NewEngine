#pragma once
#include "ParticleEmitter.h"

class ParticleEmitterBuilder
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	ParticleEmitterBuilder();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ParticleEmitterBuilder();

	/// <summary>
	/// エミッターの名前を設定
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetEmitterName(const std::string& name);

	/// <summary>
	/// エミッターの座標を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetTranslation(const Vector3& translation);

	/// <summary>
	/// パーティクルの発生範囲を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopArea(const Vector3& min, const Vector3& max);

	/// <summary>
	/// パーティクルの角度を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopRotation(const Vector3& min, const Vector3& max);

	/// <summary>
	/// パーティクルのQuaternionを設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopQuaternion(const Quaternion& quaternion);

	/// <summary>
	/// パーティクルのスケールを設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopScale(const Vector3& min, const Vector3& max);

	/// <summary>
	/// パーティクルの方位角を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopAzimuth(float min, float max);

	/// <summary>
	/// パーティクルの仰角を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopElevation(float min, float max);

	/// <summary>
	/// パーティクルの初速度を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopVelocity(const Vector3& min, const Vector3& max);

	/// <summary>
	/// パーティクルの色を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopColor(const Vector4& min, const Vector4& max);

	/// <summary>
	/// パーティクルの寿命を設定
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopLifeTime(float min, float max);

	/// <summary>
	/// 一度に発生する数
	/// </summary>
	/// <param name="count"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopCount(uint32_t count);

	/// <summary>
	/// 発生頻度
	/// </summary>
	/// <param name="frequency"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetPopFrequency(float frequency);

	/// <summary>
	/// エミッターの寿命を設定
	/// </summary>
	/// <param name="frequency"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetDeleteTime(float deleteTime);

	/// <summary>
	/// 加速フィールドを設定
	/// </summary>
	/// <param name="acceleration"></param>
	/// <param name="area"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetAccelerationField(const AccelerationField& accelerationField);

	/// <summary>
	/// 重力フィールドを設定
	/// </summary>
	/// <param name="acceleration"></param>
	/// <param name="area"></param>
	/// <returns></returns>
	ParticleEmitterBuilder& SetGravityField(const GravityField& gravityField);

	/// <summary>
	/// エミッターを作成
	/// </summary>
	/// <returns></returns>
	ParticleEmitter* Build();

private:
	//エミッター
	ParticleEmitter* particleEmitter_ = nullptr;
};

