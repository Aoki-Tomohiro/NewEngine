#pragma once
#include "IBossState.h"
#include "Engine/3D/Model/ModelManager.h"

class BossStateLaserAttack : public IBossState
{
public:
	static const uint32_t kChargeTime = 60 * 5;
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
	Vector3 targetPosition_ = { 0.0f,3.0f,0.0f };

	//レーザーのタイマー
	uint32_t chargeTimer_ = 0;
	uint32_t attackTimer_ = 0;

	//硬直時間
	uint32_t recoveryTimer_ = 0;

	//レーザー攻撃のフラグ
	bool isAttack_ = false;

	//硬直しているかのフラグ
	bool isRecovery_ = false;

	//チャージフラグ
	bool isCharge_ = false;

	//シェイクタイマー
	static const uint32_t kShakeTime = 10;
	uint32_t shakeTimer_ = 0;

	//オーディオハンドル
	uint32_t audioHandle_ = 0;

	//パーティクル
	ParticleSystem* particleSystem_ = nullptr;
};

