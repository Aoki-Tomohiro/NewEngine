#include "Player.h"
#include "Engine/Utilities/GlobalVariables.h"
#include "Application/Src/Object/LockOn/LockOn.h"
#include <cassert>
#include <numbers>

//コンボ定数表
const std::array<Player::ConstAttack, Player::ComboNum> Player::kConstAttacks_ = {
	{
		{ 0, 0, 20, 1, 0.0f, 0.0f, 0.14f},
	    { 15, 10, 15, 1, -0.04f, 0.0f, 0.2f },
	    { 15, 10, 15, 30, -0.04f, 0.0f, 0.2f },
	}
};

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
		case Behavior::kJump:
			BehaviorJumpInitialize();
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
	case Behavior::kJump:
		BehaviorJumpUpdate();
		break;
	}

	//当たっていないときは落ちる
	if (onCollision_ == false && behavior_ != Behavior::kJump) {
		const float kGravityAcceleration = 0.05f;
		fallingSpeed_ -= kGravityAcceleration;
		worldTransform_.translation_.y += fallingSpeed_;
	}
	else {
		fallingSpeed_ = 0.0f;
	}

	//落ちたらリスタート
	if (worldTransform_.translation_.y <= -10.0f) {
		Restart();
	}

	//ワールドトランスフォームの更新
	worldTransform_.quaternion_ = Mathf::Slerp(worldTransform_.quaternion_, destinationQuaternion_, 0.4f);
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

void Player::DrawParticle(const Camera& camera) {
	weapon_->DrawParticle(camera);
}

void Player::Restart() {
	//初期座標に戻す
	worldTransform_.translation_ = { 0.0f,0.0f,0.0f };
	worldTransform_.parent_ = nullptr;
	isDead_ = true;
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
	pos.y = worldTransform_.matWorld_.m[3][1] + 1.0f;
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
		velocity_ = {
			input_->GetLeftStickX(),
			0.0f,
			input_->GetLeftStickY(),
		};

		//スティックの押し込みが遊び範囲を超えていたら移動フラグをtrueにする
		if (Mathf::Length(velocity_) > threshold) {
			isMoving = true;
		}

		//スティックによる入力がある場合
		if (isMoving) {
			//速さ
			const float speed = 0.3f;

			//移動量に速さを反映
			velocity_ = Mathf::Normalize(velocity_) * speed;

			//移動ベクトルをカメラの角度だけ回転する
			Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
			velocity_ = Mathf::TransformNormal(velocity_, rotateMatrix);

			//移動
			worldTransform_.translation_ += velocity_;

			//回転
			Rotate(velocity_);
		}
		else if (lockOn_ && lockOn_->ExistTarget()) {
			//ロックオン座標
			Vector3 lockOnPosition = lockOn_->GetTargetPosition();
			//追従対象からロックオン対象へのベクトル
			Vector3 sub = lockOnPosition - GetWorldPosition();

			//距離
			float distance = Mathf::Length(sub);
			//距離しきい値
			const float threshold = 0.2f;

			//しきい値
			if (distance > threshold) {
				//回転
				Rotate(sub);
			}
		}
	}

	//攻撃行動に変更
	if (input_->IsControllerConnected()) {
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER)){
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
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X)) {
			if (workDash_.coolTime == behaviorDashCoolTime) {
				behaviorRequest_ = Behavior::kDash;
			}
		}
	}

	//ジャンプ行動に変更
	if (input_->IsControllerConnected()) {
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A)) {
			behaviorRequest_ = Behavior::kJump;
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

void Player::BehaviorJumpInitialize() {
	worldTransform_.translation_.y = 0.0f;
	const float kJumpFirstSpeed = 1.0f;
	velocity_.y = kJumpFirstSpeed;
	worldTransform_.UnsetParent();
}

void Player::BehaviorJumpUpdate() {
	worldTransform_.translation_ += velocity_;
	const float kGravityAcceleration = 0.05f;
	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector;

	if (worldTransform_.parent_) {
		behaviorRequest_ = Behavior::kRoot;
		worldTransform_.translation_.y = 0.0f;
	}
}

void Player::BehaviorAttackInitialize() {
	//攻撃用の変数を初期化
	workAttack_.attackParameter = 0;
	workAttack_.comboIndex = 0;
	workAttack_.inComboPhase = 0;
	workAttack_.comboNext = false;
	workAttack_.isAttack = true;
	workAttack_.translation = { 0.0f,0.0f,0.0f };
	workAttack_.rotation = { 0.0f,0.0f,0.0f };
}

void Player::BehaviorAttackUpdate() {
	//ロックオン中
	if (lockOn_ && lockOn_->ExistTarget()) {
		//ロックオン座標
		Vector3 lockOnPosition = lockOn_->GetTargetPosition();
		//追従対象からロックオン座標へのベクトル
		Vector3 sub = lockOnPosition - GetWorldPosition();

		//距離
		float distance = Mathf::Length(sub);
		//距離しきい値
		const float threshold = 0.2f;

		//しきい値
		if (distance > threshold) {
			//回転
			Rotate(sub);

			////しきい値を超える速さなら補正する
			//if (speed > distance - threshold) {
			//	speed = distance - threshold;
			//}
		}
	}

	//コンボ上限に達していない
	if (workAttack_.comboIndex < ComboNum - 1) {
		if (input_->IsControllerConnected()) {
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
				//コンボ有効
				workAttack_.comboNext = true;
			}
		}
	}

	//攻撃の合計時間
	uint32_t totalTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime + kConstAttacks_[workAttack_.comboIndex].recoveryTime;
	//既定の時間経過で通常行動に戻る
	if (++workAttack_.attackParameter >= totalTime) {
		//コンボ継続なら次のコンボに進む
		if (workAttack_.comboNext) {
			//コンボ継続フラグをリセット
			workAttack_.comboNext = false;
			workAttack_.attackParameter = 0;
			workAttack_.comboIndex++;
			weapon_->SetIsAttack(false);
			switch (workAttack_.comboIndex) {
			case 0:
				workAttack_.translation = { 0.0f,0.8f,0.0f };
				workAttack_.rotation = { 0.0f,0.0f,0.0f };
				break;
			case 1:
				workAttack_.translation = { 0.0f,0.8f,0.0f };
				workAttack_.rotation = { 1.0f,0.0f,3.14f / 2.0f };
				break;
			case 2:
				workAttack_.translation = { 0.0f,0.8f,0.0f };
				workAttack_.rotation = { 0.0f,0.0f,0.0f };
				break;
			}
		}
		//コンボ継続でないなら攻撃を終了してルートビヘイビアに戻る
		else {
			behaviorRequest_ = Behavior::kRoot;
			workAttack_.isAttack = false;
			weapon_->SetIsAttack(false);
		}
	}

	uint32_t anticipationTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime;
	uint32_t chargeTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime;
	uint32_t swingTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime;

	//コンボ攻撃によってモーションを分岐
	switch (workAttack_.comboIndex) {
	case 0:
		if (workAttack_.attackParameter < anticipationTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].anticipationSpeed;
		}
		
		if (workAttack_.attackParameter >= anticipationTime && workAttack_.attackParameter < chargeTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].chargeSpeed;
		}
		
		if (workAttack_.attackParameter >= chargeTime && workAttack_.attackParameter < swingTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].swingSpeed;
			weapon_->SetIsAttack(true);
		}

		if (workAttack_.attackParameter >= swingTime && workAttack_.attackParameter < totalTime) {
			weapon_->SetIsAttack(false);
		}

		weapon_->SetTranslation(workAttack_.translation);
		weapon_->SetRotation(workAttack_.rotation);
		break;
	case 1:
		if (workAttack_.attackParameter < anticipationTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].anticipationSpeed;
		}
		
		if (workAttack_.attackParameter >= anticipationTime && workAttack_.attackParameter < chargeTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].chargeSpeed;
		}
		
		if (workAttack_.attackParameter >= chargeTime && workAttack_.attackParameter < swingTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].swingSpeed;
			weapon_->SetIsAttack(true);
		}

		if (workAttack_.attackParameter >= swingTime && workAttack_.attackParameter < totalTime) {
			weapon_->SetIsAttack(false);
		}

		weapon_->SetTranslation(workAttack_.translation);
		weapon_->SetRotation(workAttack_.rotation);
		break;
	case 2:
		if (workAttack_.attackParameter < anticipationTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].anticipationSpeed;
		}

		if (workAttack_.attackParameter >= anticipationTime && workAttack_.attackParameter < chargeTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].chargeSpeed;
		}
		
		if (workAttack_.attackParameter >= chargeTime && workAttack_.attackParameter < swingTime) {
			workAttack_.rotation.x += kConstAttacks_[workAttack_.comboIndex].swingSpeed;
			weapon_->SetIsAttack(true);
		}

		if (workAttack_.attackParameter >= swingTime && workAttack_.attackParameter < totalTime) {
			weapon_->SetIsAttack(false);
		}

		weapon_->SetTranslation(workAttack_.translation);
		weapon_->SetRotation(workAttack_.rotation);
		break;
	}
}

void Player::Rotate(const Vector3& v) {
	Vector3 vector = Mathf::Normalize(v);
	Vector3 cross = Mathf::Normalize(Mathf::Cross({ 0.0f,0.0f,1.0f }, vector));
	float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, vector);
	destinationQuaternion_ = Mathf::MakeRotateAxisAngleQuaternion(cross, std::acos(dot));
}

void Player::ApplyGlobalVariables() {
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Player";
	behaviorDashTime_ = globalVariables->GetIntValue(groupName, "BehaviorDashTime");
}