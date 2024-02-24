#pragma once
#include "IBossState.h"

class BossStateMissileAttack : public IBossState
{
public:
	static const uint32_t kWaitTime = 60;
	static const uint32_t kFireInterval = 60 / 2;
	static const uint32_t kRecoveryTime = 60 * 2;

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

	//待機タイマー
	uint32_t waitTimer_ = 0;

	//ミサイルの発射タイマー
	uint32_t fireTimer_ = 0;

	//硬直時間
	uint32_t recoveryTimer_ = 0;

	//ミサイル攻撃のフラグ
	bool isAttack_ = false;

	//硬直フラグ
	bool isRecovery_ = false;

	//発射カウント
	uint32_t fireCount_ = 0;

	//シェイクタイマー
	static const uint32_t kShakeTime = 10;
	uint32_t shakeTimer_ = 0;

	//オーディオハンドル
	uint32_t audioHandle_ = 0;
};

