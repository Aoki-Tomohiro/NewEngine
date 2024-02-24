#include "Missile.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Application/Src/Object/Player/Player.h"

void Missile::Initialize(const Vector3& position, const Vector3& velocity)
{
	//モデルの生成
	model_.reset(ModelManager::CreateFromOBJ("Cube", Opaque));
	model_->SetEnableLighting(true);

	//速度の初期化
	velocity_ = velocity;

	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = { 0.6f,0.6f,0.6f };

	AABB aabb = {
		.min{-worldTransform_.scale_.x,-worldTransform_.scale_.y,-worldTransform_.scale_.z,},
		.max{worldTransform_.scale_.x,worldTransform_.scale_.y,worldTransform_.scale_.z}
	};
	SetAABB(aabb);
	SetCollisionAttribute(kCollisionAttributeMissile);
	SetCollisionMask(kCollisionMaskMissile);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);

	//音声データ読み込み
	audioHandle_ = Audio::GetInstance()->SoundLoadWave("Application/Resources/Sounds/Explosion.wav");
}

void Missile::Update()
{
	//目標への差分ベクトルを計算
	Vector3 targetPosition = GameObjectManager::GetInstance()->GetGameObject<Player>("Player")->GetWorldPosition();
	Vector3 sub = targetPosition - worldTransform_.translation_;

	//距離を計算
	float distance = Mathf::Length(sub);

	//正規化
	sub = Mathf::Normalize(sub);
	velocity_ = Mathf::Normalize(velocity_);

	//媒介変数の更新
	if (trackingParameter_ < 0.1f) {
		trackingParameter_ += 1.0f / (60.0f * 10.0f);
	}

	//追尾タイマーを進める
	if (trackingTimer_ != kTrackingTime)
	{
		trackingTimer_++;
	}

	//一定時間追尾するようにする
	if (trackingTimer_ >= kTrackingTime)
	{
		isTrackingComplete_ = true;
	}

	//追捕終了していなかったら
	if (!isTrackingComplete_)
	{
		velocity_ = Mathf::Slerp(velocity_, sub, trackingParameter_);
	}

	//移動処理
	const float kSpeed = 0.6f;
	velocity_ *= kSpeed;
	worldTransform_.translation_ += velocity_;

	//回転処理
	worldTransform_.rotation_.x += 0.2f;
	worldTransform_.rotation_.y += 0.2f;

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();

	//フィールド外に出たら死亡フラグを立てる
	if (worldTransform_.translation_.x <= -100.0f || worldTransform_.translation_.x >= 100.0f || worldTransform_.translation_.y <= 1.0f || worldTransform_.translation_.z <= -100.0f || worldTransform_.translation_.z >= 100.0f)
	{
		isDead_ = true;
		Audio::GetInstance()->SoundPlayWave(audioHandle_, false, 0.5f);
	}
}

void Missile::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}

void Missile::OnCollision(Collider* collider)
{
	isDead_ = true;
	Audio::GetInstance()->SoundPlayWave(audioHandle_, false, 0.5f);
}

const Vector3 Missile::GetWorldPosition() const
{
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}