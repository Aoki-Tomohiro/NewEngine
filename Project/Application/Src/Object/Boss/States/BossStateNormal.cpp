#include "BossStateNormal.h"
#include "Application/Src/Object/Boss/Boss.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Application/Src/Object/Player/Player.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Base/ImGuiManager.h"
#include "Engine/Utilities/RandomGenerator.h"

void BossStateNormal::Initialize(Boss* pBoss)
{
	worldTransform_ = pBoss->GetWorldTransform();
	destinationQuaternion_ = worldTransform_.quaternion_;
	attackTime_ = RandomGenerator::GetRandomInt(kMinAttackInterval, kMaxAttackInterval);
}

void BossStateNormal::Update(Boss* pBoss)
{
	//移動方向を計算
	Vector3 targetPosition = GameObjectManager::GetInstance()->GetGameObject<Player>("Player")->GetWorldPosition();
	Vector3 sub = targetPosition - worldTransform_.translation_;
	sub.y = 0.0f;

	//距離を計算
	float distance = Mathf::Length(sub);

	//正規化して移動量を掛ける
	sub = Mathf::Normalize(sub);
	const float kSpeed = 0.1f;

	//回転処理
	Vector3 cross = Mathf::Normalize(Mathf::Cross({ 0.0f,0.0f,1.0f }, sub));
	float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, sub);
	destinationQuaternion_ = Mathf::Normalize(Mathf::MakeRotateAxisAngleQuaternion(cross, std::acos(dot)));
	worldTransform_.quaternion_ = Mathf::Slerp(worldTransform_.quaternion_, destinationQuaternion_, 0.4f);

	//移動処理
	if (distance >= 10.0f)
	{
		worldTransform_.translation_ += sub * kSpeed;
	}

	//攻撃処理
	if (++attackTimer_ > attackTime_)
	{
		uint32_t attackNum = RandomGenerator::GetRandomInt(0, 3);
		IBossState* newState = nullptr;
		switch (attackNum)
		{
		case 0:
			newState = new BossStateTackle();
			newState->Initialize(pBoss);
			break;
		case 1:
			newState = new BossStateCrashDown();
			newState->Initialize(pBoss);
			break;
		case 2:
			newState = new BossStateMissileAttack();
			newState->Initialize(pBoss);
			break;
		case 3:
			newState = new BossStateLaserAttack();
			newState->Initialize(pBoss);
			break;
		}
		pBoss->ChangeState(newState);
	}

	//ノックバックの処理
	if (isKnockBack_)
	{
		if (++knockBackTimer_ >= knockBackTime_)
		{
			isKnockBack_ = false;
		}

		worldTransform_.translation_ += knockBackVelocity_;
	}

	//移動限界座標
	const float kMoveLimitX = 47;
	const float kMoveLimitZ = 47;
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.z = max(worldTransform_.translation_.z, -kMoveLimitZ);
	worldTransform_.translation_.z = min(worldTransform_.translation_.z, +kMoveLimitZ);

	ImGui::Begin("Boss");
	ImGui::DragFloat4("WorldQuaternion", &worldTransform_.quaternion_.x);
	ImGui::DragFloat4("Quaternion", &destinationQuaternion_.x);
	ImGui::DragFloat3("Sub", &sub.x);
	ImGui::End();
}

void BossStateNormal::Draw(Boss* pBoss, const Camera& camera)
{

}

void BossStateNormal::OnCollision(Collider* collider)
{
	if (collider->GetCollisionAttribute() == kCollisionAttributeWeapon)
	{
		//ノックバックの速度を決める
		const Player* player = GameObjectManager::GetInstance()->GetGameObject<Player>("Player");
		knockBackVelocity_ = player->GetVelocity();
		if (player->GetComboIndex() == 3)
		{
			Vector3 sub = worldTransform_.translation_ - player->GetWorldPosition();
			sub = Mathf::Normalize(sub);
			sub.y = 0.0f;
			const float kKnockBackSpeed = 2.0f;
			knockBackVelocity_ = sub * kKnockBackSpeed;
		}

		//ノックバックのフラグを立てる
		isKnockBack_ = true;

		//ノックバックのタイマーを設定
		knockBackTimer_ = 0;
		knockBackTime_ = player->GetAttackTime() - player->GetAttackParameter();
	}
}