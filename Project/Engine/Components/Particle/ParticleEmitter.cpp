#include "ParticleEmitter.h"
#include "Engine/Utilities/RandomGenerator.h"
#include "Engine/Math/MathFunction.h"
#include <numbers>

void ParticleEmitter::Update()
{
	//パーティクルを生成
	const float kDeltaTime = 1.0f / 60.0f;
	frequencyTime_ += kDeltaTime;
	if (popFrequency_ <= frequencyTime_)
	{
		if (!spawnFinished_)
		{
			for (uint32_t index = 0; index < popCount_; ++index)
			{
				Pop();
			}
			frequencyTime_ -= popFrequency_;
		}
	}

	//パーティクルの処理
	for (std::list<std::unique_ptr<Particle>>::iterator particleIterator = particles_.begin(); particleIterator != particles_.end();)
	{
		//死亡フラグが立ったパーティクルを削除
		if (particleIterator->get()->IsDead())
		{
			particleIterator = particles_.erase(particleIterator);
		}
		else
		{
			//加速フィールドの判定
			if (accelerationField_.isEnable)
			{
				if (IsCollision(accelerationField_.area, particleIterator->get()->GetTranslation()))
				{
					Vector3 velocity = particleIterator->get()->GetVelocity();
					particleIterator->get()->SetVelocity(velocity + accelerationField_.acceleration);
				}
			}

			//重力フィールドの判定
			if (gravityField_.isEnable)
			{
				if (IsCollision(gravityField_.area, particleIterator->get()->GetTranslation()))
				{
					//距離を計算
					Vector3 sub = gravityField_.center - particleIterator->get()->GetTranslation();
					float distance = Mathf::Length(sub);

					//引力を計算
					Vector3 gravityForce = Mathf::Normalize(sub) * gravityField_.strength;

					//中心に近づいたら速度を0にする
					if (distance < gravityField_.stopDistance)
					{
						particleIterator->get()->SetVelocity({ 0.0f,0.0f,0.0f });
					}
					else
					{
						//現在の速度に引力を足す
						Vector3 currentVelocity = particleIterator->get()->GetVelocity();
						particleIterator->get()->SetVelocity(currentVelocity + gravityForce);
					}
				}
			}

			//パーティクルの更新
			particleIterator->get()->Update();

			//イテレータを進める
			++particleIterator;
		}
	}

	//エミッターの死亡フラグを立てる
	deleteTimer_ += kDeltaTime;
	if (deleteTimer_ > deleteTime_)
	{
		spawnFinished_ = true;
		if (particles_.size() == 0)
		{
			isActive_ = false;
		}
	}
}

void ParticleEmitter::Pop()
{
	//座標
	Vector3 translation = {
		translation_.x + RandomGenerator::GetRandomFloat(popArea_.min.x,popArea_.max.x),
		translation_.y + RandomGenerator::GetRandomFloat(popArea_.min.y,popArea_.max.y),
		translation_.z + RandomGenerator::GetRandomFloat(popArea_.min.z,popArea_.max.z)
	};

	//回転
	Vector3 rotation = {
		RandomGenerator::GetRandomFloat(popRotation_.min.x,popRotation_.max.x),
		RandomGenerator::GetRandomFloat(popRotation_.min.y,popRotation_.max.y),
		RandomGenerator::GetRandomFloat(popRotation_.min.z,popRotation_.max.z)
	};

	//スケール
	Vector3 scale = {
		RandomGenerator::GetRandomFloat(popScale_.min.x,popScale_.max.x),
		RandomGenerator::GetRandomFloat(popScale_.min.y,popScale_.max.y),
		RandomGenerator::GetRandomFloat(popScale_.min.z,popScale_.max.z)
	};

	//方位角
	float azimuth = { RandomGenerator::GetRandomFloat(popAzimuth.min,popAzimuth.max) };
	float azimuthRadian = azimuth * float(std::numbers::pi / 180.0f);

	//仰角
	float elevation = { RandomGenerator::GetRandomFloat(popElevation.min,popElevation.max) };
	float elevationRadian = elevation * float(std::numbers::pi / 180.0f);

	//速度
	Vector3 velocity;
	if (azimuth != 0.0f || elevation != 0.0f)
	{
		velocity = {
			RandomGenerator::GetRandomFloat(popVelocity_.min.x,popVelocity_.max.x) * std::cos(elevationRadian) * std::cos(azimuthRadian),
			RandomGenerator::GetRandomFloat(popVelocity_.min.y,popVelocity_.max.y) * std::cos(elevationRadian) * std::sin(azimuthRadian),
			RandomGenerator::GetRandomFloat(popVelocity_.min.z,popVelocity_.max.z) * std::sin(elevationRadian)
		};
	}
	else
	{
		velocity = {
			RandomGenerator::GetRandomFloat(popVelocity_.min.x,popVelocity_.max.x),
			RandomGenerator::GetRandomFloat(popVelocity_.min.y,popVelocity_.max.y),
			RandomGenerator::GetRandomFloat(popVelocity_.min.z,popVelocity_.max.z),
		};
	}

	//色
	Vector4 color = {
		RandomGenerator::GetRandomFloat(popColor_.min.x,popColor_.max.x),
		RandomGenerator::GetRandomFloat(popColor_.min.y,popColor_.max.y),
		RandomGenerator::GetRandomFloat(popColor_.min.z,popColor_.max.z),
		RandomGenerator::GetRandomFloat(popColor_.min.w,popColor_.max.w)
	};

	//寿命
	float lifeTime = RandomGenerator::GetRandomFloat(popLifeTime_.min, popLifeTime_.max);

	//パーティクルの生成
	Particle* particle = new Particle();
	particle->Initialize(translation, rotation, popQuaternion_, scale, velocity, color, lifeTime);
	particles_.push_back(std::unique_ptr<Particle>(particle));
}

bool ParticleEmitter::IsCollision(const AABB& aabb, const Vector3& point)
{
	if (aabb.min.x <= point.x && aabb.max.x >= point.x &&
		aabb.min.y <= point.y && aabb.max.y >= point.y &&
		aabb.min.z <= point.z && aabb.max.z >= point.z)
	{
		return true;
	}

	return false;
}