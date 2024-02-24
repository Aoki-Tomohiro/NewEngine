#pragma once
#include "IBossState.h"
#include "Engine/3D/Model/ModelManager.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/Components/Particle/ParticleManager.h"
#include "Engine/Math/MathFunction.h"

class BossStateLaserAttack : public IBossState
{
public:
	//チャージ時間
	static const uint32_t kChargeTime = 60 * 5;
	//硬直時間
	static const uint32_t kRecoveryTime = 60 * 2;

	void Initialize(Boss* pBoss) override;

	void Update(Boss* pBoss) override;

	void Draw(Boss* pBoss, const Camera& camera) override;

	void OnCollision(Collider* collider) override;

	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

private:
	//警告用モデル
	std::unique_ptr<Model> waringModel_ = nullptr;

	//ワールドトランスフォーム
	WorldTransform worldTransform_{};
	WorldTransform warningWorldTransform_{};

	//クォータニオン
	Quaternion destinationQuaternion_{ 0.0f,0.0f,0.0f,1.0f };

	//目標座標
	Vector3 targetPosition_ = { 0.0f,0.0f,0.0f };

	//チャージフラグ
	bool isCharge_ = false;

	//チャージタイマー
	uint32_t chargeTimer_ = 0;

	//攻撃フラグ
	bool isAttack_ = false;

	//攻撃タイマー
	uint32_t attackTimer_ = 0;

	//硬直フラグ
	bool isRecovery_ = false;

	//硬直タイマー
	uint32_t recoveryTimer_ = 0;

	//オーディオハンドル
	uint32_t audioHandle_ = 0;

	//パーティクル
	ParticleSystem* chargeParticle_ = nullptr;
};

