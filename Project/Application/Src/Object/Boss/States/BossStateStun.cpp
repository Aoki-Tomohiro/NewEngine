#include "BossStateStun.h"
#include "Application/Src/Object/Boss/Boss.h"

void BossStateStun::Initialize(Boss* pBoss)
{
	worldTransform_ = pBoss->GetWorldTransform();
	destinationQuaternion_ = worldTransform_.quaternion_;
}

void BossStateStun::Update(Boss* pBoss)
{
	if (++stunTimer_ > kStunTime)
	{
		IBossState* newState = new BossStateNormal();
		newState->Initialize(pBoss);
		pBoss->ChangeState(newState);
	}

	//移動限界座標
	const float kMoveLimitX = 47;
	const float kMoveLimitZ = 47;
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.z = max(worldTransform_.translation_.z, -kMoveLimitZ);
	worldTransform_.translation_.z = min(worldTransform_.translation_.z, +kMoveLimitZ);
}

void BossStateStun::Draw(Boss* pBoss, const Camera& camera)
{

}

void BossStateStun::OnCollision(Collider* collider)
{
	if (collider->GetCollisionAttribute() == kCollisionAttributeWeapon)
	{

	}
}