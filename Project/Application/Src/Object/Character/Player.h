#pragma once
#include "BaseCharacter.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Components/Collision/Collider.h"

/// <summary>
/// プレイヤー
/// </summary>
class Player : public BaseCharacter,public Collider {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player() override = default;

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
	/// <param name="viewProjection"></param>
	void Draw(const Camera& camera) override;

	/// <summary>
	/// リスタート処理
	/// </summary>
	void Restart();

	/// <summary>
	/// カメラを設定
	/// </summary>
	/// <param name="viewProjection"></param>
	void SetCamera(const Camera* camera) { camera_ = camera; };

	/// <summary>
	/// 親を設定
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(const WorldTransform* parent) { parent_ = parent; };

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
	//ビュープロジェクション
	const Camera* camera_ = nullptr;
	//入力クラス
	Input* input_ = nullptr;
	//親
	const WorldTransform* parent_ = nullptr;
	//当たり判定のフラグ
	bool onCollision_ = false;
	bool preOnCollision_ = false;
};
