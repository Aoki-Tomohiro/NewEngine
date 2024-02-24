#include "GamePlayScene.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Framework/Scene/SceneManager.h"
#include "Engine/Math/MathFunction.h"
#include "Engine/Utilities/RandomGenerator.h"

void GamePlayScene::Initialize()
{
	renderer_ = Renderer::GetInstance();

	input_ = Input::GetInstance();

	audio_ = Audio::GetInstance();

	camera_.Initialize();

	collisionManager_ = std::make_unique<CollisionManager>();

	GameObjectManager::GetInstance()->Clear();

	//ロックオンの初期化
	lockOn_ = std::make_unique<LockOn>();
	lockOn_->Initialize();

	//プレイヤーの生成
	playerModel_.reset(ModelManager::CreateFromOBJ("Player", Opaque));
	playerModel_->SetColor({ 0.45f, 0.85f, 0.45f, 1.0f });
	playerModel_->SetEnableLighting(false);
	player_ = GameObjectManager::CreateGameObject<Player>();
	player_->SetTag("Player");
	player_->SetModel(playerModel_.get());
	player_->SetCamera(&camera_);
	player_->SetLockOn(lockOn_.get());

	//追従カメラの生成
	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Initialize();
	followCamera_->SetLockOn(lockOn_.get());
	followCamera_->SetTarget(&player_->GetWorldTransform());

	//ボスの生成
	bossModel_.reset(ModelManager::CreateFromOBJ("Boss", Opaque));
	bossModel_->SetEnableLighting(false);
	bossModel_->SetColor({ 0.9f, 0.5f, 0.9f, 1.0f });
	boss_ = GameObjectManager::CreateGameObject<Boss>();
	boss_->SetModel(bossModel_.get());
	boss_->SetTag("Boss");

	//天球の生成
	skydomeModel_.reset(ModelManager::CreateFromOBJ("Skydome", Opaque));
	skydomeModel_->SetEnableLighting(false);
	skydome_ = GameObjectManager::CreateGameObject<Skydome>();
	skydome_->SetModel(skydomeModel_.get());

	//地面の生成
	groundModel_.reset(ModelManager::CreateFromOBJ("Cube", Opaque));
	groundModel_->SetColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	groundModel_->SetEnableLighting(false);
	ground_ = GameObjectManager::CreateGameObject<Ground>();
	ground_->SetModel(groundModel_.get());

	//スプライトの生成
	sprite_.reset(Sprite::Create("white.png", { 0.0f,0.0f }));
	sprite_->SetSize({ 1280.0f,720.0f });
	sprite_->SetColor(spriteColor_);

	//UI
	TextureManager::Load("Guide.png");
	UISprite_.reset(Sprite::Create("Guide.png", { 0.0f,0.0f }));

	//BGM
	audioHandle_ = Audio::GetInstance()->SoundLoadWave("Application/Resources/Sounds/GamePlay2.wav");
	Audio::GetInstance()->SoundPlayWave(audioHandle_, true, 0.5f);
}

void GamePlayScene::Finalize()
{

}

void GamePlayScene::Update()
{
	//トランジションの処理
	if (!isTransitionEnd_)
	{
		transitionTimer_ += 1.0f / kTransitionTime;
		spriteColor_.w = Mathf::Lerp(spriteColor_.w, 0.0f, transitionTimer_);
		sprite_->SetColor(spriteColor_);

		if (spriteColor_.w <= 0.0f)
		{
			isTransitionEnd_ = true;
			transitionTimer_ = 0.0f;
		}
	}

	if (isTransition_)
	{
		transitionTimer_ += 1.0f / kTransitionTime;
		spriteColor_.w = Mathf::Lerp(spriteColor_.w, 1.0f, transitionTimer_);
		sprite_->SetColor(spriteColor_);

		if (spriteColor_.w >= 1.0f)
		{
			switch (nextScene)
			{
			case GameClearScene:
				sceneManager_->ChangeScene("GameClearScene");
				Audio::GetInstance()->StopAudio(audioHandle_);
				break;
			case GameOverScene:
				sceneManager_->ChangeScene("GameOverScene");
				Audio::GetInstance()->StopAudio(audioHandle_);
				break;
			}
		}
	}

	//カメラシェイクのフラグを立てる
	if (player_->GetWeapon()->GetIsHit() || player_->GetIsHit())
	{
		cameraShakeEnable_ = true;

		if (player_->GetComboIndex() == 3)
		{
			shakeIntensityX = 0.0f;
			shakeIntensityY = 0.6f;
		}
		else if (player_->GetIsHit())
		{
			shakeIntensityX = 0.6f;
			shakeIntensityY = 0.6f;
		}
		else
		{
			shakeIntensityX = 0.0f;
			shakeIntensityY = 0.1f;
		}
	}

	//ヒットストップのフラグを立てる
	if (!isStop_ && player_->GetWeapon()->GetIsHit())
	{
		isStop_ = true;
		if (player_->GetComboIndex() == 3)
		{
			kStopTime = 10;
		}
		else
		{
			kStopTime = 2;
		}
		player_->GetWeapon()->SetIsHit(false);
	}

	//カメラの処理
	camera_ = followCamera_->GetCamera();
	//カメラシェイクの処理
	if (cameraShakeEnable_)
	{
		if (++shakeTimer_ >= kShakeTime)
		{
			cameraShakeEnable_ = false;
			shakeTimer_ = 0;
		}

		camera_.translation_.x += RandomGenerator::GetRandomFloat(-shakeIntensityX, shakeIntensityX);
		camera_.translation_.y += RandomGenerator::GetRandomFloat(-shakeIntensityY, shakeIntensityY);
	}
	//行列の更新
	camera_.UpdateMatrix();

	//パーティクルの更新
	ParticleManager::GetInstance()->Update();

	//ヒットストップの処理
	if (isStop_)
	{
		if (++stopTimer_ >= kStopTime)
		{
			isStop_ = false;
			stopTimer_ = 0;
		}
		return;
	}

	//ゲームオブジェクトの更新
	GameObjectManager::GetInstance()->Update();

	//ロックオンの処理
	lockOn_->Update(boss_, camera_);

	//追従カメラの更新
	followCamera_->Update();

	//衝突判定
	collisionManager_->ClearColliderList();
	collisionManager_->SetColliderList(player_);
	Weapon* weapon = player_->GetWeapon();
	if (weapon->GetIsAttack())
	{
		collisionManager_->SetColliderList(weapon);
	}
	collisionManager_->SetColliderList(boss_);
	for (const std::unique_ptr<Missile>& missile : boss_->GetMissiles())
	{
		collisionManager_->SetColliderList(missile.get());
	}
	for (const std::unique_ptr<Laser>& laser : boss_->GetLasers())
	{
		collisionManager_->SetColliderList(laser.get());
	}
	collisionManager_->CheckAllCollisions();

	//ゲームクリア処理
	if (boss_->GetHP() <= 0.0f)
	{
		nextScene = GameClearScene;
		isTransition_ = true;
	}

	//ゲームオーバー処理
	if (player_->GetHP() <= 0.0f)
	{
		nextScene = GameOverScene;
		isTransition_ = true;
	}

	//シーン切り替え
	if (isTransitionEnd_)
	{
		if (input_->IsPushKeyEnter(DIK_F1))
		{
			nextScene = GameClearScene;
			isTransition_ = true;
		}

		if (input_->IsPushKeyEnter(DIK_F2))
		{
			nextScene = GameOverScene;
			isTransition_ = true;
		}
	}
}

void GamePlayScene::Draw() 
{
#pragma region 背景スプライト描画
	//背景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	//背景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion

	//深度バッファをクリア
	renderer_->ClearDepthBuffer();

#pragma region 3Dオブジェクト描画
	//ゲームオブジェクトの描画
	GameObjectManager::GetInstance()->Draw(camera_);

	//3Dオブジェクト描画
	renderer_->Render();
#pragma endregion

#pragma region パーティクル描画
	//パーティクル描画前処理
	renderer_->PreDrawParticles();

	//パーティクルの描画
	ParticleManager::GetInstance()->Draw(camera_);

	//パーティクル描画後処理
	renderer_->PostDrawParticles();
#pragma endregion
}

void GamePlayScene::DrawUI()
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	//ゲームオブジェクトのUIの描画
	GameObjectManager::GetInstance()->DrawUI();

	lockOn_->Draw();

	UISprite_->Draw();

	sprite_->Draw();

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}