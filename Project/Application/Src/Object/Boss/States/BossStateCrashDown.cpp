#include "BossStateCrashDown.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Application/Src/Object/Boss/Boss.h"
#include "Application/Src/Object/Player/Player.h"

void BossStateCrashDown::Initialize(Boss* pBoss)
{
	worldTransform_ = pBoss->GetWorldTransform();
	destinationQuaternion_ = worldTransform_.quaternion_;
	pBoss->SetDamage(10.0f);

	//目標座標の設定
	targetPosition_ = GameObjectManager::GetInstance()->GetGameObject<Player>("Player")->GetWorldPosition();
	targetPosition_.y = 8.0f;
	if (targetPosition_.x >= 47.0f)
	{
		targetPosition_.x = 47.0f;
	}
	else if (targetPosition_.x <= -47.0f)
	{
		targetPosition_.x = -47.0f;
	}

	if (targetPosition_.z >= 47.0f)
	{
		targetPosition_.z = 47.0f;
	}
	else if (targetPosition_.z <= -47.0f)
	{
		targetPosition_.z = -47.0f;
	}

	//回転処理
	Vector3 sub = targetPosition_ - worldTransform_.translation_;
	sub.y = 0.0f;
	sub = Mathf::Normalize(sub);
	Vector3 cross = Mathf::Normalize(Mathf::Cross({ 0.0f,0.0f,1.0f }, sub));
	float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, sub);
	destinationQuaternion_ = Mathf::Normalize(Mathf::MakeRotateAxisAngleQuaternion(cross, std::acos(dot)));

	//警告モデルの作成
	waringModel_.reset(ModelManager::CreateFromOBJ("Warning", Opaque));
	waringModel_->SetEnableLighting(false);
	waringModel_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

	//警告用のワールドトランスフォームの初期化
	warningWorldTransform_.Initialize();
	warningWorldTransform_.translation_ = targetPosition_;
	warningWorldTransform_.translation_.y = 0.11f;
	warningWorldTransform_.quaternion_ = worldTransform_.quaternion_;
	warningWorldTransform_.scale_ = { worldTransform_.scale_.x,worldTransform_.scale_.y,worldTransform_.scale_.z };
}

void BossStateCrashDown::Update(Boss* pBoss)
{
	//攻撃待機
	if (!isAttack_)
	{
		//目標の上に移動する
		worldTransform_.translation_ = Mathf::Lerp(worldTransform_.translation_, targetPosition_, 0.2f);

		const float epsilon = 0.001f;
		Vector3 abs = {
			std::abs(worldTransform_.translation_.x - targetPosition_.x),
			std::abs(worldTransform_.translation_.y - targetPosition_.y),
			std::abs(worldTransform_.translation_.z - targetPosition_.z),
		};

		//大体同じ座標になったら攻撃終了
		if (abs.x < epsilon && abs.y < epsilon && abs.z < epsilon)
		{
			if (++waitTimer_ > kWaitTime)
			{
				isAttack_ = true;
				pBoss->SetIsAttack(true);
			}
		}
	}

	//攻撃処理
	if (isAttack_ && !isRecovery_)
	{
		//攻撃処理
		const float kAttackSpeed = 2.0f;
		worldTransform_.translation_.y -= kAttackSpeed;

		//地面についたら攻撃終了
		if (worldTransform_.translation_.y <= 0.0f)
		{
			worldTransform_.translation_.y = 0.0f;
			isRecovery_ = true;
			pBoss->SetIsAttack(false);
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

void BossStateCrashDown::Draw(Boss* pBoss, const Camera& camera)
{
	if (!isAttack_)
	{
		waringModel_->Draw(warningWorldTransform_, camera);
	}
}

void BossStateCrashDown::OnCollision(Collider* collider)
{

}