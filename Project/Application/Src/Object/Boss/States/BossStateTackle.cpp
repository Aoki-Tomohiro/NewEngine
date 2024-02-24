#include "BossStateTackle.h"
#include "Application/Src/Object/Boss/Boss.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Base/ImGuiManager.h"
#include "Engine/Utilities/RandomGenerator.h"

void BossStateTackle::Initialize(Boss* pBoss)
{
	worldTransform_ = pBoss->GetWorldTransform();
	destinationQuaternion_ = worldTransform_.quaternion_;
	pBoss->SetDamage(10.0f);

	//目標座標を設定
	Vector3 offset{ 0.0f,0.0f,6.0f };
	offset = Mathf::TransformNormal(offset, worldTransform_.matWorld_);
	targetPosition_ = worldTransform_.translation_ + offset;
	if (targetPosition_.x >= 47.0f)
	{
		targetPosition_.x = 47.0f;
	}
	else if (targetPosition_.x <= -47.0f)
	{
		targetPosition_.x = -47.0f;
	}

	if (targetPosition_.z >= 47.0f)
	{
		targetPosition_.z = 47.0f;
	}
	else if (targetPosition_.z <= -47.0f)
	{
		targetPosition_.z = -47.0f;
	}

	//警告モデルの作成
	waringModel_.reset(ModelManager::CreateFromOBJ("Warning", Opaque));
	waringModel_->SetEnableLighting(false);
	waringModel_->SetColor({ 1.0f,0.0f,0.0f,1.0f });

	//警告用のワールドトランスフォームの初期化
	warningWorldTransform_.Initialize();
	warningWorldTransform_.translation_ = (targetPosition_ + worldTransform_.translation_) / 2.0f;
	warningWorldTransform_.translation_.y = 0.1f;
	warningWorldTransform_.quaternion_ = worldTransform_.quaternion_;
	warningWorldTransform_.scale_ = { worldTransform_.scale_.x,worldTransform_.scale_.y,6.0f + 3.0f + 3.0f };
}

void BossStateTackle::Update(Boss* pBoss)
{
	//シェイク処理
	if (isShake_)
	{
		if (++shakeTimer_ > kShakeTime)
		{
			isShake_ = false;
			shakeTimer_ = 0;
			worldTransform_.translation_ = originalPosition_;
		}

		if (shakeTimer_ < 2)
		{
			worldTransform_.translation_.x += RandomGenerator::GetRandomFloat(-0.2f, 0.2f);
			worldTransform_.translation_.z += RandomGenerator::GetRandomFloat(-0.2f, 0.2f);
		}
		else if (shakeTimer_ >= 2 && shakeTimer_ < 4)
		{
			worldTransform_.translation_.x += RandomGenerator::GetRandomFloat(-0.16f, 0.16f);
			worldTransform_.translation_.z += RandomGenerator::GetRandomFloat(-0.16f, 0.16f);
		}
		else if (shakeTimer_ >= 4 && shakeTimer_ < 6)
		{
			worldTransform_.translation_.x += RandomGenerator::GetRandomFloat(-0.12f, 0.12f);
			worldTransform_.translation_.z += RandomGenerator::GetRandomFloat(-0.12f, 0.12f);
		}
		else if (shakeTimer_ >= 6 && shakeTimer_ < 8)
		{
			worldTransform_.translation_.x += RandomGenerator::GetRandomFloat(-0.08f, 0.08f);
			worldTransform_.translation_.z += RandomGenerator::GetRandomFloat(-0.08f, 0.08f);
		}
		else if (shakeTimer_ >= 8 && shakeTimer_ < 10)
		{
			worldTransform_.translation_.x += RandomGenerator::GetRandomFloat(-0.04f, 0.04f);
			worldTransform_.translation_.z += RandomGenerator::GetRandomFloat(-0.04f, 0.04f);
		}
	}

	if (!isAttack_)
	{
		if (++waitTimer_ > kWaitTime)
		{
			isAttack_ = true;
			pBoss->SetIsAttack(true);
		}
	}

	if (!isRecovery_)
	{
		if (isAttack_)
		{
			worldTransform_.translation_ = Mathf::Lerp(worldTransform_.translation_, targetPosition_, 0.2f);

			const float epsilon = 0.001f;
			Vector3 abs = {
				std::abs(worldTransform_.translation_.x - targetPosition_.x),
				std::abs(worldTransform_.translation_.y - targetPosition_.y),
				std::abs(worldTransform_.translation_.z - targetPosition_.z),
			};
			if (abs.x < epsilon && abs.y < epsilon && abs.z < epsilon)
			{
				isRecovery_ = true;
				pBoss->SetIsAttack(false);
			}
		}
	}

	if (isRecovery_)
	{
		if (++recoveryTimer_ > kRecoveryTime)
		{
			IBossState* newState = new BossStateNormal();
			newState->Initialize(pBoss);
			pBoss->ChangeState(newState);
			return;
		}
	}

	//移動限界座標
	const float kMoveLimitX = 47;
	const float kMoveLimitZ = 47;
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.z = max(worldTransform_.translation_.z, -kMoveLimitZ);
	worldTransform_.translation_.z = min(worldTransform_.translation_.z, +kMoveLimitZ);

	//警告用のワールドトランスフォームの更新
	warningWorldTransform_.UpdateMatrixFromQuaternion();

	ImGui::Begin("Tackle");
	ImGui::End();
}

void BossStateTackle::Draw(Boss* pBoss, const Camera& camera)
{
	if (!isAttack_)
	{
		waringModel_->Draw(warningWorldTransform_, camera);
	}
}

void BossStateTackle::OnCollision(Collider* collider)
{
	if (collider->GetCollisionAttribute() == kCollisionAttributeWeapon)
	{
		if (!isShake_)
		{
			isShake_ = true;
			originalPosition_ = worldTransform_.translation_;
		}
	}
}