#pragma once
#include "Engine/Components/Collision/Collider.h"
#include "Engine/3D/Model/ModelManager.h"

class Laser : public Collider
{
public:
	static const uint32_t kLifeTime = 60 * 10;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

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
	/// 死亡フラグを取得
	/// </summary>
	/// <returns></returns>
	const bool GetIsDead() const { return isDead_; };

private:
	//モデル
	std::unique_ptr<Model> model_ = nullptr;

	//ワールドトランスフォーム
	WorldTransform worldTransform_{};

	//死亡フラグ
	bool isDead_ = false;

	//タイマー
	uint32_t lifeTimer_ = 0;

	//OBB
	OBB obbSize{};

	//目標スケール
	Vector3 targetScale_{ 2.0f,2.0f,70.0f };
};

