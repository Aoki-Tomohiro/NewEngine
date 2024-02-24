#include "Floor.h"
#include <cassert>

void Floor::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	//モデルの初期化
	assert(model);
	model_ = model;
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	//速度の初期化
	velocity_ = velocity;

	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributeFloor);
	SetCollisionMask(kCollisionMaskFloor);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);
	AABB aabbSize = {
		{-10.0f,-10.0f,-10.0f},
		{10.0f,10.0f,10.0f},
	};
	SetAABB(aabbSize);
}

void Floor::Update() {
	//移動処理
	worldTransform_.translation_ += velocity_;

	if (worldTransform_.translation_.x >= 30.0f || worldTransform_.translation_.x <= -30.0f) {
		velocity_.x *= -1.0f;
	}

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();
}

void Floor::Draw(const Camera& camera) {
	//モデルの描画
	model_->Draw(worldTransform_, camera);
}

void Floor::OnCollision(Collider* collider) {

}

const Vector3 Floor::GetWorldPosition() const {
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}