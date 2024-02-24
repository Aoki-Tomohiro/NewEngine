#pragma once
#include "Engine/Components/Collision/Collider.h"
#include "Engine/3D/Model/ModelManager.h"

class Missile : public Collider
{
public:
	static const uint32_t kTrackingTime = 60 * 2;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const Vector3& position, const Vector3& velocity);

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

	//速度
	Vector3 velocity_{};

	//媒介変数
	float t_ = 0.0f;

	//追尾
	bool isTrackingComplete_ = false;

	//追尾タイマー
	uint32_t trackingTimer_ = 0;

	//死亡フラグ
	bool isDead_ = false;

	//オーディオハンドル
	uint32_t audioHandle_ = 0;
};

