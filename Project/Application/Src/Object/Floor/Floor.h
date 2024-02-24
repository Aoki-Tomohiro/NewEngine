#pragma once
#include "Engine/3D/Model/Model.h"
#include "Engine/3D/Model/WorldTransform.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Components/Collision/Collider.h"
#include "Engine/Components/Collision/CollisionConfig.h"

class Floor : public Collider {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model"></param>
	/// <param name="position"></param>
	/// <param name="velocity"></param>
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camera"></param>
	void Draw(const Camera& camera);

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

	/// <summary>
	/// ワールドトランスフォームを取得
	/// </summary>
	/// <returns></returns>
	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

private:
	//モデル
	Model* model_ = nullptr;
	//ワールドトランスフォーム
	WorldTransform worldTransform_{};
	//速度
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };

};

