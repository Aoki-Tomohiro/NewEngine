#pragma once
#include "IBossState.h"
#include "Engine/3D/Model/ModelManager.h"

class BossStateCrashDown : public IBossState
{
public:
	static const uint32_t kWaitTime = 60;
	static const uint32_t kRecoveryTime = 60 * 2;

	void Initialize(Boss* pBoss) override;

	void Update(Boss* pBoss) override;

	void Draw(Boss* pBoss, const Camera& camera) override;

	void OnCollision(Collider* collider) override;

	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

private:
	//モデル
	std::unique_ptr<Model> waringModel_ = nullptr;

	//ワールドトランスフォーム
	WorldTransform worldTransform_{};
	WorldTransform warningWorldTransform_{};

	//クォータニオン
	Quaternion destinationQuaternion_{ 0.0f,0.0f,0.0f,1.0f };

	//目標座標
	Vector3 targetPosition_{};

	//待機タイマー
	uint32_t waitTimer_ = 0;

	//攻撃フラグ
	bool isAttack_ = false;

	//硬直時間
	uint32_t recoveryTimer_ = 0;

	//硬直フラグ
	bool isRecovery_ = false;
};

