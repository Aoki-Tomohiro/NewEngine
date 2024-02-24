#pragma once
#include "Engine/Framework/Object/IGameObject.h"
#include "Engine/Components/Particle/ParticleManager.h"
#include "Engine/Components/Collision/Collider.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Components/Audio/Audio.h"
#include "Weapon.h"
#include "Engine/2D/Sprite.h"
#include <array>
#include <optional>

class LockOn;

class Player : public IGameObject, public Collider
{
public:
	static const uint32_t kInvincibleTime = 120;

	//プレイヤーの状態
	enum class Behavior
	{
		kRoot,//通常状態
		kDash,//ダッシュ状態
		kAttack,//攻撃状態
		kAirAttack,//空中攻撃
		kJump,//ジャンプ中
		kKnockBack,//ノックバック
		kRapidApproach,//急接近
	};

	//ダッシュ用ワーク
	struct WorkDash
	{
		//ダッシュ用の媒介変数
		int dashParameter = 0;
		uint32_t coolTime = 0;
		const uint32_t dashCoolTime = 30;
	};

	//攻撃用ワーク
	struct WorkAttack
	{
		//攻撃ギミックの媒介変数
		Vector3 translation{};
		Vector3 rotation{};
		uint32_t attackParameter = 0;
		int32_t comboIndex = 0;
		int32_t inComboPhase = 0;
		bool comboNext = false;
	};

	//コンボの数
	static const int ComboNum = 4;
	static const int airComboNum = 4;

	//攻撃用定数
	struct ConstAttack
	{
		//アニメーション開始座標
		Vector3 startPosition{};
		//アニメーション開始角度
		Vector3 startRotation{};
		//振りかぶりの時間
		uint32_t anticipationTime;
		//ための時間
		uint32_t chargeTime;
		//攻撃振りの時間
		uint32_t swingTime;
		//硬直時間
		uint32_t recoveryTime;
		//振りかぶりの移動速さ
		Vector3 anticipationSpeed;
		Vector3 anticipationRotateSpeed;
		//ための移動速さ
		Vector3 chargeSpeed;
		Vector3 chargeRotateSpeed;
		//攻撃降りの移動速さ
		Vector3 swingSpeed;
		Vector3 swingRotateSpeed;
		//移動速度
		Vector3 velocity;
	};

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camemra"></param>
	void Draw(const Camera& camemra) override;

	/// <summary>
	/// UIの描画
	/// </summary>
	void DrawUI() override;

	/// <summary>
	/// 衝突判定
	/// </summary>
	void OnCollision(Collider* collider) override;

	/// <summary>
	/// ワールド座標を取得
	/// </summary>
	/// <returns></returns>
	const Vector3 GetWorldPosition() const override;

	/// <summary>
	/// ワールド変換データを取得
	/// </summary>
	/// <returns></returns>
	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

	/// <summary>
	/// カメラ設定
	/// </summary>
	/// <param name="camera"></param>
	void SetCamera(const Camera* camera) { camera_ = camera; };

	/// <summary>
	/// 武器を取得
	/// </summary>
	/// <returns></returns>
	Weapon* GetWeapon() { return weapon_; };

	/// <summary>
	/// 現在のコンボのインデックスを取得
	/// </summary>
	/// <returns></returns>
	const int32_t GetComboIndex() const { return workAttack_.comboIndex; };

	/// <summary>
	/// 速度を取得
	/// </summary>
	/// <returns></returns>
	const Vector3& GetVelocity() const { return velocity_; };

	/// <summary>
	/// 攻撃パラメーターを取得
	/// </summary>
	/// <returns></returns>
	const uint32_t GetAttackParameter() const { return workAttack_.attackParameter; };

	/// <summary>
	/// 攻撃のトータル時間を取得
	/// </summary>
	/// <returns></returns>
	const uint32_t GetAttackTime() const;

	/// <summary>
	/// ロックオンを設定
	/// </summary>
	/// <param name="lockOn"></param>
	void SetLockOn(const LockOn* lockOn) { lockOn_ = lockOn; }

	/// <summary>
	/// HPを取得
	/// </summary>
	/// <returns></returns>
	const float GetHP() const { return hp_; };

	/// <summary>
	/// ダメージを取得
	/// </summary>
	/// <returns></returns>
	const float GetDamage() const { return damage_; };

	/// <summary>
	/// ヒットフラグを取得
	/// </summary>
	/// <returns></returns>
	const bool GetIsHit() const { return isHit_; };

private:
	/// <summary>
	/// 通常行動初期化
	/// </summary>
	void BehaviorRootInitialize();

	/// <summary>
	/// 通常行動更新
	/// </summary>
	void BehaviorRootUpdate();

	/// <summary>
	/// ダッシュ行動初期化
	/// </summary>
	void BehaviorDashInitialize();

	/// <summary>
	/// ダッシュ行動更新
	/// </summary>
	void BehaviorDashUpdate();

	/// <summary>
	/// ジャンプ行動初期化
	/// </summary>
	void BehaviorJumpInitialize();

	/// <summary>
	/// ジャンプ行動更新
	/// </summary>
	void BehaviorJumpUpdate();

	/// <summary>
	/// 攻撃行動初期化
	/// </summary>
	void BehaviorAttackInitialize();

	/// <summary>
	/// 攻撃行動更新
	/// </summary>
	void BehaviorAttackUpdate();

	/// <summary>
	/// 空中攻撃初期化
	/// </summary>
	void BehaviorAirAttackInitialize();

	/// <summary>
	/// 空中攻撃更新
	/// </summary>
	void BehaviorAirAttackUpdate();

	/// <summary>
	/// ノックバック行動の初期化
	/// </summary>
	void BehaviorKnockBackInitialize();

	/// <summary>
	/// ノックバック行動の更新
	/// </summary>
	void BehaviorKnockBackUpdate();

	/// <summary>
	/// 急接近行動の初期化
	/// </summary>
	void BehaviorRapidApproachInitialize();

	/// <summary>
	/// 急接近行動の更新
	/// </summary>
	void BehaviorRapidApproachUpdate();

	/// <summary>
	/// 回転
	/// </summary>
	void Rotate(const Vector3& v);

	/// <summary>
	/// 攻撃モーション
	/// </summary>
	void AttackAnimation(bool isMove);

	/// <summary>
	/// 空中攻撃モーション
	/// </summary>
	/// <param name="isMove"></param>
	void AirAttackAnimation(bool isMove);

private:
	//入力クラス
	Input* input_ = nullptr;

	//オーディオ
	Audio* audio_ = nullptr;

	//振る舞い
	Behavior behavior_ = Behavior::kRoot;

	//次のふるまいのリクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	//カメラ
	const Camera* camera_ = nullptr;

	//速度
	Vector3 velocity_{};

	//クォータニオン
	Quaternion destinationQuaternion_{ 0.0f,0.0f,0.0f,1.0f };

	//ダッシュ用の変数
	WorkDash workDash_{};

	//コンボ定数表
	static std::array<ConstAttack, ComboNum> kConstAttacks_;
	static std::array<ConstAttack, airComboNum> kConstAirAttacks_;

	//攻撃用の変数
	WorkAttack workAttack_{};
	bool isAttack_ = false;
	bool isAirAttack_ = false;

	//ノックバック時の速度
	Vector3 knockBackVelocity_{};

	//武器
	std::unique_ptr<Model> modelWeapon_ = nullptr;
	Weapon* weapon_ = nullptr;

	int attackIndex_ = 0;

	Vector3 targetPosition_{ 0.0f,0.0f,0.0f };
	Vector3 rapidApproachVelocity_{ 0.0f,0.0f,0.0f };

	//オーディオハンドル
	uint32_t swishAudioHandle_ = 0;
	bool isSwishPlayed_ = false;

	//落下速度
	float fallingSpeed_ = 0.0f;

	//ロックオン
	const LockOn* lockOn_ = nullptr;

	//体力バー
	std::unique_ptr<Sprite> spriteHpBar_ = nullptr;
	std::unique_ptr<Sprite> spriteHpBarFrame_ = nullptr;

	//HP
	Vector2 hpBarSize_{ 480.0f,16.0f };
	const float kMaxHP = 30.0f;
	float hp_ = kMaxHP;

	//無敵時間
	bool invincibleFlag_ = false;
	int invincibleTimer_ = 0;

	//ダメージ
	float damage_ = 0.0f;

	//ヒットフラグ
	bool isHit_ = false;

	//パーティクル
	ParticleSystem* particleSystem_ = nullptr;
	std::unique_ptr<Model> particleModel_ = nullptr;
	bool isParticleActive_ = false;

	//オーディオハンドル
	uint32_t damageAudioHandle_ = 0;
	uint32_t dashAudioHandle_ = 0;
	uint32_t jumpAudioHandle_ = 0;
};

