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

	//パーティクル
	TextureManager::Load("circle.png");
	particleSystem_ = ParticleManager::Create("Hit");
	particleSystem_->SetTexture("circle.png");
}

void Weapon::Update() {
	//前のフレームの当たり判定のフラグを取得
	preOnCollision_ = onCollision_;
	onCollision_ = false;

	//当たり判定の位置を決める
	Vector3 direction{ 0.0f,0.0f,4.0f };
	//direction = TransformNormal(direction, worldTransform_.matWorld_);
	direction = Mathf::TransformNormal(direction, worldTransform_.parent_->matWorld_);
	worldTransformCollision_.translation_ = { worldTransform_.matWorld_.m[3][0],worldTransform_.matWorld_.m[3][1] ,worldTransform_.matWorld_.m[3][2] };
	worldTransformCollision_.translation_ += direction;

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();
	worldTransformCollision_.UpdateMatrixFromEuler();

	//パーティクルの更新
	particleSystem_->Update();
}

void Weapon::Draw(const Camera& camera) {
	//モデルの描画
	model_->Draw(worldTransform_, camera);
}

void Weapon::DrawParticle(const Camera& camera) {
	particleSystem_->Draw(camera);
}

void Weapon::OnCollision(Collider* collider) {
	onCollision_ = true;
	if (onCollision_ != preOnCollision_) {
		//パーティクル
		ParticleEmitter* newParticleEmitter = ParticleEmitterBuilder()
			.SetPopArea({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetPopAzimuth(0.0f, 360.0f)
			.SetPopColor({ 1.0f, 0.5f, 0.2f, 1.0f }, { 1.0f, 0.8f, 0.4f, 1.0f })
			.SetPopCount(100)
			.SetDeleteTime(3.0f)
			//.SetElevation(0.0f, 180.0f)
			.SetPopElevation(-90.0f, 90.0f)
			.SetEmitterName("Hit")
			.SetPopFrequency(4.0f)
			//.SetLifeTime(0.1f, 1.0f)
			.SetPopLifeTime(0.2f, 0.4f)
			.SetPopRotation({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f })
			.SetPopScale({ 0.2f, 0.2f,0.2f }, { 0.25f ,0.25f ,0.25f })
			.SetTranslation(collider->GetWorldPosition())
			//.SetVelocity({ 0.02f ,0.02f ,0.02f }, { 0.04f ,0.04f ,0.04f })
			.SetPopVelocity({ 0.2f, 0.2f, 0.2f }, { 0.4f, 0.4f, 0.4f })
			.Build();
		particleSystem_->AddParticleEmitter(newParticleEmitter);
	}
}

const Vector3 Weapon::GetWorldPosition() const {
	Vector3 pos{};
	pos.x = worldTransformCollision_.matWorld_.m[3][0];
	pos.y = worldTransformCollision_.matWorld_.m[3][1];
	pos.z = worldTransformCollision_.matWorld_.m[3][2];
	return pos;
}