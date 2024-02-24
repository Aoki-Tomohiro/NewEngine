#include "Weapon.h"
#include <cassert>

void Weapon::Initialize(Model* model) {
	//モデルの初期化
	assert(model);
	model_ = model;
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_.y = 0.8f;
	worldTransformCollision_.Initialize();
	worldTransformCollision_.translation_.y = 0.8f;
	//worldTransformCollision_.scale_ = { 0.2f,0.6f,1.8f };
	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributeWeapon);
	SetCollisionMask(kCollisionMaskWeapon);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);
	AABB aabbSize = {
		{-worldTransformCollision_.scale_.x,-worldTransformCollision_.scale_.y,-worldTransformCollision_.scale_.z},
		{worldTransformCollision_.scale_.x,worldTransformCollision_.scale_.y,worldTransformCollision_.scale_.z} };
	SetAABB(aabbSize);
}

void Weapon::Update() {

	//攻撃中の時アニメーションタイマーを進める
	if (isAttack_) {
		animationTimer_++;

		//振りかぶりアニメーション
		if (animationCount_ == 0) {
			if (animationTimer_ == 30) {
				animationCount_++;
				animationTimer_ = 0;
			}
			worldTransform_.rotation_.x -= 0.1f;
		}

		//攻撃アニメーション
		if (animationCount_ == 1) {
			if (animationTimer_ == 15) {
				animationCount_++;
				animationTimer_ = 0;
			}
			worldTransform_.rotation_.x += 0.2f;
			isHit_ = true;
		}

		//硬直アニメーション
		if (animationCount_ == 2) {
			if (animationTimer_ == 30) {
				animationCount_++;
				animationTimer_ = 0;
				isAttack_ = false;
				isHit_ = false;
			}
		}
	}

	//当たり判定の位置を決める
	Vector3 direction{ 0.0f,0.0f,4.0f };
	direction = Mathf::TransformNormal(direction, worldTransform_.matWorld_);
	worldTransformCollision_.translation_ = { worldTransform_.matWorld_.m[3][0],worldTransform_.matWorld_.m[3][1] ,worldTransform_.matWorld_.m[3][2] };
	worldTransformCollision_.translation_ += direction;

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();
	worldTransformCollision_.UpdateMatrixFromEuler();
}

void Weapon::Draw(const Camera& camera) {
	//モデルの描画
	model_->Draw(worldTransform_, camera);
}

void Weapon::OnCollision(Collider* collider) {

}

const Vector3 Weapon::GetWorldPosition() const {
	Vector3 pos{};
	pos.x = worldTransformCollision_.matWorld_.m[3][0];
	pos.y = worldTransformCollision_.matWorld_.m[3][1];
	pos.z = worldTransformCollision_.matWorld_.m[3][2];
	return pos;
}

void Weapon::Attack(){
	if (isAttack_ == false) {
		Weapon::AttackInitialize();
		isAttack_ = true;
	}
}

void Weapon::AttackInitialize() {
	animationTimer_ = 0;
	animationCount_ = 0;
	worldTransform_.rotation_.x = 1.5f;
}