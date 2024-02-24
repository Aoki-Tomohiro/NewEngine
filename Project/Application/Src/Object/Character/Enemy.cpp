#include "Enemy.h"
#include "Application/Src/Object/Character/Player.h"

void Enemy::Initialize(const std::vector<Model*>& models) {
	//基底クラスの初期化
	BaseCharacter::Initialize(models);
	startPosition = worldTransform_.translation_;
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
	//前のフレームの当たり判定のフラグを取得
	preOnCollision_ = onCollision_;
	onCollision_ = false;

	//プレイヤーのコンボ数が最大までいったら当たった攻撃数をリセット
	if (isPlayerAttack_ == false) {
		hitCount_ = 0;
	}

	//移動
	worldTransform_.translation_ += velocity_;

	//移動制限
	if (worldTransform_.translation_.x >= 10.0f || worldTransform_.translation_.x <= -10.0f) {
		velocity_.x *= -1;
	}

	if (isDead_) {
		const float kRotSpeed = 0.2f;
		const float kDisappearanceTime = 60.0f;
		worldTransform_.translation_ += deathAnimationVelocity;
		worldTransform_.rotation_.x += kRotSpeed;
		worldTransform_.scale_.x -= 1.0f / kDisappearanceTime;
		worldTransform_.scale_.y -= 1.0f / kDisappearanceTime;
		worldTransform_.scale_.z -= 1.0f / kDisappearanceTime;
		if (worldTransform_.scale_.x <= 0.0f || worldTransform_.scale_.y <= 0.0f || worldTransform_.scale_.z <= 0.0f) {
			isDeathAnimationEnd_ = true;
		}
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
	if (isDead_ == false) {
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
}

void Enemy::OnCollision(Collider* collider) {
	//床と当たっていた場合
	if (collider->GetCollisionAttribute() & kCollisionAttributeFloor) {
		//親を設定する
		parent_ = &collider->GetWorldTransform();
		//現在の親と別の親なら親子付けする
		if (worldTransform_.parent_ != parent_) {
			worldTransform_.UnsetParent();
			worldTransform_.SetParent(parent_);
		}
	}

	if (collider->GetCollisionAttribute() & kCollisionAttributeWeapon) {
		onCollision_ = true;
		if (onCollision_ != preOnCollision_) {
			hitCount_++;
		}

		if (hitCount_ >= Player::ComboNum) {
			isDead_ = true;
			const float kSpeed = 1.0f;
			deathAnimationVelocity = { 0.0f,0.4f,kSpeed };
			deathAnimationVelocity = Mathf::TransformNormal(deathAnimationVelocity, collider->GetWorldTransform().matWorld_);
		}
	}
}

Vector3 Enemy::GetCenterPosition() const {
	//見た目上の中心点オフセット(モデル座標系)
	const Vector3 offset = { 0.0f,1.0f,0.0f };
	//ワールド座標に変換
	Vector3 worldPos = Mathf::Transform(offset, worldTransform_.matWorld_);
	return worldPos;
}

const Vector3 Enemy::GetWorldPosition() const {
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}

void Enemy::Reset() {
	worldTransform_.UnsetParent();
	worldTransform_.translation_ = startPosition;
	worldTransform_.rotation_ = { 0.0f,0.0f,0.0f };
	worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
	isDead_ = false;
	isDeathAnimationEnd_ = false;
}