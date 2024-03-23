#include "Player.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Application/Src/Object/LockOn/LockOn.h"
#include <numbers>

void Player::Initialize()
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_.z = -20.0f;
	//worldTransform_.scale_ = { 0.8f,0.8f,0.8f };

	//パーツのワールドトランスフォームの初期化
	for (uint32_t i = 0; i < kCountOfParts; ++i)
	{
		worldTransforms[i].Initialize();
	}
	worldTransforms[kHead].translation_ = { 0.0f,1.85f,0.0f };
	worldTransforms[kHead].rotation_ = { 0.0f,0.0f,0.0f };
	worldTransforms[kL_Arm].translation_ = { -0.7f,1.8f,0.0f };
	worldTransforms[kL_Arm].rotation_ = { 0.0f,0.0f,0.0f };
	worldTransforms[kR_Arm].translation_ = { 0.7f,1.8f,0.0f };
	worldTransforms[kR_Arm].rotation_ = { 0.0f,0.0f,0.0f };

	//親子付け
	worldTransforms[kBody].parent_ = &worldTransform_;
	worldTransforms[kHead].parent_ = &worldTransforms[kBody];
	worldTransforms[kL_Arm].parent_ = &worldTransforms[kBody];
	worldTransforms[kR_Arm].parent_ = &worldTransforms[kBody];

	//入力クラスのインスタンスを取得
	input_ = Input::GetInstance();

	//オーディオのインスタンスを取得
	audio_ = Audio::GetInstance();

	//武器の生成
	modelWeapon_.reset(ModelManager::CreateFromOBJ("Weapon", Opaque));
	modelWeapon_->SetEnableLighting(false);
	weapon_ = GameObjectManager::CreateGameObject<Weapon>();
	weapon_->SetModel(modelWeapon_.get());
	weapon_->SetParent(&worldTransform_);

	//スプライトの生成
	TextureManager::Load("HpBar.png");
	spriteHpBar_.reset(Sprite::Create("HpBar.png", { 80.0f,32.0f }));
	spriteHpBar_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	TextureManager::Load("HpBarFrame.png");
	spriteHpBarFrame_.reset(Sprite::Create("HpBarFrame.png", { 79.0f,31.0f }));
	spriteHpBarFrame_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	damageSprite_.reset(Sprite::Create("white.png", { 0.0f,0.0f }));
	damageSprite_->SetColor(damageSpriteColor_);
	damageSprite_->SetSize({ 1280.0f,720.0f });

	//パーティクルシステムの初期化
	particleModel_.reset(ModelManager::CreateFromOBJ("Cube", Opaque));
	particleSystem_ = ParticleManager::Create("Dash");
	particleSystem_->SetModel(particleModel_.get());
	particleSystem_->SetIsBillBoard(false);

	//オーディオの読み込み
	swishAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Swish.wav");
	damageAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Damage.wav");
	dashAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Dash.wav");
	jumpAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Jump.wav");

	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributePlayer);
	SetCollisionMask(kCollisionMaskPlayer);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);
}

void Player::Update()
{
	//ダッシュのクールタイム
	if (workDash_.coolTime != workDash_.dashCoolTime)
	{
		workDash_.coolTime++;
	}

	//ヒットフラグのリセット
	isHit_ = false;

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
		case Behavior::kGuard:
			BehaviorGuardInitialize();
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
	case Behavior::kGuard:
		BehaviorGuardUpdate();
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
	for (uint32_t i = 0; i < kCountOfParts; ++i)
	{
		worldTransforms[i].UpdateMatrixFromEuler();
	}

	//無敵時間の処理
	if (workInvincible_.invincibleFlag)
	{
		if (++workInvincible_.invincibleTimer > workInvincible_.kInvincibleTime)
		{
			workInvincible_.invincibleFlag = false;
		}

		//ダメージスプライトを徐々に透明にする
		damageSpriteColor_.w = 0.1f * (1.0f - (static_cast<float>(workInvincible_.invincibleTimer) / workInvincible_.kInvincibleTime));

		//ダメージスプライトの色を設定
		damageSprite_->SetColor(damageSpriteColor_);
	}

	//HPバーの処理
	hp_ = (hp_ <= 0.0f) ? 0.0f : hp_;
	hpBarSize_ = { (hp_ / kMaxHP) * 480.0f,16.0f };
	spriteHpBar_->SetSize(hpBarSize_);
}

void Player::Draw(const Camera& camera)
{
	//プレイヤーのモデルの描画
	for (uint32_t i = 0; i < kCountOfParts; ++i)
	{
		models_[i]->Draw(worldTransforms[i], camera);
	}

	//武器の描画
	if (behavior_ == Behavior::kAttack || behavior_ == Behavior::kAirAttack || behavior_ == Behavior::kGuard)
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

	if (workInvincible_.invincibleFlag)
	{
		damageSprite_->Draw();
	}
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
			if (behavior_ != Behavior::kDash && behavior_ != Behavior::kKnockBack)
			{
				//ノックバック状態にする
				Vector3 kKnockBackSpeed = { 0.0f,0.4f,0.4f };
				knockBackVelocity_ = Mathf::TransformNormal(kKnockBackSpeed, boss->GetWorldTransform().matWorld_);
				behaviorRequest_ = Behavior::kKnockBack;

				//無敵状態でなければ
				if (!workInvincible_.invincibleFlag)
				{
					//HPを減らす
					float damage = boss->GetDamage();
					if (behavior_ == Behavior::kGuard)
					{
						damage *= 0.4f;
					}
					hp_ -= damage;

					//無敵状態にする
					workInvincible_.invincibleFlag = true;
					workInvincible_.invincibleTimer = 0;

					//SEを再生
					audio_->SoundPlayWave(damageAudioHandle_, false, 0.5f);

					//ダメージスプライトのアルファ値を設定
					damageSpriteColor_.w = 0.5f;

					//ヒットフラグを立てる
					isHit_ = true;
				}
			}
		}
	}

	//ミサイルの衝突判定
	if (collider->GetCollisionAttribute() == kCollisionAttributeMissile)
	{
		if (behavior_ != Behavior::kDash)
		{
			//無敵状態でなければ
			if (!workInvincible_.invincibleFlag)
			{
				//HPを減らす
				float damage = 6.0f;
				if (behavior_ == Behavior::kGuard)
				{
					damage *= 0.4f;
				}
				hp_ -= damage;

				//無敵状態にする
				workInvincible_.invincibleFlag = true;
				workInvincible_.invincibleTimer = 0;

				//SEを再生
				audio_->SoundPlayWave(damageAudioHandle_, false, 0.5f);

				//ダメージスプライトのアルファ値を設定
				damageSpriteColor_.w = 0.5f;

				//ヒットフラグを立てる
				isHit_ = true;
			}
		}
	}

	//レーザーの衝突判定
	if (collider->GetCollisionAttribute() == kCollisionAttributeLaser)
	{
		if (behavior_ != Behavior::kDash)
		{
			//無敵状態でなければ
			if (!workInvincible_.invincibleFlag)
			{
				//HPを減らす
				float damage = 10.0f;
				if (behavior_ == Behavior::kGuard)
				{
					damage *= 0.4f;
				}
				hp_ -= damage;

				//無敵状態にする
				workInvincible_.invincibleFlag = true;
				workInvincible_.invincibleTimer = 0;

				//SEを再生
				audio_->SoundPlayWave(damageAudioHandle_, false, 0.5f);

				//ダメージスプライトのアルファ値を設定
				damageSpriteColor_.w = 0.5f;

				//ヒットフラグを立てる
				isHit_ = true;
			}
		}
	}
}

const Vector3 Player::GetWorldPosition() const
{
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1] + 1.0f;
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}

const uint32_t Player::GetAttackTotalTime() const
{
	//攻撃の合計時間
	uint32_t attackTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime + kConstAttacks_[workAttack_.comboIndex].chargeTime + kConstAttacks_[workAttack_.comboIndex].swingTime;
	if (isDashAttack_)
	{
		attackTime = kConstAttacks_[4].anticipationTime + kConstAttacks_[4].chargeTime + kConstAttacks_[4].swingTime;
	}

	return attackTime;
}

const uint32_t Player::GetAirAttackTotalTime() const
{
	//攻撃の合計時間
	uint32_t attackTime = kConstAirAttacks_[workAttack_.comboIndex].anticipationTime + kConstAirAttacks_[workAttack_.comboIndex].chargeTime + kConstAirAttacks_[workAttack_.comboIndex].swingTime;

	return attackTime;
}

void Player::BehaviorRootInitialize()
{
	gravity_ = { 0.0f,0.0f,0.0f };
	if (!particleSystem_->GetParticleEmitter("Move"))
	{
		Vector3 translation = { worldTransforms[kBody].matWorld_.m[3][0], 0.0f ,worldTransforms[kBody].matWorld_.m[3][2] };
		ParticleEmitter* emitter = ParticleEmitterBuilder()
			.SetDeleteTime(60)
			.SetEmitterName("Move")
			.SetPopArea({ -0.5f,0.0f,-0.5f }, { 0.5f,0.0f,0.5f })
			.SetPopAzimuth(0.0f, 0.0f)
			.SetPopColor({ 0.1f,0.1f,0.1f,1.0f }, { 0.1f,0.1f,0.1f,1.0f })
			.SetPopCount(1)
			.SetPopElevation(0.0f, 0.0f)
			.SetPopFrequency(0.01f)
			.SetPopLifeTime(0.2f, 0.4f)
			.SetPopQuaternion(destinationQuaternion_)
			.SetPopRotation({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetPopScale({ 0.1f,0.1f,0.1f }, { 0.2f,0.2f,0.2f })
			.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetTranslation(translation)
			.Build();
		particleSystem_->AddParticleEmitter(emitter);
	}
	worldTransforms[kBody].rotation_.x = 0.0f;
	worldTransforms[kBody].rotation_.y = 0.0f;
	weapon_->SetTranslation({ 0.0f,0.0f,0.0f });
	weapon_->SetRotation({ 0.0f,0.0f,0.0f });
}

void Player::BehaviorRootUpdate()
{
	//移動処理
	const float speed = 0.6f;
	Move(speed);

	//移動しているときにアニメーションをする
	if (velocity_ != Vector3{ 0.0f,0.0f,0.0f })
	{
		MoveAnimation();

		//パーティクルの座標を更新
		Vector3 translation = { worldTransforms[kBody].matWorld_.m[3][0], 0.0f ,worldTransforms[kBody].matWorld_.m[3][2] };
		ParticleEmitter* emitter = particleSystem_->GetParticleEmitter("Move");
		if (emitter)
		{
			emitter->SetPopCount(1);
			emitter->SetTranslation(translation);
		}
		//エミッターが消えてたら再生成
		else
		{
			ParticleEmitter* emitter = ParticleEmitterBuilder()
				.SetDeleteTime(60)
				.SetEmitterName("Move")
				.SetPopArea({ -0.5f,0.0f,-0.5f }, { 0.5f,0.0f,0.5f })
				.SetPopAzimuth(0.0f, 0.0f)
				.SetPopColor({ 0.1f,0.1f,0.1f,1.0f }, { 0.1f,0.1f,0.1f,1.0f })
				.SetPopCount(1)
				.SetPopElevation(0.0f, 0.0f)
				.SetPopFrequency(0.01f)
				.SetPopLifeTime(0.2f, 0.4f)
				.SetPopQuaternion(destinationQuaternion_)
				.SetPopRotation({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
				.SetPopScale({ 0.1f,0.1f,0.1f }, { 0.2f,0.2f,0.2f })
				.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
				.SetTranslation(translation)
				.Build();
			particleSystem_->AddParticleEmitter(emitter);
		}
	}
	else
	{
		moveAnimationWork_.startRotation_ = 0.0f;
		worldTransforms[kL_Arm].rotation_.x = 0.0f;
		worldTransforms[kR_Arm].rotation_.x = 0.0f;

		//パーティクルを出さないようにする
		ParticleEmitter* emitter = particleSystem_->GetParticleEmitter("Move");
		if (emitter)
		{
			emitter->SetPopCount(0);
		}
	}

	//地面にいなかったら落下する
	if (worldTransform_.translation_.y >= 0.0f)
	{
		const float kGravityAcceleration = 0.05f;
		gravity_.y -= kGravityAcceleration;
		worldTransform_.translation_ += gravity_;
	}

	//地面に埋まらないようにする
	if (worldTransform_.translation_.y < 0.0f)
	{
		worldTransform_.translation_.y = 0.0f;

		//地面に最初に触れたときにパーティクルを出す
		if (!isGroundHit_)
		{
			isGroundHit_ = true;
			//パーティクルの生成
			Vector3 translation = { worldTransforms[kBody].matWorld_.m[3][0], 0.0f ,worldTransforms[kBody].matWorld_.m[3][2] };
			for (float i = 0.0f; i < 6.32f; i += 0.1f)
			{
				Vector3 velocity = { 0.0f,0.0f,0.1f };
				Matrix4x4 rotateYMatrix = Mathf::MakeRotateYMatrix(i);
				velocity = Mathf::TransformNormal(velocity, rotateYMatrix);
				ParticleEmitter* emitter = ParticleEmitterBuilder()
					.SetDeleteTime(0.4f)
					.SetEmitterName("Jump")
					.SetPopAzimuth(0.0f, 0.0f)
					.SetPopColor({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f })
					.SetPopCount(1)
					.SetPopElevation(0.0f, 0.0f)
					.SetPopFrequency(2.0f)
					.SetPopLifeTime(0.2f, 0.2f)
					.SetPopRotation({ 0.0f,i,0.0f }, { 0.0f,i,0.0f })
					.SetPopScale({ 0.1f,0.1f,0.1f }, { 0.1f,0.1f,0.1f })
					.SetPopVelocity(velocity, velocity)
					.SetTranslation(translation)
					.Build();
				particleSystem_->AddParticleEmitter(emitter);
			}
		}
	}
	else
	{
		isGroundHit_ = false;
	}

	if (input_->IsControllerConnected())
	{
		//ダッシュ行動に変更
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER))
		{
			if (workDash_.coolTime == workDash_.dashCoolTime)
			{
				behaviorRequest_ = Behavior::kDash;
				//パーティクルを出さないようにする
				ParticleEmitter* emitter = particleSystem_->GetParticleEmitter("Move");
				if (emitter)
				{
					emitter->SetPopCount(0);
				}
			}
		}

		//ジャンプ行動に変更
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A))
		{
			if (worldTransform_.translation_.y == 0.0f)
			{
				behaviorRequest_ = Behavior::kJump;
				//パーティクルを出さないようにする
				ParticleEmitter* emitter = particleSystem_->GetParticleEmitter("Move");
				if (emitter)
				{
					emitter->SetPopCount(0);
				}
			}
		}

		//攻撃行動に変更
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
		{
			if (worldTransform_.translation_.y == 0.0f)
			{
				behaviorRequest_ = Behavior::kAttack;
				//パーティクルを出さないようにする
				ParticleEmitter* emitter = particleSystem_->GetParticleEmitter("Move");
				if (emitter)
				{
					emitter->SetPopCount(0);
				}
			}
		}

		//ガード状態に変更
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_LEFT_SHOULDER))
		{
			if (worldTransform_.translation_.y == 0.0f)
			{
				behaviorRequest_ = Behavior::kGuard;
				//パーティクルを出さないようにする
				ParticleEmitter* emitter = particleSystem_->GetParticleEmitter("Move");
				if (emitter)
				{
					emitter->SetPopCount(0);
				}
			}
		}
	}
}

void Player::MoveAnimation()
{
	//パラメータを加算
	moveAnimationWork_.parameter_ += 1.0f / 10.0f;

	//イージングが終わったら回転の向きを変える
	if (moveAnimationWork_.parameter_ > 1.0f)
	{
		moveAnimationWork_.rotationChange_ = true;
		moveAnimationWork_.parameter_ = 0.0f;
	}

	if (moveAnimationWork_.rotationChange_)
	{
		if (moveAnimationWork_.endRotation_ == 1.0f)
		{
			moveAnimationWork_.startRotation_ = 1.0f;
			moveAnimationWork_.endRotation_ = -1.0f;
		}
		else
		{
			moveAnimationWork_.startRotation_ = -1.0f;
			moveAnimationWork_.endRotation_ = 1.0f;
		}

		moveAnimationWork_.rotationChange_ = false;
	}

	//回転処理
	worldTransforms[kL_Arm].rotation_.x = moveAnimationWork_.startRotation_ + (moveAnimationWork_.endRotation_ - moveAnimationWork_.startRotation_) * Mathf::EaseInOutSine(moveAnimationWork_.parameter_);
	worldTransforms[kR_Arm].rotation_.x = -moveAnimationWork_.startRotation_ + (-moveAnimationWork_.endRotation_ - -moveAnimationWork_.startRotation_) * Mathf::EaseInOutSine(moveAnimationWork_.parameter_);
}

void Player::BehaviorDashInitialize()
{
	workDash_.dashParameter = 0;
	workDash_.coolTime = 0;
	worldTransform_.quaternion_ = destinationQuaternion_;
	workDash_.backStep = false;
	workDash_.backStepRotation = 0.0f;
	audio_->SoundPlayWave(dashAudioHandle_, false, 0.5f);

	if (velocity_ != Vector3{ 0.0f,0.0f,0.0f })
	{
		//速さ
		float kSpeed = 1.0f;

		//移動量
		if (input_->IsControllerConnected())
		{
			velocity_ = {
				input_->GetLeftStickX(),
				0.0f,
				input_->GetLeftStickY(),
			};
		}

		//移動量に速さを反映
		velocity_ = Mathf::Normalize(velocity_) * kSpeed;

		//移動ベクトルをカメラの角度だけ回転する
		Matrix4x4 rotateMatrix = Mathf::MakeRotateYMatrix(camera_->rotation_.y);
		velocity_ = Mathf::TransformNormal(velocity_, rotateMatrix);

		//回転処理
		Vector3 nVelocity = Mathf::Normalize(velocity_);
		Vector3 axis = Mathf::Normalize(Mathf::Cross({ 0.0f,1.0f,0.0f }, nVelocity));
		Quaternion rotationQuaternion = Mathf::MakeRotateAxisAngleQuaternion(axis, 0.2f);
		destinationQuaternion_ = rotationQuaternion * destinationQuaternion_;

		//パーティクルの生成
		Vector3 translation = { worldTransforms[kBody].matWorld_.m[3][0],worldTransforms[kBody].matWorld_.m[3][1] ,worldTransforms[kBody].matWorld_.m[3][2] };
		ParticleEmitter* emitter = ParticleEmitterBuilder()
			.SetDeleteTime(1.0f / 60.0f * 10)
			.SetEmitterName("Dash")
			.SetPopArea({ -0.8f,-0.8f,-0.8f },{ 0.8f,0.8f,0.8f })
			.SetPopAzimuth(0.0f, 0.0f)
			.SetPopColor({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f })
			.SetPopCount(10)
			.SetPopElevation(0.0f, 0.0f)
			.SetPopFrequency(1.0f / 60.0f)
			.SetPopLifeTime(0.2f, 0.4f)
			.SetPopQuaternion(worldTransform_.quaternion_)
			.SetPopScale({ 0.01f,0.01f,0.6f }, { 0.01f,0.01f,0.8f })
			.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetTranslation(translation)
			.Build();
		particleSystem_->AddParticleEmitter(emitter);
	}
	else
	{
		//バックステップのフラグを立てる
		workDash_.backStep = true;

		//入力がない場合後ろに下がるようにする
		velocity_ = { 0.0f,0.2f,-1.0f };

		//移動ベクトルをプレイヤーの角度だけ回転する
		velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);
	}
}

void Player::BehaviorDashUpdate()
{
	//移動
	worldTransform_.translation_ += velocity_;

	//バックステップしていなかったらパーティクルの位置を更新
	if (!workDash_.backStep)
	{
		Vector3 translation = { worldTransforms[kBody].matWorld_.m[3][0],worldTransforms[kBody].matWorld_.m[3][1] ,worldTransforms[kBody].matWorld_.m[3][2] };
		std::list<ParticleEmitter*> emitters = particleSystem_->GetParticleEmitters("Dash");
		for (ParticleEmitter* emitter : emitters)
		{
			emitter->SetTranslation(translation);
		}
	}
	//バックステップだったらアニメーションを挟む
	else
	{
		BackStepAnimation();
	}

	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X) && !workDash_.backStep)
		{
			isDashAttack_ = true;
			behaviorRequest_ = Behavior::kAttack;
		}
	}

	//規定の時間経過で通常行動に戻る
	const float dashTime = 10;
	if (++workDash_.dashParameter >= dashTime)
	{
		behaviorRequest_ = Behavior::kRoot;

		//元の姿勢に戻す
		if (!workDash_.backStep)
		{
			Rotate(velocity_);
			std::list<ParticleEmitter*> emitters = particleSystem_->GetParticleEmitters("Dash");
			for (ParticleEmitter* emitter : emitters)
			{
				emitter->SetPopCount(0);
			}
		}
	}
}

void Player::BackStepAnimation()
{
	workDash_.backStepRotation += std::numbers::pi_v<float> * 2.0f / 10.0f;
	Vector3 nVelocity = Mathf::Normalize(velocity_);
	Vector3 axis = Mathf::Normalize(Mathf::Cross({ 0.0f,1.0f,0.0f }, nVelocity));
	Quaternion rotationQuaternion = Mathf::MakeRotateAxisAngleQuaternion(axis, workDash_.backStepRotation);
	worldTransform_.quaternion_ = rotationQuaternion * destinationQuaternion_;
}

void Player::BehaviorJumpInitialize()
{
	const float kJumpFirstSpeed = 0.75f;
	velocity_.y = kJumpFirstSpeed;
	isGroundHit_ = false;
	audio_->SoundPlayWave(jumpAudioHandle_, false, 0.2f);
}

void Player::BehaviorJumpUpdate()
{
	//ジャンプ処理
	const float kGravityAcceleration = 0.05f;
	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector;
	worldTransform_.translation_ += velocity_;

	if (input_->IsControllerConnected())
	{
		//空中攻撃行動に変更
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
		{
			behaviorRequest_ = Behavior::kAirAttack;
		}
	}

	//通常状態に変更
	if (worldTransform_.translation_.y <= 0.0f)
	{
		behaviorRequest_ = Behavior::kRoot;
		worldTransform_.translation_.y = 0.0f;
	}
}

//コンボ定数表
const std::array<Player::ConstAttack, Player::ComboNum + 1> Player::kConstAttacks_ = 
{
	{
		////振りかぶり、攻撃前硬直、攻撃振り時間、硬直、振りかぶり速度、攻撃前硬直速度、攻撃振り速度
		//{15,        10,       15,        0,   0.2f,        0.0f,        0.0f },
		//{0,         0,        20,        0,   0.0f,        0.0f,        0.15f},
		//{15,        10,       15,        30,  0.2f,        0.0f,        0.0f },

		////振りかぶり、攻撃前硬直、攻撃振り時間、硬直、振りかぶり速度、攻撃前硬直速度、攻撃振り速度
		//{0,         0,        5,         16,  0.2f,        0.0f,        0.0f },
		//{0,         0,        5,         16,  0.0f,        0.0f,        0.15f},
		//{0,         0,        20,        16,  0.2f,        0.0f,        0.0f },

		//振りかぶり、攻撃前硬直、攻撃振り時間、硬直、振りかぶり速度、攻撃前硬直速度、攻撃振り速度
		{5,         0,        5,         14,  0.2f,        0.0f,        0.0f },
		{5,         0,        5,         14,  0.0f,        0.0f,        0.15f},
		{5,         0,        25,        14,  0.2f,        0.0f,        0.0f },
		{10,        10,       5,         20,  0.2f,        0.0f,        0.0f },
		{5,         0,        25,        14,  0.2f,        0.0f,        0.0f },
	}
};

void Player::BehaviorAttackInitialize()
{
	//攻撃用の変数の初期化
	workAttack_.attackParameter = 0;
	workAttack_.comboIndex = 0;
	workAttack_.comboNext = false;
	workAttack_.inComboPhase = 0;
	workAttack_.inComboPhaseAttackParameter = 0.0f;

	if (!isDashAttack_)
	{
		//パーツの初期値を設定
		worldTransforms[kL_Arm].rotation_.x = 0.0f;
		worldTransforms[kR_Arm].rotation_.x = 0.0f;

		//速度の設定
		velocity_ = { 0.0f,0.0f,0.6f };
		velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

		//武器の初期値を設定
		Vector3 weaponTranslation = { 0.0f,1.5f,0.0f };
		Vector3 weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
		weapon_->SetTranslation(weaponTranslation);
		weapon_->SetRotation(weaponRotation);

		//ダメージを設定
		workAttack_.damage = 8;
	}
	else
	{
		//パーツの初期値を設定
		worldTransforms[kL_Arm].rotation_.x = 0.0f;
		worldTransforms[kR_Arm].rotation_.x = 0.0f;

		//武器の初期値を設定
		Vector3 weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
		Vector3 weaponTranslation = { 0.0f,1.5f,0.0f };
		weapon_->SetTranslation(weaponTranslation);
		weapon_->SetRotation(weaponRotation);

		//ダメージを設定
		workAttack_.damage = 5;
	}
}

void Player::BehaviorAttackUpdate()
{
	//コンボ上限に達していない
	if (workAttack_.comboIndex < ComboNum - 1)
	{
		if (input_->IsControllerConnected())
		{
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X) && !workAttack_.isAttack)
			{
				//コンボ有効
				workAttack_.comboNext = true;
				workAttack_.isAttack = true;
			}

			//ボタンを離したときに攻撃フラグをfalseにする
			if (!input_->IsPressButton(XINPUT_GAMEPAD_X))
			{
				workAttack_.isAttack = false;
			}
		}
	}

	//コンボの合計時間
	uint32_t anticipationTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime;
	uint32_t chargeTime = kConstAttacks_[workAttack_.comboIndex].chargeTime;
	uint32_t swingTime = kConstAttacks_[workAttack_.comboIndex].swingTime;
	uint32_t recoveryTime = kConstAttacks_[workAttack_.comboIndex].recoveryTime;
	uint32_t totalTime = anticipationTime + chargeTime + swingTime + recoveryTime;
	if (isDashAttack_)
	{
		anticipationTime = kConstAttacks_[4].anticipationTime;
		chargeTime = kConstAttacks_[4].chargeTime;
		swingTime = kConstAttacks_[4].swingTime;
		recoveryTime = kConstAttacks_[4].recoveryTime;
		totalTime = anticipationTime + chargeTime + swingTime + recoveryTime;
	}

	//規定の時間経過で通常状態に戻る
	if (++workAttack_.attackParameter > totalTime)
	{
		//コンボ継続なら次のコンボに進む
		if (workAttack_.comboNext)
		{
			//コンボ継続フラグをリセット
			workAttack_.comboNext = false;
			//攻撃の色々な変数をリセットする
			workAttack_.comboIndex++;
			workAttack_.inComboPhase = 0;
			workAttack_.attackParameter = 0;
			workAttack_.inComboPhaseAttackParameter = 0.0f;

			//コンボ切り替わりの瞬間だけ、スティック入力による方向転換を受け付ける
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

			//各パーツの角度などを次のコンボ用に初期化
			Vector3 weaponTranslation{};
			Vector3 weaponRotation{};
			switch (workAttack_.comboIndex)
			{
			case 0:
				if (!isDashAttack_)
				{
					//パーツの初期値を設定
					worldTransforms[kBody].rotation_.y = 0.0f;
					worldTransforms[kL_Arm].rotation_.x = 0.0f;
					worldTransforms[kR_Arm].rotation_.x = 0.0f;

					//速度の設定
					velocity_ = { 0.0f,0.0f,0.6f };
					velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

					//武器の初期値を設定
					weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
					weaponTranslation = { 0.0f,1.5f,0.0f };
					weapon_->SetTranslation(weaponTranslation);
					weapon_->SetRotation(weaponRotation);

					//ダメージを設定
					workAttack_.damage = 8;
				}
				else
				{
					//パーツの初期値を設定
					worldTransforms[kBody].rotation_.x = 0.0f;
					worldTransforms[kBody].rotation_.y = 0.0f;
					worldTransforms[kL_Arm].rotation_.x = 0.0f;
					worldTransforms[kR_Arm].rotation_.x = 0.0f;

					//武器の初期値を設定
					Vector3 weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
					Vector3 weaponTranslation = { 0.0f,1.5f,0.0f };
					weapon_->SetTranslation(weaponTranslation);
					weapon_->SetRotation(weaponRotation);

					//ダメージを設定
					workAttack_.damage = 5;
				}
				break;
			case 1:
				//パーツの初期値を設定
				worldTransforms[kBody].rotation_.y = 0.0f;
				worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float>;
				worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float>;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.6f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { 0.0f, 0.0f, 0.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 8;

				break;
			case 2:
				//パーツの初期値を設定
				worldTransforms[kBody].rotation_.y = 0.0f;
				worldTransforms[kL_Arm].rotation_.x = 0.0f;
				worldTransforms[kR_Arm].rotation_.x = 0.0f;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.6f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 5;

				break;
			case 3:
				//パーツの初期値を設定
				worldTransforms[kBody].rotation_.y = 0.0f;
				worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f;
				worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.0f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 30;

				break;
			}
		}
		//コンボ継続でないなら攻撃を終了してルートビヘイビアに戻る
		else
		{
			behaviorRequest_ = Behavior::kRoot;
			worldTransforms[kBody].rotation_ = { 0.0f,0.0f,0.0f };
			worldTransforms[kL_Arm].rotation_ = { 0.0f,0.0f,0.0f };
			worldTransforms[kR_Arm].rotation_ = { 0.0f,0.0f,0.0f };
		}
	}

	//攻撃アニメーション
	AttackAnimation();
}

void Player::AttackAnimation()
{
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

		//ボスとの距離が近かったら移動しないようにする
		isMove = (distance < 6.0f) ? false : true;
	}

	//各パラメーターの時間
	uint32_t anticipationTime = kConstAttacks_[workAttack_.comboIndex].anticipationTime;
	uint32_t chargeTime = kConstAttacks_[workAttack_.comboIndex].chargeTime;
	uint32_t swingTime = kConstAttacks_[workAttack_.comboIndex].swingTime;
	uint32_t recoveryTime = kConstAttacks_[workAttack_.comboIndex].recoveryTime;
	if (isDashAttack_)
	{
		anticipationTime = kConstAttacks_[4].anticipationTime;
		chargeTime = kConstAttacks_[4].chargeTime;
		swingTime = kConstAttacks_[4].swingTime;
		recoveryTime = kConstAttacks_[4].recoveryTime;
	}

	//コンボ段階によってモーションを分岐
	switch (workAttack_.comboIndex)
	{
	//攻撃1
	case 0:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		if (!isDashAttack_)
		{
			//振りかぶり
			if (workAttack_.inComboPhase == kAnticipation)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
				worldTransforms[kBody].rotation_.y = 0.0f + (-1.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				worldTransforms[kL_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				worldTransforms[kR_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				Vector3 weaponRotation = weapon_->GetRotation();
				weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (std::numbers::pi_v<float> / 2.0f - 1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				weapon_->SetRotation(weaponRotation);

				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhase++;
					workAttack_.inComboPhaseAttackParameter = 0.0f;
				}
			}

			//チャージ
			if (workAttack_.inComboPhase == kCharge)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhase++;
					workAttack_.inComboPhaseAttackParameter = 0.0f;
				}
			}

			//攻撃振り
			if (workAttack_.inComboPhase == kSwing)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
				worldTransforms[kBody].rotation_.y = -1.0f + (1.0f - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				Vector3 weaponRotation = weapon_->GetRotation();
				weaponRotation.x = (std::numbers::pi_v<float> / 2.0f - 1.0f) + ((std::numbers::pi_v<float> / 2.0f + 1.0f) - (std::numbers::pi_v<float> / 2.0f - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				weapon_->SetRotation(weaponRotation);

				//衝突判定をつける処理
				if (++workAttack_.collisionParameter % swingTime - 1 == 0)
				{
					weapon_->SetIsAttack(true);
					audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
				}
				else
				{
					weapon_->SetIsAttack(false);
				}

				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhase++;
					workAttack_.inComboPhaseAttackParameter = 0.0f;
					workAttack_.collisionParameter = 0;
					weapon_->SetIsAttack(false);
				}
			}

			//硬直
			if (workAttack_.inComboPhase == kRecovery)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhaseAttackParameter = 0.0f;
				}
			}
		}
		else
		{
			//移動処理
			if (workAttack_.inComboPhase != kRecovery && isMove)
			{
				worldTransform_.translation_ += velocity_;
			}

			//振りかぶり
			if (workAttack_.inComboPhase == kAnticipation)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
				worldTransforms[kBody].rotation_.y = 0.0f + (-1.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				worldTransforms[kL_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				worldTransforms[kR_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				Vector3 weaponRotation = weapon_->GetRotation();
				weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (std::numbers::pi_v<float> / 2.0f - 1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				weapon_->SetRotation(weaponRotation);

				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhase++;
					workAttack_.inComboPhaseAttackParameter = 0.0f;
				}
			}

			//チャージ
			if (workAttack_.inComboPhase == kCharge)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhase++;
					workAttack_.inComboPhaseAttackParameter = 0.0f;
				}
			}

			//攻撃振り
			if (workAttack_.inComboPhase == kSwing)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
				worldTransforms[kBody].rotation_.y = -1.0f + (std::numbers::pi_v<float> *8.0f + 1.0f - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				Vector3 weaponRotation = weapon_->GetRotation();
				weaponRotation.x = (std::numbers::pi_v<float> / 2.0f - 1.0f) + ((std::numbers::pi_v<float> *6.0f + std::numbers::pi_v<float> / 2.0f + 1.0f) - (std::numbers::pi_v<float> / 2.0f - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
				weapon_->SetRotation(weaponRotation);

				//衝突判定をつける処理
				if (++workAttack_.collisionParameter % (swingTime / 4) == 0)
				{
					weapon_->SetIsAttack(true);
					audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
				}
				else
				{
					weapon_->SetIsAttack(false);
				}

				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhase++;
					workAttack_.inComboPhaseAttackParameter = 0.0f;
					workAttack_.collisionParameter = 0;
					weapon_->SetIsAttack(false);
				}
			}

			//硬直
			if (workAttack_.inComboPhase == kRecovery)
			{
				workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
				if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
				{
					workAttack_.inComboPhaseAttackParameter = 0.0f;
					isDashAttack_ = false;
				}
			}
		}

		break;
	//攻撃2
	case 1:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> + (0.0f - -std::numbers::pi_v<float>) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> + (0.0f - -std::numbers::pi_v<float>) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = 0.0f + (std::numbers::pi_v<float> - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % swingTime - 1 == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
	//攻撃3
	case 2:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			worldTransforms[kBody].rotation_.y = 0.0f + (-1.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kL_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (std::numbers::pi_v<float> / 2.0f - 1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kBody].rotation_.y = -1.0f + (std::numbers::pi_v<float> * 8.0f + 1.0f - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = (std::numbers::pi_v<float> / 2.0f - 1.0f) + ((std::numbers::pi_v<float> * 6.0f + std::numbers::pi_v<float> / 2.0f + 1.0f) - (std::numbers::pi_v<float> / 2.0f - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % (swingTime / 4) == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
	//攻撃4
	case 3:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f + (-std::numbers::pi_v<float> - 1.0f - -std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f + (-std::numbers::pi_v<float> - 1.0f - -std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (-1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kL_Arm].rotation_.x = (-std::numbers::pi_v<float> - 1.0f) + (0.0f - (-std::numbers::pi_v<float> - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = (-std::numbers::pi_v<float> - 1.0f) + (0.0f - (-std::numbers::pi_v<float> - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = -1.0f + (std::numbers::pi_v<float> - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % swingTime - 1 == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
	}
}

const std::array<Player::ConstAttack, Player::ComboNum> Player::kConstAirAttacks_ =
{
	{
		////振りかぶり、攻撃前硬直、攻撃振り時間、硬直、振りかぶり速度、攻撃前硬直速度、攻撃振り速度
		//{15,        10,       15,        0,   0.2f,        0.0f,        0.0f },
		//{0,         0,        20,        0,   0.0f,        0.0f,        0.15f},
		//{15,        10,       15,        30,  0.2f,        0.0f,        0.0f },

		////振りかぶり、攻撃前硬直、攻撃振り時間、硬直、振りかぶり速度、攻撃前硬直速度、攻撃振り速度
		//{0,         0,        5,         16,  0.2f,        0.0f,        0.0f },
		//{0,         0,        5,         16,  0.0f,        0.0f,        0.15f},
		//{0,         0,        20,        16,  0.2f,        0.0f,        0.0f },

		//振りかぶり、攻撃前硬直、攻撃振り時間、硬直、振りかぶり速度、攻撃前硬直速度、攻撃振り速度
		{5,         0,        5,         14,  0.2f,        0.0f,        0.0f },
		{5,         0,        5,         14,  0.0f,        0.0f,        0.15f},
		{5,         0,        25,        14,  0.2f,        0.0f,        0.0f },
		{10,        10,       5,         20,  0.2f,        0.0f,        0.0f },
	}
};

void Player::BehaviorAirAttackInitialize()
{
	//攻撃用の変数の初期化
	workAttack_.attackParameter = 0;
	workAttack_.comboIndex = 0;
	workAttack_.comboNext = false;
	workAttack_.inComboPhase = 0;
	workAttack_.inComboPhaseAttackParameter = 0.0f;

	//パーツの初期値を設定
	worldTransforms[kL_Arm].rotation_.x = 0.0f;
	worldTransforms[kR_Arm].rotation_.x = 0.0f;

	//速度の設定
	velocity_ = { 0.0f,0.0f,0.6f };
	velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

	//武器の初期値を設定
	Vector3 weaponTranslation = { 0.0f,1.5f,0.0f };
	Vector3 weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
	weapon_->SetTranslation(weaponTranslation);
	weapon_->SetRotation(weaponRotation);

	//ダメージを設定
	workAttack_.damage = 8;
}

void Player::BehaviorAirAttackUpdate()
{
	//コンボ上限に達していない
	if (workAttack_.comboIndex < ComboNum - 1)
	{
		if (input_->IsControllerConnected())
		{
			//攻撃ボタンをトリガーしたら
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X) && !workAttack_.isAttack)
			{
				//コンボ有効
				workAttack_.comboNext = true;
				workAttack_.isAttack = true;
			}

			//ボタンを離したときに攻撃フラグをfalseにする
			if (!input_->IsPressButton(XINPUT_GAMEPAD_X))
			{
				workAttack_.isAttack = false;
			}
		}
	}

	//コンボの合計時間
	uint32_t anticipationTime = kConstAirAttacks_[workAttack_.comboIndex].anticipationTime;
	uint32_t chargeTime = kConstAirAttacks_[workAttack_.comboIndex].chargeTime;
	uint32_t swingTime = kConstAirAttacks_[workAttack_.comboIndex].swingTime;
	uint32_t recoveryTime = kConstAirAttacks_[workAttack_.comboIndex].recoveryTime;
	uint32_t totalTime = anticipationTime + chargeTime + swingTime + recoveryTime;

	//規定の時間経過で通常状態に戻る
	if (++workAttack_.attackParameter > totalTime)
	{
		//コンボ継続なら次のコンボに進む
		if (workAttack_.comboNext)
		{
			//コンボ継続フラグをリセット
			workAttack_.comboNext = false;
			//攻撃の色々な変数をリセットする
			workAttack_.comboIndex++;
			workAttack_.inComboPhase = 0;
			workAttack_.attackParameter = 0;
			workAttack_.inComboPhaseAttackParameter = 0.0f;

			//コンボ切り替わりの瞬間だけ、スティック入力による方向転換を受け付ける
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

			//各パーツの角度などを次のコンボ用に初期化
			Vector3 weaponTranslation{};
			Vector3 weaponRotation{};
			switch (workAttack_.comboIndex)
			{
			case 0:
				//パーツの初期値を設定
				worldTransforms[kL_Arm].rotation_.x = 0.0f;
				worldTransforms[kR_Arm].rotation_.x = 0.0f;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.6f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 8;

				break;
			case 1:
				//パーツの初期値を設定
				worldTransforms[kBody].rotation_.y = 0.0f;
				worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float>;
				worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float>;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.6f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { 0.0f, 0.0f, 0.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 8;

				break;
			case 2:
				//パーツの初期値を設定
				worldTransforms[kL_Arm].rotation_.x = 0.0f;
				worldTransforms[kR_Arm].rotation_.x = 0.0f;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.6f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, std::numbers::pi_v<float> / 2.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 5;

				break;
			case 3:
				//パーツの初期値を設定
				worldTransforms[kBody].rotation_.y = 0.0f;
				worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f;
				worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f;

				//速度の設定
				velocity_ = { 0.0f,0.0f,0.0f };
				velocity_ = Mathf::TransformNormal(velocity_, worldTransform_.matWorld_);

				//武器の初期値を設定
				weaponRotation = { std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f };
				weaponTranslation = { 0.0f,1.5f,0.0f };
				weapon_->SetTranslation(weaponTranslation);
				weapon_->SetRotation(weaponRotation);

				//ダメージを設定
				workAttack_.damage = 30;

				break;
			}
		}
		//コンボ継続でないなら攻撃を終了してルートビヘイビアに戻る
		else
		{
			behaviorRequest_ = Behavior::kRoot;
			worldTransforms[kBody].rotation_ = { 0.0f,0.0f,0.0f };
			worldTransforms[kL_Arm].rotation_ = { 0.0f,0.0f,0.0f };
			worldTransforms[kR_Arm].rotation_ = { 0.0f,0.0f,0.0f };
		}
	}

	//攻撃アニメーション
	AirAttackAnimation();
}

void Player::AirAttackAnimation()
{
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

		//ボスとの距離が近かったら移動しないようにする
		isMove = (distance < 6.0f) ? false : true;
	}

	//各パラメーターの時間
	uint32_t anticipationTime = kConstAirAttacks_[workAttack_.comboIndex].anticipationTime;
	uint32_t chargeTime = kConstAirAttacks_[workAttack_.comboIndex].chargeTime;
	uint32_t swingTime = kConstAirAttacks_[workAttack_.comboIndex].swingTime;
	uint32_t recoveryTime = kConstAirAttacks_[workAttack_.comboIndex].recoveryTime;

	//コンボ段階によってモーションを分岐
	switch (workAttack_.comboIndex)
	{
		//攻撃1
	case 0:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			worldTransforms[kBody].rotation_.y = 0.0f + (-1.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kL_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (std::numbers::pi_v<float> / 2.0f - 1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kBody].rotation_.y = -1.0f + (1.0f - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = (std::numbers::pi_v<float> / 2.0f - 1.0f) + ((std::numbers::pi_v<float> / 2.0f + 1.0f) - (std::numbers::pi_v<float> / 2.0f - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % swingTime - 1 == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
		//攻撃2
	case 1:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> +(0.0f - -std::numbers::pi_v<float>) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> +(0.0f - -std::numbers::pi_v<float>) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = 0.0f + (std::numbers::pi_v<float> -0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % swingTime - 1 == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
		//攻撃3
	case 2:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			worldTransforms[kBody].rotation_.y = 0.0f + (-1.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kL_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = 0.0f + (-std::numbers::pi_v<float> / 2.0f - 0.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (std::numbers::pi_v<float> / 2.0f - 1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kBody].rotation_.y = -1.0f + (std::numbers::pi_v<float> *8.0f + 1.0f - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = (std::numbers::pi_v<float> / 2.0f - 1.0f) + ((std::numbers::pi_v<float> *6.0f + std::numbers::pi_v<float> / 2.0f + 1.0f) - (std::numbers::pi_v<float> / 2.0f - 1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % (swingTime / 4) == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
		//攻撃4
	case 3:
		//移動処理
		if (workAttack_.inComboPhase != kRecovery && isMove)
		{
			worldTransform_.translation_ += velocity_;
		}

		//振りかぶり
		if (workAttack_.inComboPhase == kAnticipation)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)anticipationTime;
			worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f + (-std::numbers::pi_v<float> -1.0f - -std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f + (-std::numbers::pi_v<float> -1.0f - -std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = std::numbers::pi_v<float> / 2.0f + (-1.0f - std::numbers::pi_v<float> / 2.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//チャージ
		if (workAttack_.inComboPhase == kCharge)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)chargeTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		//攻撃振り
		if (workAttack_.inComboPhase == kSwing)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)swingTime;
			worldTransforms[kL_Arm].rotation_.x = (-std::numbers::pi_v<float> -1.0f) + (0.0f - (-std::numbers::pi_v<float> -1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			worldTransforms[kR_Arm].rotation_.x = (-std::numbers::pi_v<float> -1.0f) + (0.0f - (-std::numbers::pi_v<float> -1.0f)) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			Vector3 weaponRotation = weapon_->GetRotation();
			weaponRotation.x = -1.0f + (std::numbers::pi_v<float> - -1.0f) * Mathf::EaseInSine(workAttack_.inComboPhaseAttackParameter);
			weapon_->SetRotation(weaponRotation);

			//衝突判定をつける処理
			if (++workAttack_.collisionParameter % swingTime - 1 == 0)
			{
				weapon_->SetIsAttack(true);
				audio_->SoundPlayWave(swishAudioHandle_, false, 0.5f);
			}
			else
			{
				weapon_->SetIsAttack(false);
			}

			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhase++;
				workAttack_.inComboPhaseAttackParameter = 0.0f;
				workAttack_.collisionParameter = 0;
				weapon_->SetIsAttack(false);
			}
		}

		//硬直
		if (workAttack_.inComboPhase == kRecovery)
		{
			workAttack_.inComboPhaseAttackParameter += 1.0f / (float)recoveryTime;
			if (workAttack_.inComboPhaseAttackParameter >= 1.0f)
			{
				workAttack_.inComboPhaseAttackParameter = 0.0f;
			}
		}

		break;
	}
}

void Player::BehaviorKnockBackInitialize()
{
	worldTransforms[kBody].rotation_ = { 0.0f,0.0f,0.0f };
	worldTransforms[kL_Arm].rotation_ = { 0.0f,0.0f,0.0f };
	worldTransforms[kR_Arm].rotation_ = { 0.0f,0.0f,0.0f };
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

void Player::BehaviorGuardInitialize()
{
	worldTransforms[kL_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f;
	worldTransforms[kR_Arm].rotation_.x = -std::numbers::pi_v<float> / 2.0f;
	weapon_->SetTranslation({ 3.6f,1.4f,1.5f });
	weapon_->SetRotation({ 1.4f,-std::numbers::pi_v<float> / 2.0f, 0.0f });
}

void Player::BehaviorGuardUpdate()
{
	const float speed = 0.2f;
	Move(speed);

	//ボスの座標を取得
	Vector3 targetPosition = GameObjectManager::GetInstance()->GetGameObject<Boss>("Boss")->GetWorldPosition();

	//差分ベクトルを計算
	Vector3 sub = targetPosition - GetWorldPosition();

	//Y軸は必要ないので0にする
	sub.y = 0.0f;

	//ロックオン中ならボスの方向に向かせる
	if (lockOn_->ExistTarget())
	{
		//回転
		Rotate(sub);
	}

	if (input_->IsControllerConnected())
	{
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_X))
		{
			behaviorRequest_ = Behavior::kAttack;
		}

		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A))
		{
			behaviorRequest_ = Behavior::kJump;
		}

		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_SHOULDER))
		{
			behaviorRequest_ = Behavior::kDash;
		}

		if (!input_->IsPressButton(XINPUT_GAMEPAD_LEFT_SHOULDER))
		{
			behaviorRequest_ = Behavior::kRoot;
			worldTransforms[kL_Arm].rotation_.x = 0.0f;
			worldTransforms[kR_Arm].rotation_.x = 0.0f;
		}
	}
}

void Player::Move(const float speed)
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
		else
		{
			velocity_ = { 0.0f,0.0f,0.0f };
		}
	}
}

void Player::Rotate(const Vector3& v)
{
	Vector3 vector = Mathf::Normalize(v);
	Vector3 cross = Mathf::Normalize(Mathf::Cross({ 0.0f,0.0f,1.0f }, vector));
	float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, vector);
	destinationQuaternion_ = Mathf::MakeRotateAxisAngleQuaternion(cross, std::acos(dot));
}