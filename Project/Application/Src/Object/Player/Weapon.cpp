#include "Weapon.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Application/Src/Object/Boss/Boss.h"

void Weapon::Initialize()
{
	//オーディオのインスタンスを取得
	audio_ = Audio::GetInstance();

	worldTransform_.Initialize();
	worldTransform_.translation_.y = 0.8f;

	//衝突判定用のワールドトランスフォームの初期化
	worldTransformCollision_.Initialize();

	//テクスチャ読み込み
	TextureManager::Load("DefaultParticle.png");
	TextureManager::Load("ShockWave.png");

	//パーティクルの初期化
	particleSystem_ = ParticleManager::Create("Weapon");
	particleSystem_->SetTexture("DefaultParticle.png");
	shockWaveParticleSystem_ = ParticleManager::Create("ShockWave");
	shockWaveParticleSystem_->SetTexture("ShockWave.png");

	//オーディオ読み込み
	slashAudioHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/Slash.wav");

	//衝突属性を設定
	SetOBB(obbSize);
	SetCollisionAttribute(kCollisionAttributeWeapon);
	SetCollisionMask(kCollisionMaskWeapon);
	SetCollisionPrimitive(kCollisionPrimitiveOBB);
}

void Weapon::Update()
{
	//前のフレームの当たり判定のフラグを取得
	preOnCollision_ = onCollision_;
	onCollision_ = false;
	isHit_ = false;

	//当たり判定の位置を決める
	Vector3 offset{ 0.0f,0.0f,2.0f };
	offset = Mathf::TransformNormal(offset, worldTransform_.parent_->matWorld_);
	worldTransformCollision_.translation_ = worldTransform_.parent_->translation_;
	worldTransformCollision_.quaternion_ = worldTransform_.parent_->quaternion_;
	worldTransformCollision_.translation_ += offset;

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();
	worldTransformCollision_.UpdateMatrixFromQuaternion();

	//OBBのサイズを変更
	obbSize = {
		.center{worldTransformCollision_.translation_},
		.orientations{
			{worldTransformCollision_.matWorld_.m[0][0],worldTransformCollision_.matWorld_.m[0][1],worldTransformCollision_.matWorld_.m[0][2]},
			{worldTransformCollision_.matWorld_.m[1][0],worldTransformCollision_.matWorld_.m[1][1],worldTransformCollision_.matWorld_.m[1][2]},
			{worldTransformCollision_.matWorld_.m[2][0],worldTransformCollision_.matWorld_.m[2][1],worldTransformCollision_.matWorld_.m[2][2]},},
		.size{1.0f,1.0f,4.0f}
	};
	SetOBB(obbSize);
}

void Weapon::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}

void Weapon::DrawUI()
{

}

void Weapon::OnCollision(Collider* collider)
{
	onCollision_ = true;

	if (onCollision_ != preOnCollision_)
	{
		//ヒットフラグを立てる
		isHit_ = true;

		//オーディオ再生
		audio_->SoundPlayWave(slashAudioHandle_, false, 0.5f);

		//座標を決める
		Vector3 offset{ 0.0f,0.0f,4.0f };
		offset = Mathf::TransformNormal(offset, worldTransform_.parent_->matWorld_);
		Vector3 translation = GetWorldPosition() + offset;

		//武器のワールド座標を取得
		Vector3 worldPosition = {
			worldTransform_.matWorld_.m[3][0],
			worldTransform_.matWorld_.m[3][1],
			worldTransform_.matWorld_.m[3][2],
		};
		//ボスのワールド座標を取得
		Vector3 bossPosition = gameObjectManager_->GetGameObject<Boss>("Boss")->GetWorldPosition();

		//差分ベクトルを計算
		Vector3 velocity = worldPosition - bossPosition;
		velocity.y = 2.0f;
		velocity = Mathf::Normalize(velocity);

		//火花のパーティクルの生成
		AccelerationField accelerationField = { { 0.0f,-0.04f,0.0f }, { {-100.0f,-100.0f,-100.0f}, {100.0f,100.0f,100.0f}},true };
		ParticleEmitter* emitter = ParticleEmitterBuilder()
			.SetAccelerationField(accelerationField)
			.SetDeleteTime(1.0f)
			.SetEmitterName("Hit")
			.SetPopArea({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetPopAzimuth(0.0f, 0.0f)
			.SetPopColor({ 1.0f, 0.2f, 0.2f, 1.0f }, { 1.0f, 0.2f, 0.2f, 1.0f })
			.SetPopCount(200)
			.SetPopElevation(0.0f, 0.0f)
			.SetPopFrequency(2.0f)
			.SetPopLifeTime(0.4f, 0.6f)
			.SetPopRotation({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetPopScale({ 0.08f,0.08f,0.08f }, { 0.1f,0.1f,0.1f })
			.SetPopVelocity({ velocity + Vector3{-0.2f,-0.2f,-0.2f} }, { velocity + Vector3{0.2f,0.2f,0.2f} })
			.SetTranslation(translation)
			.Build();
		particleSystem_->AddParticleEmitter(emitter);

		//衝撃波のパーティクルの生成
		ParticleEmitter* shockWaveEmitter = ParticleEmitterBuilder()
			.SetDeleteTime(1.0f)
			.SetEmitterName("ShockWave")
			.SetPopAzimuth(0.0f, 0.0f)
			.SetPopCount(1)
			.SetPopElevation(0.0f, 0.0f)
			.SetPopFrequency(10.0f)
			.SetPopLifeTime(0.2f, 0.2f)
			.SetPopScale({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f })
			.SetPopVelocity({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetTranslation(translation)
			.Build();
		shockWaveParticleSystem_->AddParticleEmitter(shockWaveEmitter);
	}
}

const Vector3 Weapon::GetWorldPosition() const
{
	Vector3 pos{};
	pos.x = worldTransformCollision_.matWorld_.m[3][0];
	pos.y = worldTransformCollision_.matWorld_.m[3][1];
	pos.z = worldTransformCollision_.matWorld_.m[3][2];
	return pos;
}