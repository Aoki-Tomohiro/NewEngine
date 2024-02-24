#pragma once
#include "BaseCharacter.h"
#include "../Weapon/Weapon.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Components/Collision/Collider.h"
#include <optional>

class LockOn;

/// <summary>
/// プレイヤー
/// </summary>
class Player : public BaseCharacter, public Collider {
public:
	//プレイヤーの状態
	enum class Behavior {
		kRoot,//通常状態
		kDash,//ダッシュ状態
		kAttack,//攻撃状態
		kJump,//ジャンプ中
	};

	//ダッシュ用ワーク
	struct WorkDash {
		//ダッシュ用の媒介変数
		int dashParameter_ = 0;
		uint32_t coolTime = 0;
	};

	//攻撃用ワーク
	struct WorkAttack {
		//攻撃ギミックの媒介変数
		Vector3 translation{};
		Vector3 rotation{};
		uint32_t attackParameter = 0;
		int32_t comboIndex = 0;
		int32_t inComboPhase = 0;
		bool comboNext = false;
		bool isAttack = false;
	};

	//コンボの数
	static const int ComboNum = 3;

	//攻撃用定数
	struct ConstAttack {
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

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model"></param>
	void Initialize(const std::vector<Model*>& models) override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camera"></param>
	void Draw(const Camera& camera) override;

	/// <summary>
	/// パーティクルの描画
	/// </summary>
	/// <param name="camera"></param>
	void DrawParticle(const Camera& camera);

	/// <summary>
	/// リスタート処理
	/// </summary>
	void Restart();

	/// <summary>
	/// カメラ設定
	/// </summary>
	/// <param name="camera"></param>
	void SetCamera(const Camera* camera) { camera_ = camera; };

	/// <summary>
	/// 親を設定
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(const WorldTransform* parent) { parent_ = parent; };

	/// <summary>
	/// 武器を取得
	/// </summary>
	/// <returns></returns>
	Weapon* GetWeapon() { return weapon_.get(); };

	/// <summary>
	/// ロックオンを設定
	/// </summary>
	/// <param name="lockOn"></param>
	void SetLockOn(const LockOn* lockOn) { lockOn_ = lockOn; };

	/// <summary>
	/// 攻撃中かどうか
	/// </summary>
	/// <returns></returns>
	bool GetIsAttack() { return workAttack_.isAttack; };

	/// <summary>
	/// 死亡フラグを取得
	/// </summary>
	/// <returns></returns>
	bool GetIsDead() { return isDead_; };

	/// <summary>
	/// 死亡フラグを設定
	/// </summary>
	/// <param name="isDead"></param>
	void SetIsDead(bool isDead) { isDead_ = isDead; };

	/// <summary>
	/// ワールド変換データを取得
	/// </summary>
	/// <returns></returns>
	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name="collider"></param>
	void OnCollision(Collider* collider) override;

	/// <summary>
	/// ワールドポジションを取得
	/// </summary>
	/// <returns></returns>
	const Vector3 GetWorldPosition() const override;

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
	/// 攻撃行動初期化
	/// </summary>
	void BehaviorAttackInitialize();

	/// <summary>
	/// 攻撃行動更新
	/// </summary>
	void BehaviorAttackUpdate();

	/// <summary>
	/// ジャンプ行動初期化
	/// </summary>
	void BehaviorJumpInitialize();

	/// <summary>
	/// ジャンプ行動更新
	/// </summary>
	void BehaviorJumpUpdate();

	/// <summary>
	/// プレイヤーの向きを変える
	/// </summary>
	void Rotate(const Vector3& vector);

	/// <summary>
	/// グローバル変数の適応
	/// </summary>
	void ApplyGlobalVariables();

private:
	//入力クラス
	Input* input_ = nullptr;
	//振る舞い
	Behavior behavior_ = Behavior::kRoot;
	//次のふるまいのリクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//ダッシュ用の変数
	WorkDash workDash_{};
	//目標角度
	float destinationAngleY_ = 0.0f;
	//当たり判定のフラグ
	bool onCollision_ = false;
	bool preOnCollision_ = false;
	//ビュープロジェクション
	const Camera* camera_ = nullptr;
	//親のポインタ
	const WorldTransform* parent_ = nullptr;
	//武器
	std::unique_ptr<Weapon> weapon_ = nullptr;
	//ダッシュ時間
	int behaviorDashTime_ = 10;
	//クォータニオン
	Quaternion destinationQuaternion_{ 0.0f,0.0f,0.0f,1.0f };
	//速度
	Vector3 velocity_{};
	//コンボ定数表
	static const std::array<ConstAttack, ComboNum> kConstAttacks_;
	//攻撃用の変数
	WorkAttack workAttack_{};
	//ロックオン
	const LockOn* lockOn_ = nullptr;
	//死亡フラグ
	bool isDead_ = false;
	//落下スピード
	float fallingSpeed_ = 0.0f;
};