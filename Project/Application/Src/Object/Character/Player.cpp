#include "Player.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include <cassert>

void Player::Initialize(const std::vector<Model*>& models) {
	//基底クラスの初期化
	BaseCharacter::Initialize(models);
	//入力クラスのインスタンスを取得
	input_ = Input::GetInstance();
	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	SetCollisionMask(kCollisionMaskPlayer);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);
}

void Player::Update() {

	//当たった瞬間に親子付けする
	if (preOnCollision_ == false && onCollision_ == true) {
		worldTransform_.SetParent(parent_);
	}

	//離れた瞬間に親子付けを外す
	if (preOnCollision_ == true && onCollision_ == false) {
		worldTransform_.UnsetParent();
	}

	XINPUT_STATE joyState{};

	if (input_->IsControllerConnected()) {

		//しきい値
		const float threshold = 0.7f;

		//移動フラグ
		bool isMoving = false;

		//移動量
		Vector3 move = {
			input_->GetLeftStickX(),
			0.0f,
			input_->GetLeftStickY(),
		};

		//スティックの押し込みが遊び範囲を超えていたら移動フラグをtrueにする
		if (Mathf::Length(move) > threshold) {
			isMoving = true;
		}

		if (isMoving) {

			//速さ
			const float speed = 0.3f;

			//移動量に速さを反映
			move = Mathf::Normalize(move) * speed;

			//移動ベクトルをカメラの角度だけ回転する
			Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
			move = Mathf::TransformNormal(move, rotateMatrix);

			//移動方向に見た目を合わせる
			worldTransform_.rotation_.y = std::atan2(move.x, move.z);

			//移動
			worldTransform_.translation_ += move;
		}
	}

	//当たっていないときは落ちる
	if (onCollision_ == false) {
		worldTransform_.translation_.y -= 0.1f;
	}
	else {
		worldTransform_.translation_.y = 0.0f;
	}

	//落ちたらリスタート
	if (worldTransform_.translation_.y <= -5.0f) {
		Restart();
	}

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();

	//次のフレーム用のフラグを保存
	preOnCollision_ = onCollision_;
	onCollision_ = false;
}

void Player::Draw(const Camera& camera) {
	//基底クラスの描画
	BaseCharacter::Draw(camera);
}

void Player::Restart() {
	//初期座標に戻す
	worldTransform_.translation_ = { 0.0f,0.0f,0.0f };
	worldTransform_.parent_ = nullptr;
}

void Player::OnCollision(Collider* collider) {
	//床と当たっていた場合
	if (collider->GetCollisionAttribute() & kCollisionAttributeFloor) {
		//衝突フラグを立てる
		onCollision_ = true;
		//親を設定する
		parent_ = &collider->GetWorldTransform();
		//現在の親と別の親なら親子付けする
		if (worldTransform_.parent_ != parent_) {
			worldTransform_.UnsetParent();
			worldTransform_.SetParent(parent_);
		}
	}

	//敵と当たったらリスタート
	if (collider->GetCollisionAttribute() & kCollisionAttributeEnemy) {
		Restart();
	}

	//ゴールに触れたらリスタート
	if (collider->GetCollisionAttribute() & kCollisionAttributeGoal) {
		Restart();
	}
}

const Vector3 Player::GetWorldPosition() const {
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}
