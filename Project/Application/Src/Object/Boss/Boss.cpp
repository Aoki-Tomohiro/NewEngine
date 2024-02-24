#include "Boss.h"
#include "Engine/Components/Collision/CollisionConfig.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Application/Src/Object/Player/Player.h"

Boss::~Boss()
{
	delete state_;
}

void Boss::Initialize()
{
	worldTransform_.Initialize();
	worldTransform_.translation_.y = 3.0f;
	worldTransform_.scale_ = { 3.0f,3.0f,3.0f };

	//状態の初期化
	state_ = new BossStateNormal();
	state_->Initialize(this);

	//テクスチャ読み込み
	TextureManager::Load("HpBarFrame.png");
	TextureManager::Load("HpBar.png");

	//スプライトの生成
	spriteHpBar_.reset(Sprite::Create("HpBar.png", { 720.0f,32.0f }));
	spriteHpBar_->SetColor({ 1.0f, 0.1f, 0.0f, 1.0f });
	spriteHpBarFrame_.reset(Sprite::Create("HpBarFrame.png", { 719.0f,31.0f }));
	spriteHpBarFrame_->SetColor({ 1.0f, 0.1f, 0.0f, 1.0f });

	//衝突属性を設定
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionMaskEnemy);
	SetCollisionPrimitive(kCollisionPrimitiveAABB);
	AABB aabb{
		.min{-worldTransform_.scale_.x,-worldTransform_.scale_.y,-worldTransform_.scale_.z},
		.max{worldTransform_.scale_.x,worldTransform_.scale_.y,worldTransform_.scale_.z} };
	SetAABB(aabb);

	//パーティクルシステムの生成
	particleSystem_ = ParticleManager::Create("Charge");
}

void Boss::Update()
{
	//前のフレームの座標を取得
	oldPosition_ = worldTransform_.translation_;

	//前のフレームの当たり判定のフラグを取得
	preOnCollision_ = onCollision_;
	onCollision_ = false;

	//状態の更新
	state_->Update(this);

	//死亡フラグの立ったミサイルを削除
	missiles_.remove_if([](std::unique_ptr<Missile>& missile)
		{
			if (missile->GetIsDead())
			{
				missile.reset();
				return true;
			}
			return false;
		}
	);

	//ミサイルの更新
	for (const std::unique_ptr<Missile>& missile : missiles_)
	{
		missile->Update();
	}

	//死亡フラグの立ったレーザーを削除
	lasers_.remove_if([](std::unique_ptr<Laser>& laser)
		{
			if (laser->GetIsDead())
			{
				laser.reset();
				return true;
			}
			return false;
		}
	);

	//レーザーの更新
	for (const std::unique_ptr<Laser>& laser : lasers_)
	{
		laser->Update();
	}

	//ワールドトランスフォームを取得
	worldTransform_ = state_->GetWorldTransform();

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromQuaternion();

	//HPバーの処理
	hpBarSize_ = { (hp_ / kMaxHP) * 480.0f,16.0f };
	spriteHpBar_->SetSize(hpBarSize_);
}

void Boss::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);

	//状態の描画
	state_->Draw(this, camera);

	//ミサイルの描画
	for (const std::unique_ptr<Missile>& missile : missiles_)
	{
		missile->Draw(camera);
	}

	//レーザーの描画
	for (const std::unique_ptr<Laser>& laser : lasers_)
	{
		laser->Draw(camera);
	}
}

void Boss::DrawUI()
{
	spriteHpBar_->Draw();
	spriteHpBarFrame_->Draw();
}

void Boss::ChangeState(IBossState* newState)
{
	delete state_;
	state_ = newState;
}

void Boss::OnCollision(Collider* collider)
{
	if (collider->GetCollisionAttribute() == kCollisionAttributeWeapon)
	{
		onCollision_ = true;

		if (onCollision_ != preOnCollision_)
		{
			const Player* player = GameObjectManager::GetInstance()->GetGameObject<Player>("Player");
			hp_ -= player->GetDamage();
			if (hp_ <= 0.0f)
			{
				hp_ = 0.0f;
			}
			//衝突判定の応答
			state_->OnCollision(collider);
			worldTransform_ = state_->GetWorldTransform();
			worldTransform_.UpdateMatrixFromQuaternion();
		}
	}
}

const Vector3 Boss::GetWorldPosition() const
{
	Vector3 pos{};
	pos.x = worldTransform_.matWorld_.m[3][0];
	pos.y = worldTransform_.matWorld_.m[3][1];
	pos.z = worldTransform_.matWorld_.m[3][2];
	return pos;
}