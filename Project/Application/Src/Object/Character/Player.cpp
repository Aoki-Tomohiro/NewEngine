#include "Player.h"
#include "Engine/Utilities/GlobalVariables.h"
#include <cassert>
#include <numbers>

void Player::Initialize(const std::vector<Model*>& models) {
	//基底クラスの初期化
	BaseCharacter::Initialize(models);
	//入力クラスのインスタンスを取得
	input_ = Input::GetInstance();
	//武器の作成
	weapon_ = std::make_unique<Weapon>();
	weapon_->Initialize(models_[1]);
	weapon_->SetParent(&worldTransform_);
	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	SetCollisionMask(kCollisionMaskPlayer);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);

	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Player";
	//グループを追加
	globalVariables->CreateGroup(groupName);
	globalVariables->AddItem(groupName, "BehaviorDashTime", behaviorDashTime_);
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


	//Behaviorの遷移処理
	if (behaviorRequest_) {
		//振る舞いを変更する
		behavior_ = behaviorRequest_.value();
		//各振る舞いごとの初期化を実行
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kDash:
			BehaviorDashInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}
		behaviorRequest_ = std::nullopt;
	}


	//Behaviorの実行
	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	case Behavior::kDash:
		BehaviorDashUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
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
	worldTransform_.quaternion_ = Mathf::Slerp(worldTransform_.quaternion_, moveQuaternion_, 0.4f);
	worldTransform_.UpdateMatrixFromQuaternion();

	//武器の更新
	weapon_->Update();

	//次のフレーム用のフラグを保存
	preOnCollision_ = onCollision_;
	onCollision_ = false;

	//グローバル変数の適応
	Player::ApplyGlobalVariables();
}

void Player::Draw(const Camera& camera) {
	//モデルの描画
	models_[0]->Draw(worldTransform_, camera);
	if (behavior_ == Behavior::kAttack) {
		weapon_->Draw(camera);
	}
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

void Player::BehaviorRootInitialize() {

}

void Player::BehaviorRootUpdate() {
	//ダッシュのクールタイム
	const uint32_t behaviorDashCoolTime = 60;

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

			//移動
			worldTransform_.translation_ += move;

			//回転
			move = Mathf::Normalize(move);
			Vector3 cross = Mathf::Normalize(Mathf::Cross({ 0.0f,0.0f,1.0f }, move));
			float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, move);
			moveQuaternion_ = Mathf::MakeRotateAxisAngleQuaternion(cross, std::acos(dot));
		}
	}

	//攻撃行動に変更
	if (input_->IsControllerConnected()) {
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
			if (workDash_.coolTime == behaviorDashCoolTime) {
				behaviorRequest_ = Behavior::kAttack;
			}
		}
	}

	//ダッシュ行動に変更
	if (workDash_.coolTime != behaviorDashCoolTime) {
		workDash_.coolTime++;
	}

	if (input_->IsControllerConnected()) {
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A)) {
			if (workDash_.coolTime == behaviorDashCoolTime) {
				behaviorRequest_ = Behavior::kDash;
			}
		}
	}
}

void Player::BehaviorDashInitialize() {
	workDash_.dashParameter_ = 0;
	workDash_.coolTime = 0;
	worldTransform_.rotation_.y = destinationAngleY_;
}

void Player::BehaviorDashUpdate() {

	if (input_->IsControllerConnected()) {
		//速さ
		float kSpeed = 1.0f;
		//移動量
		Vector3 move = {
			input_->GetLeftStickX(),
			0.0f,
			input_->GetLeftStickY(),
		};

		//移動量に速さを反映
		move = Mathf::Normalize(move) * kSpeed;

		//移動ベクトルをカメラの角度だけ回転する
		Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
		move = Mathf::TransformNormal(move, rotateMatrix);

		//移動
		worldTransform_.translation_ += move;
	}

	//規定の時間経過で通常行動に戻る
	if (++workDash_.dashParameter_ >= behaviorDashTime_) {
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::BehaviorAttackInitialize() {
	//攻撃開始
	weapon_->Attack();
}

void Player::BehaviorAttackUpdate() {
	//攻撃が終わったら通常状態に戻す
	if (weapon_->GetIsAttack() == false) {
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::ApplyGlobalVariables() {
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Player";
	behaviorDashTime_ = globalVariables->GetIntValue(groupName, "BehaviorDashTime");
}