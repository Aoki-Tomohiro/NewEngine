#include "Player.h"
#include "Engine/3D/Model/ModelManager.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Base/ImGuiManager.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Application/Src/Object/LockOn/LockOn.h"
#include "Application/Src/Object/Boss/Boss.h"

void Player::Initialize()
{
	worldTransform_.Initialize();
	worldTransform_.translation_.y = 1.0f;
	worldTransform_.translation_.z = -20.0f;

	//入力クラスのインスタンスを取得
	input_ = Input::GetInstance();

	//オーディオのインスタンスを取得
	audio_ = Audio::GetInstance();

	//武器の生成
	modelWeapon_.reset(ModelManager::CreateFromOBJ("Weapon", Opaque));
	modelWeapon_->SetEnableLighting(false);
	weapon_ = GameObjectManager::CreateGameObject<Weapon>();
	weapon_->SetModel(modelWeapon_.get());
	weapon_->SetPlayerWorldTransform(&worldTransform_);

	//オーディオの読み込み
	swishAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Swish.wav");
	damageAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Damage1.wav");
	dashAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Dash.wav");
	jumpAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Jump1.wav");

	//テクスチャ読み込み
	TextureManager::Load("HpBarFrame.png");
	TextureManager::Load("HpBar.png");

	//スプライトの生成
	spriteHpBar_.reset(Sprite::Create("HpBar.png", { 80.0f,32.0f }));
	spriteHpBar_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	spriteHpBarFrame_.reset(Sprite::Create("HpBarFrame.png", { 79.0f,31.0f }));
	spriteHpBarFrame_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });

	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	SetCollisionMask(kCollisionMaskPlayer);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);

	//パーティクルの生成
	particleModel_.reset(ModelManager::CreateFromOBJ("Cube", Transparent));
	particleModel_->SetEnableLighting(false);
	particleSystem_ = ParticleManager::Create("Dash");
	particleSystem_->SetModel(particleModel_.get());
	particleSystem_->SetIsBillBoard(false);
}

void Player::Update()
{
	isHit_ = false;

	//ダッシュのクールタイム
	if (workDash_.coolTime != workDash_.dashCoolTime)
	{
		workDash_.coolTime++;
	}

	//Behaviorの遷移処理
	if (behaviorRequest_)
	{
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
		case Behavior::kJump:
			BehaviorJumpInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		case Behavior::kAirAttack:
			BehaviorAirAttackInitialize();
			break;
		case Behavior::kKnockBack:
			BehaviorKnockBackInitialize();
			break;
		case Behavior::kRapidApproach:
			BehaviorRapidApproachInitialize();
			break;
		}
		behaviorRequest_ = std::nullopt;
	}

	//Behaviorの実行
	switch (behavior_)
	{
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	case Behavior::kDash:
		BehaviorDashUpdate();
		break;
	case Behavior::kJump:
		BehaviorJumpUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	case Behavior::kAirAttack:
		BehaviorAirAttackUpdate();
		break;
	case Behavior::kKnockBack:
		BehaviorKnockBackUpdate();
		break;
	case Behavior::kRapidApproach:
		BehaviorRapidApproachUpdate();
		break;
	}

	//移動限界座標
	const float kMoveLimitX = 49;
	const float kMoveLimitZ = 49;
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.z = max(worldTransform_.translation_.z, -kMoveLimitZ);
	worldTransform_.translation_.z = min(worldTransform_.translation_.z, +kMoveLimitZ);

	//ワールドトランスフォームの更新
	worldTransform_.quaternion_ = Mathf::Slerp(worldTransform_.quaternion_, destinationQuaternion_, 0.4f);
	worldTransform_.UpdateMatrixFromQuaternion();

	//HPバーの処理
	hpBarSize_ = { (hp_ / kMaxHP) * 480.0f,16.0f };
	spriteHpBar_->SetSize(hpBarSize_);

	//無敵時間の処理
	if (invincibleFlag_)
	{
		if (++invincibleTimer_ > kInvincibleTime)
		{
			invincibleFlag_ = false;
		}
	}

	ImGui::Begin("Player");
	ImGui::Text("HP : %f", hp_);
	ImGui::End();
}

void Player::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);

	//武器の描画
	if (behavior_ == Behavior::kAttack || behavior_ == Behavior::kAirAttack)
	{
		weapon_->SetIsVisible(true);
	}
	else
	{
		weapon_->SetIsVisible(false);
	}
}

void Player::DrawUI()
{
	spriteHpBar_->Draw();
	spriteHpBarFrame_->Draw();
}

void Player::OnCollision(Collider* collider)
{
	if (collider->GetCollisionAttribute() == kCollisionAttributeEnemy)
	{
		const Boss* boss = GameObjectManager::GetInstance()->GetGameObject<Boss>("Boss");
		if (!boss->GetIsAttack())
		{
			AABB aabbA = {
				.min{worldTransform_.translation_.x + GetAABB().min.x,worldTransform_.translation_.y + GetAABB().min.y,worldTransform_.translation_.z + GetAABB().min.z},
				.max{worldTransform_.translation_.x + GetAABB().max.x,worldTransform_.translation_.y + GetAABB().max.y,worldTransform_.translation_.z + GetAABB().max.z},
			};
			AABB aabbB = {
				.min{collider->GetWorldTransform().translation_.x + collider->GetAABB().min.x,collider->GetWorldTransform().translation_.y + collider->GetAABB().min.y,collider->GetWorldTransform().translation_.z + collider->GetAABB().min.z},
				.max{collider->GetWorldTransform().translation_.x + collider->GetAABB().max.x,collider->GetWorldTransform().translation_.y + collider->GetAABB().max.y,collider->GetWorldTransform().translation_.z + collider->GetAABB().max.z},
			};

			Vector3 overlapAxis = {
				std::min<float>(aabbA.max.x,aabbB.max.x) - std::max<float>(aabbA.min.x,aabbB.min.x),
				std::min<float>(aabbA.max.y,aabbB.max.y) - std::max<float>(aabbA.min.y,aabbB.min.y),
				std::min<float>(aabbA.max.z,aabbB.max.z) - std::max<float>(aabbA.min.z,aabbB.min.z),
			};

			Vector3 directionAxis{};
			if (overlapAxis.x < overlapAxis.y && overlapAxis.x < overlapAxis.z) {
				//X軸方向で最小の重なりが発生している場合
				directionAxis.x = (worldTransform_.translation_.x < collider->GetWorldTransform().translation_.x) ? -1.0f : 1.0f;
				directionAxis.y = 0.0f;
			}
			else if (overlapAxis.y < overlapAxis.x && overlapAxis.y < overlapAxis.z) {
				//Y軸方向で最小の重なりが発生している場合
				directionAxis.y = (worldTransform_.translation_.y < collider->GetWorldTransform().translation_.y) ? -1.0f : 1.0f;
				directionAxis.x = 0.0f;
			}
			else if (overlapAxis.z < overlapAxis.x && overlapAxis.z < overlapAxis.y)
			{
				directionAxis.z = (worldTransform_.translation_.z < collider->GetWorldTransform().translation_.z) ? -1.0f : 1.0f;
				directionAxis.x = 0.0f;
				directionAxis.y = 0.0f;
			}

			worldTransform_.translation_ += overlapAxis * directionAxis;
			worldTransform_.UpdateMatrixFromQuaternion();
		}
		//攻撃中の場合吹っ飛ばす
		else
		{
			if (behavior_ != Behavior::kDash)
			{
				Vector3 kKnockBackSpeed = { 0.0f,0.2f,0.4f };
				knockBackVelocity_ = Mathf::TransformNormal(kKnockBackSpeed, boss->GetWorldTransform().matWorld_);
				behaviorRequest_ = Behavior::kKnockBack;
				if (!invincibleFlag_)
				{
					audio_->SoundPlayWave(damageAudioHandle_, false, 0.5f);
					isHit_ = true;
					invincibleFlag_ = true;
					invincibleTimer_ = 0;
					hp_ -= boss->GetDamage();
					if (hp_ <= 0.0f)
					{
						hp_ = 0.0f;
					}
				}
			}
		}
	}

	//ミサイルの衝突判定
	if (collider->GetCollisionAttribute() == kCollisionAttributeMissile)
	{
		if (behavior_ != Behavior::kDash)
		{
			if (!invincibleFlag_)
			{
				audio_->SoundPlayWave(damageAudioHandle_, false, 0.5f);
				isHit_ = true;
				invincibleFlag_ = true;
				invincibleTimer_ = 0;
				hp_ -= 10.0f;
				if (hp_ <= 0.0f)
				{
					hp_ = 0.0f;
				}
			}
		}
	}

	//レーザーの衝突判定
	if (collider->GetCollisionAttribute() == kCollisionAttributeLaser)
	{
		if (behavior_ != Behavior::kDash)
		{
			if (!invincibleFlag_)
			{
				audio_->SoundPlayWave(damageAudioHandle_, false, 0.5f);
				isHit_ = true;
				invincibleFlag_ = true;
				invincibleTimer_ = 0;
				hp_ -= 10.0f;
				if (hp_ <= 0.0f)
				{
					hp_ = 0.0f;
				}
			}
		}
	}
}

const Vector3 Player::GetWorldPosition() const
{
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}

void Player::BehaviorRootInitialize()
{
	fallingSpeed_ = 0.0f;
}

void Player::BehaviorRootUpdate()
{
	if (input_->IsControllerConnected())
	{
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
		if (Mathf::Length(velocity_) > threshold)
		{
			isMoving = true;
		}

		//スティックによる入力がある場合
		if (isMoving)
		{
			isParticleActive_ = true;

			//速さ
			//const float speed = 0.3f;
			const float speed = 0.6f;

			//移動量に速さを反映
			velocity_ = Mathf::Normalize(velocity_) * speed;

			//移動ベクトルをカメラの角度だけ回転する
			Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
			velocity_ = Mathf::TransformNormal(velocity_, rotateMatrix);

			//移動
			worldTransform_.translation_ += velocity_;

			//回転
			Rotate(velocity_);

			////パーティクルの生成
			//if (particleSystem_->GetParticleEmitter("Move") == nullptr)
			//{
			//	ParticleEmitter* emitter = ParticleEmitterBuilder()
			//		.SetDeleteTime(60.0f * 3.0f)
			//		.SetEmitterName("Move")
			//		.SetPopArea({ -worldTransform_.scale_.x,0.0f,-worldTransform_.scale_.z }, { worldTransform_.scale_.x,0.0f,worldTransform_.scale_.x })
			//		.SetPopAzimuth(0.0f, 0.0f)
			//		.SetPopColor({ 0.5f, 0.5f, 0.5f, 1.0f }, { 0.5f, 0.5f, 0.5f, 1.0f })
			//		.SetPopCount(4)
			//		.SetPopElevation(0.0f, 0.0f)
			//		.SetPopFrequency(0.1f)
			//		.SetPopLifeTime(0.2f, 0.4f)
			//		.SetPopRotation({ 0.0f,0.0f,0.0f }, { 3.14f,3.14f,3.14f })
			//		.SetPopScale({ 0.2f,0.2f,0.2f }, { 0.4f,0.4f,0.4f })
			//		.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			//		.SetTranslation({ worldTransform_.translation_.x ,0.0f,worldTransform_.translation_.z })
			//		.Build();
			//	particleSystem_->AddParticleEmitter(emitter);
			//}
			//else
			//{
			//	particleSystem_->GetParticleEmitter("Move")->SetTranslation({ worldTransform_.translation_.x ,0.0f,worldTransform_.translation_.z });
			//	particleSystem_->GetParticleEmitter("Move")->SetPopCount(4);
			//}
		}
		else
		{
			//isParticleActive_ = false;
			//if (particleSystem_->GetParticleEmitter("Move"))
			//{
			//	particleSystem_->GetParticleEmitter("Move")->SetPopCount(0);
			//}
		}
	}

	//地面にいなかったら落下する
	if (worldTransform_.translation_.y >= 1.0f)
	{
		const float kGravityAcceleration = 0.05f;
		fallingSpeed_ -= kGravityAcceleration;
		worldTransform_.translation_.y += fallingSpeed_;

		if (worldTransform_.translation_.y < 1.0f)
		{
			worldTransform_.translation_.y = 1.0f;
			isAirAttack_ = false;
		}
	}

	//ダッシュ行動に変更
	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER))
		{
			if (workDash_.coolTime == workDash_.dashCoolTime)
			{
				behaviorRequest_ = Behavior::kDash;
			}
		}
	}

	//ジャンプ行動に変更
	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A))
		{
			behaviorRequest_ = Behavior::kJump;
		}
	}

	//攻撃行動に変更
	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
		{
			if (!isAirAttack_)
			{
				behaviorRequest_ = Behavior::kAttack;
			}
		}
	}

	////急接近行動に変更
	//if (input_->IsControllerConnected())
	//{
	//	if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_B))
	//	{
	//		behaviorRequest_ = Behavior::kRapidApproach;
	//	}
	//}
}

void Player::BehaviorDashInitialize()
{
	workDash_.dashParameter = 0;
	workDash_.coolTime = 0;
	worldTransform_.quaternion_ = destinationQuaternion_;

	if (input_->IsControllerConnected())
	{
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
		if (Mathf::Length(velocity_) > threshold)
		{
			isMoving = true;
		}

		//スティックによる入力がある場合
		if (isMoving)
		{
			//速さ
			float kSpeed = 1.0f;

			//移動量に速さを反映
			velocity_ = Mathf::Normalize(velocity_) * kSpeed;

			//移動ベクトルをカメラの角度だけ回転する
			Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
			velocity_ = Mathf::TransformNormal(velocity_, rotateMatrix);
		}
		//入力がない場合後ろに下がるようにする
		else
		{
			velocity_ = { 0.0f,0.0f,-1.0f };

			//移動ベクトルをプレイヤーの角度だけ回転する
			velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);
		}
	}

	audio_->SoundPlayWave(dashAudioHandle_, false, 0.5f);

	////パーティクルの生成
	//ParticleEmitter* emitter = ParticleEmitterBuilder()
	//	.SetDeleteTime(10)
	//	.SetEmitterName("Dash")
	//	.SetPopArea({ -worldTransform_.scale_.x,worldTransform_.scale_.y,-worldTransform_.scale_.z }, { worldTransform_.scale_.x,worldTransform_.scale_.y,worldTransform_.scale_.x })
	//	.SetPopAzimuth(0.0f, 0.0f)
	//	.SetPopColor({ 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f })
	//	.SetPopCount(100)
	//	.SetPopElevation(0.0f, 0.0f)
	//	.SetPopFrequency(20.0f)
	//	.SetPopLifeTime(0.2f, 0.4f)
	//	.SetPopRotation({0.0f,0.0f,0.0f}, { 0.0f,0.0f,0.0f })
	//	.SetPopScale({ 0.01f,0.01f,0.8f }, { 0.01f,0.01f,1.2f })
	//	.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
	//	.SetTranslation(worldTransform_.translation_)
	//	.Build();
	//particleSystem_->AddParticleEmitter(emitter);
}

void Player::BehaviorDashUpdate()
{
	//移動
	worldTransform_.translation_ += velocity_;

	//規定の時間経過で通常行動に戻る
	const float dashTime = 10;
	if (++workDash_.dashParameter >= dashTime)
	{
		behaviorRequest_ = Behavior::kRoot;
	}

	////エミッターの座標をずらす
	//if (particleSystem_->GetParticleEmitter("Dash"))
	//{
	//	particleSystem_->GetParticleEmitter("Dash")->SetTranslation(worldTransform_.translation_);
	//}
}

void Player::BehaviorJumpInitialize()
{
	const float kJumpFirstSpeed = 0.7f;
	velocity_.x = 0.0f;
	velocity_.y = kJumpFirstSpeed;
	velocity_.z = 0.0f;
	audio_->SoundPlayWave(jumpAudioHandle_, false, 0.2f);
}

void Player::BehaviorJumpUpdate()
{
	if (input_->IsControllerConnected())
	{
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
		if (Mathf::Length(move) > threshold)
		{
			isMoving = true;
		}

		//スティックによる入力がある場合
		if (isMoving)
		{
			//速さ
			const float speed = 0.3f;
			//const float speed = 0.6f;

			//移動量に速さを反映
			move = Mathf::Normalize(move) * speed;

			//移動ベクトルをカメラの角度だけ回転する
			Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
			move = Mathf::TransformNormal(move, rotateMatrix);

			//移動
			worldTransform_.translation_ += move;

			//回転
			Rotate(move);
		}
	}

	worldTransform_.translation_ += velocity_;
	const float kGravityAcceleration = 0.05f;
	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector;

	//攻撃行動に変更
	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
		{
			if (!isAirAttack_)
			{
				behaviorRequest_ = Behavior::kAirAttack;
			}
		}
	}

	//ダッシュ行動に変更
	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER))
		{
			if (workDash_.coolTime == workDash_.dashCoolTime)
			{
				behaviorRequest_ = Behavior::kDash;
			}
		}
	}

	//通常状態に変更
	if (worldTransform_.translation_.y <= 1.0f)
	{
		behaviorRequest_ = Behavior::kRoot;
		worldTransform_.translation_.y = 1.0f;
	}
}

void Player::BehaviorAttackInitialize()
{
	//攻撃用の変数を初期化
	workAttack_.translation = { 0.0f,0.0f,0.0f };
	workAttack_.rotation = { 0.0f,0.0f,0.0f };
	workAttack_.attackParameter = 0;
	workAttack_.comboIndex = 0;
	workAttack_.inComboPhase = 0;
	workAttack_.comboNext = false;
}

//コンボ定数表
//アニメーション開始座標 //アニメーション開始角度 振りかぶりの時間 ための時間 攻撃振りの時間 硬直時間 振りかぶりの移動速度 振りかぶりの回転速度 ための移動速度 ための回転速度,攻撃降りの移動速度 攻撃降りの回転速度
std::array<Player::ConstAttack, Player::ComboNum> Player::kConstAttacks_ = {
	{
		//開始座標             開始角度              振りかぶりの時間 ための時間 攻撃振りの時間 硬直時間  振りかぶりの移動速度  振りかぶりの回転速度  ための移動速度      ための回転速度       攻撃降りの移動速度   攻撃降りの回転速度      
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f },  0,            0,       5,          16,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.4f,0.0f,0.0f},  {0.0f,0.0f,0.6f}},
		{{ 0.0f,0.0f,0.0f }, { 2.8f,0.0f,-1.57f}, 0,            0,       5,          16,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {-0.4f,0.0f,0.0f}, {0.0f,0.0f,0.6f}},
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,1.57f }, 0,            0,       20,         16,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.75f,0.0f,0.0f}, {0.0f,0.0f,0.6f}},
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f },  10,           10,      5,          22,      {0.0f,0.0f,0.0f}, {-0.1f,0.0f,0.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.56f,0.0f,0.0f}, {0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 2.345f,0.0f,0.0f}, 0,            0,       8,          10,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {-0.26f,0.0f,0.0f},{0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 0.0f,0.0f,0.0f },  0,            0,       40,         10,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.72f,0.0f,0.0f}, {0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 0.0f,0.0f,0.0f },  0,            0,       20,         60,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.4f,0.0f,0.0f},  {0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 0.0f,0.0f,0.0f },  0,            0,       20,         60,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.4f,0.0f,0.0f},  {0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 0.0f,0.0f,0.0f },  0,            0,       20,         60,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.4f,0.0f,0.0f},  {0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 0.0f,0.0f,0.0f },  0,            0,       20,         60,      {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.4f,0.0f,0.0f},  {0.0f,0.0f,0.0f}},
	}
};

void Player::BehaviorAttackUpdate()
{
	//コンボ上限に達していない
	if (workAttack_.comboIndex < ComboNum - 1)
	{
		if (input_->IsControllerConnected())
		{
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
			{
				if (!isAttack_)
				{
					//コンボ有効
					workAttack_.comboNext = true;
					isAttack_ = true;
				}
			}

			if (!input_->IsPressButton(XINPUT_GAMEPAD_X))
			{
				isAttack_ = false;
			}
		}
	}

	//攻撃の合計時間
	uint32_t totalTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime + kConstAttacks_[workAttack_.comboIndex].recoveryTime;
	//既定の時間経過で通常行動に戻る
	if (++workAttack_.attackParameter >= totalTime)
	{
		//コンボ継続なら次のコンボに進む
		if (workAttack_.comboNext)
		{
			//コンボ継続フラグをリセット
			workAttack_.comboNext = false;
			workAttack_.attackParameter = 0;
			workAttack_.comboIndex++;
			if (workAttack_.comboIndex >= 0 && workAttack_.comboIndex <= ComboNum - 1)
			{
				workAttack_.translation = kConstAttacks_[workAttack_.comboIndex].startPosition;
				workAttack_.rotation = kConstAttacks_[workAttack_.comboIndex].startRotation;
			}
			weapon_->SetIsAttack(false);

			if (input_->IsControllerConnected())
			{
				//しきい値
				const float threshold = 0.7f;

				//移動フラグ
				bool isRotation = false;

				//移動量
				Vector3 direction = {
					input_->GetLeftStickX(),
					0.0f,
					input_->GetLeftStickY(),
				};

				//スティックの押し込みが遊び範囲を超えていたら移動フラグをtrueにする
				if (Mathf::Length(direction) > threshold)
				{
					isRotation = true;
				}

				//スティックによる入力がある場合
				if (isRotation)
				{
					//移動量に速さを反映
					direction = Mathf::Normalize(direction);

					//移動ベクトルをカメラの角度だけ回転する
					Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
					direction = Mathf::TransformNormal(direction, rotateMatrix);

					//回転
					Rotate(direction);
				}
			}
		}
		//コンボ継続でないなら攻撃を終了してルートビヘイビアに戻る
		else
		{
			behaviorRequest_ = Behavior::kRoot;
			workAttack_.comboIndex = 0;
			weapon_->SetIsAttack(false);
		}
	}

	//移動フラグ
	bool isMove = true;

	//ボスの座標を取得
	Vector3 targetPosition = GameObjectManager::GetInstance()->GetGameObject<Boss>("Boss")->GetWorldPosition();

	//差分ベクトルを計算
	Vector3 sub = targetPosition - GetWorldPosition();

	//Y軸は必要ないので0にする
	sub.y = 0.0f;

	//距離を計算
	float distance = Mathf::Length(sub);

	//閾値
	float threshold = 16.0f;

	//ボスとの距離が閾値より小さかったらボスの方向に回転させる
	if (distance < threshold || lockOn_->ExistTarget())
	{
		//回転
		Rotate(sub);

		if (distance < 6.0f)
		{
			//アニメーションの移動フラグをfalseにする
			isMove = false;
		}
	}


	switch (workAttack_.comboIndex)
	{
	case 0:
		AttackAnimation(isMove);
		damage_ = 8;
		break;
	case 1:
		AttackAnimation(isMove);
		damage_ = 8;
		break;
	case 2:
		AttackAnimation(isMove);
		damage_ = 5;
		break;
	case 3:
		AttackAnimation(isMove);
		damage_ = 30;
		break;
	}

	weapon_->SetTranslation(workAttack_.translation);
	weapon_->SetRotation(workAttack_.rotation);
}

void Player::AttackAnimation(bool isMove)
{
	//総合時間
	uint32_t totalTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime + kConstAttacks_[workAttack_.comboIndex].recoveryTime;

	//現在のコンボの情報を取得
	uint32_t anticipationTime = 0, chargeTime = 0, swingTime = 0, recoveryTime = 0;

	//移動速度
	velocity_ = kConstAttacks_[workAttack_.comboIndex].velocity;
	velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

	//配列外参照を防ぐ
	if (workAttack_.comboIndex >= 0 && workAttack_.comboIndex <= ComboNum - 1)
	{
		anticipationTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime;
		chargeTime = anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime;
		swingTime = chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime;
		recoveryTime = swingTime + kConstAttacks_[workAttack_.comboIndex].recoveryTime;
	}

	//振りかぶりの処理
	if (workAttack_.attackParameter < anticipationTime)
	{
		workAttack_.translation += kConstAttacks_[workAttack_.comboIndex].anticipationSpeed;
		workAttack_.rotation += kConstAttacks_[workAttack_.comboIndex].anticipationRotateSpeed;
		if (isMove)
		{
			worldTransform_.translation_ += velocity_;
		}
	}

	//チャージの処理
	if (workAttack_.attackParameter >= anticipationTime && workAttack_.attackParameter < chargeTime)
	{
		workAttack_.translation += kConstAttacks_[workAttack_.comboIndex].chargeSpeed;
		workAttack_.rotation += kConstAttacks_[workAttack_.comboIndex].chargeRotateSpeed;
		if (isMove)
		{
			worldTransform_.translation_ += velocity_;
		}
	}

	//攻撃振りの処理
	if (workAttack_.attackParameter >= chargeTime && workAttack_.attackParameter < swingTime)
	{
		//移動処理
		workAttack_.translation += kConstAttacks_[workAttack_.comboIndex].swingSpeed;
		workAttack_.rotation += kConstAttacks_[workAttack_.comboIndex].swingRotateSpeed;
		if (isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//衝突判定をつける処理
		uint32_t collisionTime = swingTime - chargeTime;
		int collisionInterval = collisionTime / 4;
		if (workAttack_.attackParameter % collisionInterval == 0)
		{
			weapon_->SetIsAttack(true);
			//オーディオ再生
			if (!isSwishPlayed_)
			{
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
				isSwishPlayed_ = true;
			}
		}
		else
		{
			weapon_->SetIsAttack(false);
			isSwishPlayed_ = false;
		}
	}

	//硬直時間
	if (workAttack_.attackParameter >= swingTime && workAttack_.attackParameter < recoveryTime)
	{
		weapon_->SetIsAttack(false);
		isSwishPlayed_ = false;
	}

	//入力受付時間
	if (workAttack_.attackParameter >= recoveryTime && workAttack_.attackParameter < totalTime)
	{
		if (input_->IsControllerConnected())
		{
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
			{
				//コンボ有効
				workAttack_.attackParameter = totalTime;
			}
		}
	}
}

void Player::BehaviorAirAttackInitialize()
{
	//攻撃用の変数を初期化
	workAttack_.translation = kConstAirAttacks_[0].startPosition;
	workAttack_.rotation = kConstAirAttacks_[0].startRotation;
	workAttack_.attackParameter = 0;
	workAttack_.comboIndex = 0;
	workAttack_.inComboPhase = 0;
	workAttack_.comboNext = false;
	isAirAttack_ = true;
}

//コンボ定数表
//アニメーション開始座標 //アニメーション開始角度 振りかぶりの時間 ための時間 攻撃振りの時間 硬直時間 入力受付時間 振りかぶりの移動速度 振りかぶりの回転速度 ための移動速度 ための回転速度,攻撃降りの移動速度 攻撃降りの回転速度
std::array<Player::ConstAttack, Player::airComboNum> Player::kConstAirAttacks_ = {
	{
		//開始座標             開始角度              振りかぶりの時間 ための時間 攻撃振りの時間 硬直時間  振りかぶりの移動速度  振りかぶりの回転速度  ための移動速度      ための回転速度       攻撃降りの移動速度   攻撃降りの回転速度      
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-0.45f }, 0,            0,       5,          16,     {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.45f,0.0f,0.0f}, {0.0f,0.0f,0.6f}},
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.45f },  0,            0,       5,          16,     {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.45f,0.0f,0.0f}, {0.0f,0.0f,0.6f}},
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f },   0,            0,       20,         16,     {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.75f,0.0f,0.0f}, {0.0f,0.0f,0.6f}},
		{{ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f },   10,           10,      5,          22,     {0.0f,0.0f,0.0f}, {-0.1f,0.0f,0.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.56f,0.0f,0.0f}, {0.0f,0.0f,0.0f}},
		//{{ 0.0f,0.8f,0.0f }, { 2.8f,0.0f,-1.57f }, 17,           0,       5,          20,          {0.0f,0.0f,0.0f}, {-0.36f,0.0f,0.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {-0.8f,0.0f,0.0f}, {0.0f,0.0f,0.6f}},
	}
};

void Player::BehaviorAirAttackUpdate()
{
	//コンボ上限に達していない
	if (workAttack_.comboIndex < airComboNum - 1)
	{
		if (input_->IsControllerConnected())
		{
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
			{
				//コンボ有効
				workAttack_.comboNext = true;
			}
		}
	}

	//攻撃の合計時間
	uint32_t totalTime = kConstAirAttacks_[workAttack_.comboIndex].anticipationTime + kConstAirAttacks_[workAttack_.comboIndex].chargeTime + kConstAirAttacks_[workAttack_.comboIndex].swingTime + kConstAirAttacks_[workAttack_.comboIndex].recoveryTime;
	//既定の時間経過で通常行動に戻る
	if (++workAttack_.attackParameter >= totalTime)
	{
		//コンボ継続なら次のコンボに進む
		if (workAttack_.comboNext)
		{
			//コンボ継続フラグをリセット
			workAttack_.comboNext = false;
			workAttack_.attackParameter = 0;
			workAttack_.comboIndex++;
			if (workAttack_.comboIndex >= 0 && workAttack_.comboIndex <= ComboNum - 1)
			{
				workAttack_.translation = kConstAirAttacks_[workAttack_.comboIndex].startPosition;
				workAttack_.rotation = kConstAirAttacks_[workAttack_.comboIndex].startRotation;
			}
			weapon_->SetIsAttack(false);

			if (input_->IsControllerConnected())
			{
				//しきい値
				const float threshold = 0.7f;

				//移動フラグ
				bool isRotation = false;

				//移動量
				Vector3 direction = {
					input_->GetLeftStickX(),
					0.0f,
					input_->GetLeftStickY(),
				};

				//スティックの押し込みが遊び範囲を超えていたら移動フラグをtrueにする
				if (Mathf::Length(direction) > threshold)
				{
					isRotation = true;
				}

				//スティックによる入力がある場合
				if (isRotation)
				{
					//移動量に速さを反映
					direction = Mathf::Normalize(direction);

					//移動ベクトルをカメラの角度だけ回転する
					Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
					direction = Mathf::TransformNormal(direction, rotateMatrix);

					//回転
					Rotate(direction);
				}
			}
		}
		//コンボ継続でないなら攻撃を終了してルートビヘイビアに戻る
		else
		{
			behaviorRequest_ = Behavior::kRoot;
			weapon_->SetIsAttack(false);
		}
	}

	//移動フラグ
	bool isMove = true;

	//ボスの座標を取得
	Vector3 targetPosition = GameObjectManager::GetInstance()->GetGameObject<Boss>("Boss")->GetWorldPosition();

	//差分ベクトルを計算
	Vector3 sub = targetPosition - GetWorldPosition();

	//Y軸は必要ないので0にする
	sub.y = 0.0f;

	//距離を計算
	float distance = Mathf::Length(sub);

	//閾値
	float threshold = 16.0f;

	//ボスとの距離が閾値より小さかったらボスの方向に回転させる
	if (distance < threshold || lockOn_->ExistTarget())
	{
		//回転
		Rotate(sub);

		if (distance < 6.0f)
		{
			//アニメーションの移動フラグをfalseにする
			isMove = false;
		}
	}


	switch (workAttack_.comboIndex)
	{
	case 0:
		AirAttackAnimation(isMove);
		damage_ = 8;
		break;
	case 1:
		AirAttackAnimation(isMove);
		damage_ = 8;
		break;
	case 2:
		AirAttackAnimation(isMove);
		damage_ = 5;
		break;
	case 3:
		AirAttackAnimation(isMove);
		damage_ = 20;
		break;
	}

	weapon_->SetTranslation(workAttack_.translation);
	weapon_->SetRotation(workAttack_.rotation);
}

void Player::AirAttackAnimation(bool isMove)
{
	//総合時間
	uint32_t totalTime = kConstAirAttacks_[workAttack_.comboIndex].anticipationTime + kConstAirAttacks_[workAttack_.comboIndex].chargeTime + kConstAirAttacks_[workAttack_.comboIndex].swingTime + kConstAirAttacks_[workAttack_.comboIndex].recoveryTime;

	//現在のコンボの情報を取得
	uint32_t anticipationTime = 0, chargeTime = 0, swingTime = 0, recoveryTime = 0;

	//移動速度
	velocity_ = kConstAirAttacks_[workAttack_.comboIndex].velocity;
	velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

	//配列外参照を防ぐ
	if (workAttack_.comboIndex >= 0 && workAttack_.comboIndex <= ComboNum - 1)
	{
		anticipationTime = kConstAirAttacks_[workAttack_.comboIndex].anticipationTime;
		chargeTime = anticipationTime + kConstAirAttacks_[workAttack_.comboIndex].chargeTime;
		swingTime = chargeTime + kConstAirAttacks_[workAttack_.comboIndex].swingTime;
		recoveryTime = swingTime + kConstAirAttacks_[workAttack_.comboIndex].recoveryTime;
	}

	//振りかぶりの処理
	if (workAttack_.attackParameter < anticipationTime)
	{
		workAttack_.translation += kConstAirAttacks_[workAttack_.comboIndex].anticipationSpeed;
		workAttack_.rotation += kConstAirAttacks_[workAttack_.comboIndex].anticipationRotateSpeed;
		if (isMove)
		{
			worldTransform_.translation_ += velocity_;
		}
	}

	//チャージの処理
	if (workAttack_.attackParameter >= anticipationTime && workAttack_.attackParameter < chargeTime)
	{
		workAttack_.translation += kConstAirAttacks_[workAttack_.comboIndex].chargeSpeed;
		workAttack_.rotation += kConstAirAttacks_[workAttack_.comboIndex].chargeRotateSpeed;
		if (isMove)
		{
			worldTransform_.translation_ += velocity_;
		}
	}

	//攻撃振りの処理
	if (workAttack_.attackParameter >= chargeTime && workAttack_.attackParameter < swingTime)
	{
		//移動処理
		workAttack_.translation += kConstAirAttacks_[workAttack_.comboIndex].swingSpeed;
		workAttack_.rotation += kConstAirAttacks_[workAttack_.comboIndex].swingRotateSpeed;
		if (isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//衝突判定をつける処理
		uint32_t collisionTime = swingTime - chargeTime;
		int collisionInterval = collisionTime / 4;
		if (workAttack_.attackParameter % collisionInterval == 0)
		{
			weapon_->SetIsAttack(true);
			//オーディオ再生
			if (!isSwishPlayed_)
			{
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
				isSwishPlayed_ = true;
			}
		}
		else
		{
			weapon_->SetIsAttack(false);
			isSwishPlayed_ = false;
		}
	}

	//硬直時間
	if (workAttack_.attackParameter >= swingTime && workAttack_.attackParameter < recoveryTime)
	{
		weapon_->SetIsAttack(false);
		isSwishPlayed_ = false;
	}

	//入力受付時間
	if (workAttack_.attackParameter >= recoveryTime && workAttack_.attackParameter < totalTime)
	{
		if (input_->IsControllerConnected())
		{
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
			{
				//コンボ有効
				workAttack_.attackParameter = totalTime;
			}
		}
	}
}

void Player::BehaviorKnockBackInitialize()
{

}

void Player::BehaviorKnockBackUpdate()
{
	const float kGravityAcceleration = 0.05f;
	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	knockBackVelocity_ += accelerationVector;
	worldTransform_.translation_ += knockBackVelocity_;

	if (worldTransform_.translation_.y <= 1.0f)
	{
		behaviorRequest_ = Behavior::kRoot;
		worldTransform_.translation_.y = 1.0f;
	}
}

void Player::BehaviorRapidApproachInitialize()
{
	const float kRapidApproachSpeed = 2.0f;
	targetPosition_ = GameObjectManager::GetInstance()->GetGameObject<Boss>("Boss")->GetWorldPosition();
	rapidApproachVelocity_ = Mathf::Normalize(targetPosition_ - worldTransform_.translation_) * kRapidApproachSpeed;
	rapidApproachVelocity_.y = 0.0f;
}

void Player::BehaviorRapidApproachUpdate()
{
	worldTransform_.translation_ += rapidApproachVelocity_;
	Rotate(rapidApproachVelocity_);

	if (Mathf::Length(targetPosition_ - worldTransform_.translation_) <= 6.0f)
	{
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::Rotate(const Vector3& v)
{
	Vector3 vector = Mathf::Normalize(v);
	Vector3 cross = Mathf::Normalize(Mathf::Cross({ 0.0f,0.0f,1.0f }, vector));
	float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, vector);
	destinationQuaternion_ = Mathf::MakeRotateAxisAngleQuaternion(cross, std::acos(dot));
}

const uint32_t Player::GetAttackTime() const
{
	//総合時間
	uint32_t attackTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime;

	return attackTime;
}