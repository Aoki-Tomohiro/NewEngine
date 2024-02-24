#pragma once
#include "Engine/Framework/Object/IGameObject.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/Components/Collision/Collider.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Engine/Components/Particle/ParticleManager.h"

class Weapon : public IGameObject, public Collider
{
public:
	void Initialize() override;

	void Update() override;

	void Draw(const Camera& camera) override;

	void DrawUI() override;

	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

	void OnCollision(Collider* collider) override;

	const Vector3 GetWorldPosition() const override;

	const Vector3& GetTranslation() const { return worldTransform_.translation_; };

	void SetTranslation(const Vector3& translation) { worldTransform_.translation_ = translation; };

	const Vector3& GetRotation() const { return worldTransform_.rotation_; };

	void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; };

	void SetParent(const WorldTransform* worldTransform) { worldTransform_.parent_ = worldTransform; };

	bool GetIsAttack() const { return isAttack_; };

	void SetIsAttack(bool isAttack) { isAttack_ = isAttack; };

	bool GetIsHit() const { return isHit_; };

	void SetIsHit(bool isHit) { isHit_ = isHit; };

private:
	//オーディオ
	Audio* audio_ = nullptr;

	//当たり判定用のワールドトランスフォーム
	WorldTransform worldTransformCollision_{};

	//パーティクル
	ParticleSystem* particleSystem_ = nullptr;
	ParticleSystem* shockWaveParticleSystem_ = nullptr;

	//OBB
	OBB obbSize{ .center{worldTransformCollision_.translation_},.orientations{{1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}},.size{1.0f,1.0f,1.0f} };

	//オーディオハンドル
	uint32_t slashAudioHandle_ = 0;

	//当たり判定のフラグ
	bool onCollision_ = false;
	bool preOnCollision_ = false;

	//攻撃フラグ
	bool isAttack_ = false;
	
	//ヒットフラグ
	bool isHit_ = false;
};

