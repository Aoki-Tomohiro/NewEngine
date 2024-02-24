#include "BossStateTackle.h"
#include "Application/Src/Object/Boss/Boss.h"

void BossStateTackle::Initialize(Boss* pBoss)
{
	worldTransform_ = pBoss->GetWorldTransform();
	destinationQuaternion_ = worldTransform_.quaternion_;
	pBoss->SetDamage(10.0f);

	//目標座標を設定
	Vector3 offset{ 0.0f,0.0f,6.0f };
	offset = Mathf::TransformNormal(offset, worldTransform_.matWorld_);
	targetPosition_ = worldTransform_.translation_ + offset;
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

	//警告モデルの作成
	waringModel_.reset(ModelManager::CreateFromOBJ("Warning", Opaque));
	waringModel_->SetEnableLighting(false);
	waringModel_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

	//警告用のワールドトランスフォームの初期化
	warningWorldTransform_.Initialize();
	warningWorldTransform_.translation_ = (targetPosition_ + worldTransform_.translation_) / 2.0f;
	warningWorldTransform_.translation_.y = 0.1f;
	warningWorldTransform_.quaternion_ = worldTransform_.quaternion_;
	warningWorldTransform_.scale_ = { worldTransform_.scale_.x,worldTransform_.scale_.y,6.0f + 3.0f + 3.0f };
}

void BossStateTackle::Update(Boss* pBoss)
{
	//攻撃待機
	if (!isAttack_)
	{
		if (++waitTimer_ > kWaitTime)
		{
			isAttack_ = true;
			pBoss->SetIsAttack(true);
		}
	}

	//攻撃処理
	if (isAttack_ && !isRecovery_)
	{
		//目標座標に攻撃
		worldTransform_.translation_ = Mathf::Lerp(worldTransform_.translation_, targetPosition_, 0.2f);

		//大体同じ座標になったら攻撃終了
		const float epsilon = 0.001f;
		Vector3 abs = {
			std::abs(worldTransform_.translation_.x - targetPosition_.x),
			std::abs(worldTransform_.translation_.y - targetPosition_.y),
			std::abs(worldTransform_.translation_.z - targetPosition_.z),
		};
		if (abs.x < epsilon && abs.y < epsilon && abs.z < epsilon)
		{
			isRecovery_ = true;
			pBoss->SetIsAttack(false);
		}
	}

	//硬直処理
	if (isRecovery_)
	{
		//硬直状態が終わったら通常状態に戻す
		if (++recoveryTimer_ > kRecoveryTime)
		{
			IBossState* newState = new BossStateNormal();
			newState->Initialize(pBoss);
			pBoss->ChangeState(newState);
			return;
		}
	}

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

void BossStateTackle::Draw(Boss* pBoss, const Camera& camera)
{
	if (!isAttack_)
	{
		waringModel_->Draw(warningWorldTransform_, camera);
	}
}

void BossStateTackle::OnCollision(Collider* collider)
{

}