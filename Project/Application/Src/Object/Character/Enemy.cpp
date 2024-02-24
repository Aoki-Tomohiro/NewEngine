#include "Enemy.h"

void Enemy::Initialize(const std::vector<Model*>& models) {
	//基底クラスの初期化
	BaseCharacter::Initialize(models);
	worldTransform_.translation_.z = 0.0f;
	//ワールドトランスフォームの初期化
	worldTransformBody_.Initialize();
	worldTransformL_arm_.Initialize();
	worldTransformL_arm_.translation_.x = -0.8f;
	worldTransformL_arm_.translation_.y = 1.0f;
	worldTransformR_arm_.Initialize();
	worldTransformR_arm_.translation_.x = 0.8f;
	worldTransformR_arm_.translation_.y = 1.0f;
	//親子関係
	worldTransformBody_.parent_ = &worldTransform_;
	worldTransformL_arm_.parent_ = &worldTransformBody_;
	worldTransformR_arm_.parent_ = &worldTransformBody_;
	//浮遊ギミックの初期化
	InitializeFloatingGimmick();
	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionMaskEnemy);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);
}

void Enemy::Update() {
	//移動
	worldTransform_.translation_ += velocity_;

	//移動制限
	if (worldTransform_.translation_.x >= 10.0f || worldTransform_.translation_.x <= -10.0f) {
		velocity_.x *= -1;
	}

	// 浮遊ギミックの更新
	UpdateFloatingGimmick();
	//行列の更新
	BaseCharacter::Update();
	worldTransformBody_.UpdateMatrixFromEuler();
	worldTransformL_arm_.UpdateMatrixFromEuler();
	worldTransformR_arm_.UpdateMatrixFromEuler();
}

void Enemy::Draw(const Camera& camera) {
	//3Dモデルを描画
	models_[0]->Draw(worldTransformBody_, camera);
	models_[1]->Draw(worldTransformL_arm_, camera);
	models_[2]->Draw(worldTransformR_arm_, camera);
}

void Enemy::InitializeFloatingGimmick() {
	// 浮遊ギミックの媒介変数の初期化
	floatingParameter_ = 0.0f;
	// 浮遊移動のサイクルの初期化
	cycle_ = 60;
	// 浮遊の振動の初期化
	amplitude_ = 0.1f;
}

void Enemy::UpdateFloatingGimmick() {
	//1フレームでのパラメータ加算値
	const float step = 2.0f * 3.14f / cycle_;
	//パラメータを１ステップ分加算
	floatingParameter_ += step;
	//2πを超えたら０に戻す
	floatingParameter_ = std::fmod(floatingParameter_, 2.0f * 3.14f);
	//浮遊を座標に反映
	worldTransformBody_.translation_.y = std::sin(floatingParameter_) * amplitude_;
	worldTransformL_arm_.rotation_.y = std::sin(floatingParameter_) * amplitude_;
	worldTransformR_arm_.rotation_.y = -std::sin(floatingParameter_) * amplitude_;
}

void Enemy::OnCollision(Collider* collider) {
	if (collider->GetCollisionAttribute() & kCollisionAttributeWeapon) {
		isDead_ = true;
	}
}

const Vector3 Enemy::GetWorldPosition() const {
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}