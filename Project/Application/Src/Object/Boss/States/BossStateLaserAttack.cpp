#include "BossStateLaserAttack.h"
#include "Application/Src/Object/Boss/Boss.h"
#include <numbers>

void BossStateLaserAttack::Initialize(Boss* pBoss)
{
	worldTransform_ = pBoss->GetWorldTransform();
	destinationQuaternion_ = Mathf::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, std::numbers::pi_v<float> / 2.0f);

	//警告モデルの作成
	waringModel_.reset(ModelManager::CreateFromOBJ("Warning", Opaque));
	waringModel_->SetEnableLighting(false);
	waringModel_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

	//警告用のワールドトランスフォームの初期化
	warningWorldTransform_.Initialize();
	warningWorldTransform_.translation_ = targetPosition_;
	warningWorldTransform_.translation_.y = 0.1f;
	warningWorldTransform_.quaternion_ = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	warningWorldTransform_.scale_ = { 1.5f,1.5f,50.0f };

	//音声データ読み込み
	audioHandle_ = Audio::GetInstance()->SoundLoadWave("Application/Resources/Sounds/Charge.wav");

	//パーティクルの生成
	chargeParticle_ = ParticleManager::Create("Charge");
	chargeParticle_->SetTexture("DefaultParticle.png");
}

void BossStateLaserAttack::Update(Boss* pBoss)
{
	//移動処理
	worldTransform_.translation_ = Mathf::Lerp(worldTransform_.translation_, targetPosition_, 0.1f);

	const float epsilon = 0.001f;
	Vector3 abs = {
		std::abs(worldTransform_.translation_.x - targetPosition_.x),
		std::abs(worldTransform_.translation_.y - targetPosition_.y),
		std::abs(worldTransform_.translation_.z - targetPosition_.z),
	};
	//目標位置についたらレーザーをためる
	if (abs.x < epsilon && abs.y < epsilon && abs.z < epsilon)
	{
		if (!isCharge_ && !isAttack_)
		{
			Vector3 translation = pBoss->GetWorldPosition();
			GravityField gravityField = { translation,  { {-100.0f,-100.0f,-100.0f},{100.0f,100.0f,100.0f} } ,0.006f, 0.2f,true };
			ParticleEmitter* emitter = ParticleEmitterBuilder()
				.SetDeleteTime(kChargeTime / 80)
				.SetEmitterName("Charge")
				.SetGravityField(gravityField)
				.SetPopArea({ worldTransform_.translation_ - worldTransform_.scale_ * 2.0f }, { worldTransform_.translation_ + worldTransform_.scale_ * 2.0f })
				.SetPopAzimuth(0.0f, 0.0f)
				.SetPopColor({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f })
				.SetPopCount(10)
				.SetPopFrequency(0.01f)
				.SetPopElevation(0.0f, 0.0f)
				.SetPopLifeTime(1.0f, 1.0f)
				.SetPopRotation({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
				.SetPopScale({ 0.1f,0.1f,0.1f }, { 0.2f,0.2f,0.2f })
				.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
				.SetTranslation(translation)
				.Build();
			chargeParticle_->AddParticleEmitter(emitter);
		}

		isCharge_ = true;

		//ため処理
		if (!isAttack_)
		{
			//音声再生
			Audio::GetInstance()->SoundPlayWave(audioHandle_, false, 0.1f);

			if (++chargeTimer_ > kChargeTime)
			{
				//攻撃フラグをtrueにする
				isAttack_ = true;
				isCharge_ = false;
				chargeParticle_->GetParticleEmitter("Charge")->SetPopCount(0);

				//レーザーを追加
				Laser* newLaser = new Laser();
				newLaser->Initialize();
				pBoss->AddLaser(newLaser);
			}

			if (chargeTimer_ > kChargeTime - 60)
			{
				Audio::GetInstance()->StopAudio(audioHandle_);
			}
		}

		//攻撃処理
		if (isAttack_)
		{
			if (!isRecovery_)
			{
				//回転処理
				Quaternion rotateQuaternion = Mathf::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, 0.02f);
				destinationQuaternion_ *= rotateQuaternion;

				if (++attackTimer_ > Laser::kLifeTime)
				{
					isRecovery_ = true;
				}
			}
		}

		//硬直処理
		if (isRecovery_)
		{
			if (++recoveryTimer_ > kRecoveryTime)
			{
				IBossState* newState = new BossStateNormal();
				newState->Initialize(pBoss);
				pBoss->ChangeState(newState);
				return;
			}
		}
	}

	//回転処理
	worldTransform_.quaternion_ = Mathf::Slerp(worldTransform_.quaternion_, destinationQuaternion_, 0.4f);

	//移動限界座標
	const float kMoveLimitX = 47;
	const float kMoveLimitZ = 47;
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.z = max(worldTransform_.translation_.z, -kMoveLimitZ);
	worldTransform_.translation_.z = min(worldTransform_.translation_.z, +kMoveLimitZ);

	//警告用のワールドトランスフォームの更新
	warningWorldTransform_.UpdateMatrixFromQuaternion();
}

void BossStateLaserAttack::Draw(Boss* pBoss, const Camera& camera)
{
	if (isCharge_ && !isAttack_)
	{
		waringModel_->Draw(warningWorldTransform_, camera);
	}
}

void BossStateLaserAttack::OnCollision(Collider* collider)
{
	if (collider->GetCollisionAttribute() == kCollisionAttributeWeapon)
	{

	}
}