#include "Laser.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Engine/Math/MathFunction.h"

void Laser::Initialize()
{
	//モデルの生成
	model_.reset(ModelManager::CreateFromOBJ("Laser", Opaque));
	model_->SetEnableLighting(false);

	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_.y = 3.0f;
	worldTransform_.scale_ = { 0.01f,0.01f,70.0f };

	//衝突属性を設定
	obbSize = {
	.center{GetWorldPosition()},
	.orientations{
		{worldTransform_.matWorld_.m[0][0],worldTransform_.matWorld_.m[0][1],worldTransform_.matWorld_.m[0][2]},
		{worldTransform_.matWorld_.m[1][0],worldTransform_.matWorld_.m[1][1],worldTransform_.matWorld_.m[1][2]},
		{worldTransform_.matWorld_.m[2][0],worldTransform_.matWorld_.m[2][1],worldTransform_.matWorld_.m[2][2]},},
	.size{worldTransform_.scale_}
	};
	SetOBB(obbSize);
	SetCollisionAttribute(kCollisionAttributeLaser);
	SetCollisionMask(kCollisionMaskLaser);
	SetCollisionPrimitive(kCollisionPrimitiveOBB);
}

void Laser::Update()
{
	//ワールドトランスフォームの更新
	const float kRotSpeed = 0.02f;
	worldTransform_.rotation_.y += kRotSpeed;
	worldTransform_.UpdateMatrixFromEuler();

	//OBBのサイズを変更
	obbSize = {
		.center{worldTransform_.translation_},
		.orientations{
			{worldTransform_.matWorld_.m[0][0],worldTransform_.matWorld_.m[0][1],worldTransform_.matWorld_.m[0][2]},
			{worldTransform_.matWorld_.m[1][0],worldTransform_.matWorld_.m[1][1],worldTransform_.matWorld_.m[1][2]},
			{worldTransform_.matWorld_.m[2][0],worldTransform_.matWorld_.m[2][1],worldTransform_.matWorld_.m[2][2]},},
		.size{worldTransform_.scale_}
	};
	SetOBB(obbSize);

	//発射開始時と終了時にレーザーのスケールの変更
	if (lifeTimer_ < 5)
	{
		worldTransform_.scale_ = Mathf::Lerp(worldTransform_.scale_, targetScale_, 0.2f);
	}

	if (lifeTimer_ > kLifeTime - 5)
	{
		worldTransform_.scale_ = Mathf::Lerp(worldTransform_.scale_, { 0.0f,0.0f,70.0f }, 0.2f);
	}

	//レーザーの寿命
	if (++lifeTimer_ > kLifeTime)
	{
		isDead_ = true;
	}
}

void Laser::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}

void Laser::OnCollision(Collider* collider)
{

}

const Vector3 Laser::GetWorldPosition() const
{
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}