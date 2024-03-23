#pragma once
#include "IBossState.h"

class BossStateStun : public IBossState
{
public:
	static const uint32_t kStunTime = 60 * 2;

	void Initialize(Boss* pBoss) override;

	void Update(Boss* pBoss) override;

	void Draw(Boss* pBoss, const Camera& camera) override;

	void OnCollision(Collider* collider) override;

	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

private:
	//ワールドトランスフォーム
	WorldTransform worldTransform_{};

	//クォータニオン
	Quaternion destinationQuaternion_{ 0.0f,0.0f,0.0f,1.0f };

	//スタンタイマー
	int stunTimer_ = 0;
};

