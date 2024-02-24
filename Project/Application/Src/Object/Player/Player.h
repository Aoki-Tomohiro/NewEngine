#pragma once
#include "Engine/Framework/Object/IGameObject.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Base/ImGuiManager.h"
#include "Engine/Components/Collision/Collider.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/Components/Particle/ParticleManager.h"
#include "Engine/2D/Sprite.h"
#include "Engine/Math/MathFunction.h"
#include "Application/Src/Object/Player/Weapon.h"
#include <optional>

class LockOn;

class Player : public IGameObject, public Collider
{
public:
	void Initialize() override;

	void Update() override;

	void Draw(const Camera& camemra) override;

	void DrawUI() override;

	void OnCollision(Collider* collider) override;

	const Vector3 GetWorldPosition() const override;

	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

	void SetModels(const std::vector<Model*> models) { models_ = models; };

	void SetCamera(const Camera* camera) { camera_ = camera; };

	void SetLockOn(const LockOn* lockOn) { lockOn_ = lockOn; }

	Weapon* GetWeapon() { return weapon_; };

	const Vector3& GetVelocity() const { return velocity_; };

	const int32_t GetComboIndex() const { return workAttack_.comboIndex; };

	const uint32_t GetAttackParameter() const { return workAttack_.attackParameter; };

	const uint32_t GetAttackTotalTime() const;

	const float GetDamage() const { return workAttack_.damage; };

	const float GetHP() const { return hp_; };

	const bool GetIsHit() const { return isHit_; };

private:
	void BehaviorRootInitialize();

	void BehaviorRootUpdate();

	void MoveAnimation();

	void BehaviorDashInitialize();

	void BehaviorDashUpdate();

	void BackStepAnimation();

	void BehaviorJumpInitialize();

	void BehaviorJumpUpdate();

	void BehaviorAttackInitialize();

	void BehaviorAttackUpdate();

	void AttackAnimation();

	void BehaviorKnockBackInitialize();

	void BehaviorKnockBackUpdate();

	void Move(const float speed);

	void Rotate(const Vector3& v);

private:
	enum Parts
	{
		kBody,
		kHead,
		kL_Arm,
		kR_Arm,
		kCountOfParts
	};

	//プレイヤーの状態
	enum class Behavior
	{
		kRoot,//通常状態
		kDash,//ダッシュ状態
		kJump,//ジャンプ中
		kAttack,//攻撃状態
		kKnockBack,//ノックバック
	};

	//移動アニメーション用ワーク
	struct MoveAnimationWork
	{
		float parameter_ = 0.0f;
		float startRotation_ = 0.0f;
		float endRotation_ = 1.0f;
		bool rotationChange_ = false;
	};

	//ダッシュ用ワーク
	struct WorkDash
	{
		//ダッシュ用の媒介変数
		int dashParameter = 0;
		uint32_t coolTime = 0;
		const uint32_t dashCoolTime = 30;
		bool backStep = false;
		float backStepRotation = 0.0f;
	};

	//攻撃用ワーク
	struct WorkAttack
	{
		uint32_t attackParameter = 0;
		uint32_t collisionParameter = 0;
		int32_t comboIndex = 0;
		int32_t inComboPhase = 0;
		float inComboPhaseAttackParameter = 0;
		bool comboNext = false;
		bool isAttack = false;
		float damage = 0.0f;
	};

	//無敵状態用ワーク
	struct WorkInvincible
	{
		const uint32_t kInvincibleTime = 60;
		uint32_t invincibleTimer = 0;
		bool invincibleFlag = false;
	};

	//コンボの数
	static const int ComboNum = 4;

	//攻撃フェーズ
	enum AttackPhase
	{
		kAnticipation,
		kCharge,
		kSwing,
		kRecovery,
	};

	//攻撃用定数
	struct ConstAttack
	{
		//振りかぶりの時間
		uint32_t anticipationTime;
		//ための時間
		uint32_t chargeTime;
		//攻撃振りの時間
		uint32_t swingTime;
		//硬直時間
		uint32_t recoveryTime;
		//振りかぶりの移動速さ
		float anticipationSpeed;
		//ための移動速さ
		float chargeSpeed;
		//攻撃降りの移動速さ
		float swingSpeed;
	};

	//コンボ定数表
	static const std::array<ConstAttack, ComboNum> kConstAttacks_;

private:
	//モデル
	std::vector<Model*> models_{};

	//ワールドトランスフォーム
	WorldTransform worldTransforms[kCountOfParts]{};

	//入力クラス
	Input* input_ = nullptr;

	//オーディオ
	Audio* audio_ = nullptr;

	//振る舞い
	Behavior behavior_ = Behavior::kRoot;

	//次のふるまいのリクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	//速度
	Vector3 velocity_{};

	//ノックバック時の速度
	Vector3 knockBackVelocity_{};

	//重力
	Vector3 gravity_{};

	//クォータニオン
	Quaternion destinationQuaternion_{ 0.0f,0.0f,0.0f,1.0f };

	//移動アニメーション用のパラメーター
	MoveAnimationWork moveAnimationWork_{};

	//ダッシュ用ワーク
	WorkDash workDash_{};

	//攻撃用ワーク
	WorkAttack workAttack_{};

	//無敵状態用ワーク
	WorkInvincible workInvincible_{};

	//地面に触れているか
	bool isGroundHit_ = true;

	//ダメージを食らったか
	bool isHit_ = false;

	//カメラ
	const Camera* camera_ = nullptr;

	//ロックオン
	const LockOn* lockOn_ = nullptr;

	//体力バー
	std::unique_ptr<Sprite> spriteHpBar_ = nullptr;
	std::unique_ptr<Sprite> spriteHpBarFrame_ = nullptr;

	//ダメージエフェクトのスプライト
	std::unique_ptr<Sprite> damageSprite_ = nullptr;
	Vector4 damageSpriteColor_ = { 1.0f,0.0f,0.0f,0.0f };

	//HP
	Vector2 hpBarSize_{ 480.0f,16.0f };
	const float kMaxHP = 40.0f;
	float hp_ = kMaxHP;

	//パーティクル
	std::unique_ptr<Model> particleModel_ = nullptr;
	ParticleSystem* particleSystem_ = nullptr;

	//武器
	std::unique_ptr<Model> modelWeapon_ = nullptr;
	Weapon* weapon_ = nullptr;

	//オーディオハンドル
	uint32_t swishAudioHandle_ = 0;
	uint32_t damageAudioHandle_ = 0;
	uint32_t dashAudioHandle_ = 0;
	uint32_t jumpAudioHandle_ = 0;
};

