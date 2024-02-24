#pragma once
#include "BaseCharacter.h"
#include "../Weapon/Weapon.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Components/Collision/Collider.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include <optional>

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
	};

	//ダッシュ用ワーク
	struct WorkDash {
		//ダッシュ用の媒介変数
		int dashParameter_ = 0;
		uint32_t coolTime = 0;
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
	Quaternion moveQuaternion_{ 0.0f,0.0f,0.0f,1.0f };
};