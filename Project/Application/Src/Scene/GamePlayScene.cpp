#include "GamePlayScene.h"
#include "Engine/Framework/Scene/SceneManager.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Utilities/RandomGenerator.h"

void GamePlayScene::Initialize()
{
	renderer_ = Renderer::GetInstance();

	input_ = Input::GetInstance();

	audio_ = Audio::GetInstance();

	//ゲームオブジェクトをクリア
	gameObjectManager_ = GameObjectManager::GetInstance();
	gameObjectManager_->Clear();

	//パーティクルをクリア
	particleManager_ = ParticleManager::GetInstance();
	particleManager_->Clear();

	//衝突マネージャーの生成
	collisionManager_ = std::make_unique<CollisionManager>();

	//カメラの初期化
	camera_.Initialize();

	//ロックオンの初期化
	lockOn_ = std::make_unique<LockOn>();
	lockOn_->Initialize();

	//追従カメラの生成
	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Initialize();
	followCamera_->SetLockOn(lockOn_.get());

	//プレイヤーの生成
	playerModelHead_.reset(ModelManager::CreateFromOBJ("PlayerHead", Opaque));
	playerModelHead_->SetEnableLighting(false);
	playerModelBody_.reset(ModelManager::CreateFromOBJ("PlayerBody", Opaque));
	playerModelBody_->SetEnableLighting(false);
	playerModelL_Arm_.reset(ModelManager::CreateFromOBJ("PlayerL_arm", Opaque));
	playerModelL_Arm_->SetEnableLighting(false);
	playerModelR_Arm_.reset(ModelManager::CreateFromOBJ("PlayerR_arm", Opaque));
	playerModelR_Arm_->SetEnableLighting(false);
	std::vector<Model*> playerModels = { playerModelBody_.get(),playerModelHead_.get(),playerModelL_Arm_.get(),playerModelR_Arm_.get() };
	player_ = GameObjectManager::CreateGameObject<Player>();
	player_->SetModels(playerModels);
	player_->SetTag("Player");
	player_->SetCamera(&camera_);
	player_->SetLockOn(lockOn_.get());
	//追従対象にプレイヤーを設定
	followCamera_->SetTarget(&player_->GetWorldTransform());

	//ボスの生成
	bossModel_.reset(ModelManager::CreateFromOBJ("Boss", Opaque));
	bossModel_->SetEnableLighting(false);
	bossModel_->SetColor({ 0.9f, 0.5f, 0.9f, 1.0f });
	boss_ = GameObjectManager::CreateGameObject<Boss>();
	boss_->SetModel(bossModel_.get());
	boss_->SetTag("Boss");

	//天球の作成
	skydomeModel_.reset(ModelManager::CreateFromOBJ("Skydome", Opaque));
	skydomeModel_->SetEnableLighting(false);
	skydome_ = GameObjectManager::CreateGameObject<Skydome>();
	skydome_->SetModel(skydomeModel_.get());

	//地面の生成
	groundModel_.reset(ModelManager::CreateFromOBJ("Ground", Opaque));
	groundModel_->SetEnableLighting(false);
	ground_ = GameObjectManager::CreateGameObject<Ground>();
	ground_->SetModel(groundModel_.get());

	//トランジションの初期化
	transitionSprite_.reset(Sprite::Create("white.png", { 0.0f,0.0f }));
	transitionSprite_->SetSize({ 1280.0f,720.0f });
	transitionSprite_->SetColor(transitionSpriteColor_);

	//ガイドのスプライトの生成
	TextureManager::Load("Guide.png");
	guideSprite_.reset(Sprite::Create("Guide.png", { 0.0f,0.0f }));

	//BGMの読み込みと再生
	bgmHandle_ = audio_->SoundLoadWave("Application/Resources/Sounds/GamePlay.wav");
	audio_->SoundPlayWave(bgmHandle_, true, 0.5f);
}

void GamePlayScene::Finalize()
{

}

void GamePlayScene::Update()
{
	//トランジションの更新
	UpdateTransition();

	//パーティクルの更新
	particleManager_->Update();

	//プレイヤーの攻撃がヒットしたか、またはプレイヤーがダメージを受けたとき
	if (player_->GetWeapon()->GetIsHit() || player_->GetIsHit())
	{
		//カメラシェイクのフラグを立てる
		cameraShakeEnable_ = true;

		//最後の攻撃の時
		if (player_->GetComboIndex() == 3)
		{
			shakeIntensityX = 0.0f;
			shakeIntensityY = 0.4f;
		}
		//プレイヤーがダメージを食らった時
		else if (player_->GetIsHit())
		{
			shakeIntensityX = 0.6f;
			shakeIntensityY = 0.6f;
		}
		//そのほか
		else
		{
			shakeIntensityX = 0.0f;
			shakeIntensityY = 0.1f;
		}
	}

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

	//カメラの更新
	camera_.UpdateMatrix();

	//ヒットストップ中じゃないときに武器が当たったら
	if (!isStop_ && player_->GetWeapon()->GetIsHit())
	{
		//ヒットストップのフラグを立てる
		isStop_ = true;
		//最後の攻撃の時はヒットストップを長めに
		stopTime_ = (player_->GetComboIndex() == 3) ? 10 : 2;
		//ヒットフラグをリセット
		player_->GetWeapon()->SetIsHit(false);
	}

	//ヒットストップの処理
	if (isStop_)
	{
		if (++stopTimer_ >= stopTime_)
		{
			isStop_ = false;
			stopTimer_ = 0;
		}
		return;
	}

	//ゲームオブジェクトの更新
	gameObjectManager_->Update();

	//ロックオンの処理
	lockOn_->Update(boss_, camera_);

	//追従カメラの更新
	followCamera_->Update();
	camera_ = followCamera_->GetCamera();

	//衝突判定
	collisionManager_->ClearColliderList();
	collisionManager_->SetColliderList(player_);
	Weapon* weapon = player_->GetWeapon();
	if (weapon->GetIsAttack())
	{
		collisionManager_->SetColliderList(weapon);
	}
	for (const std::unique_ptr<Missile>& missile : boss_->GetMissiles())
	{
		collisionManager_->SetColliderList(missile.get());
	}
	for (const std::unique_ptr<Laser>& laser : boss_->GetLasers())
	{
		collisionManager_->SetColliderList(laser.get());
	}
	collisionManager_->SetColliderList(boss_);
	collisionManager_->CheckAllCollisions();
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
	//ゲームオブジェクトのモデル描画
	gameObjectManager_->Draw(camera_);

	//3Dオブジェクト描画
	renderer_->Render();
#pragma endregion

#pragma region パーティクル描画
	//パーティクル描画前処理
	renderer_->PreDrawParticles();

	//パーティクルの描画
	particleManager_->Draw(camera_);

	//パーティクル描画後処理
	renderer_->PostDrawParticles();
#pragma endregion
}

void GamePlayScene::DrawUI()
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	//ゲームオブジェクトのスプライト描画
	gameObjectManager_->DrawUI();

	//ロックオンの描画
	lockOn_->Draw();

	//ガイドのスプライトの描画
	guideSprite_->Draw();

	//トランジション用のスプライトの描画
	transitionSprite_->Draw();

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}

void GamePlayScene::UpdateTransition()
{
	//フェードアウトの処理
	if (isFadeOut_)
	{
		//徐々に透明にする
		transitionTimer_ += 1.0f / 60.0f;
		transitionSpriteColor_.w = Mathf::Lerp(transitionSpriteColor_.w, 0.0f, transitionTimer_);
		transitionSprite_->SetColor(transitionSpriteColor_);

		//完全に透明になったら終了
		if (transitionSpriteColor_.w <= 0.0f)
		{
			isFadeOut_ = false;
			transitionTimer_ = 0.0f;
		}
	}

	//フェードインの処理
	if (isFadeIn_)
	{
		//徐々に暗くする
		transitionTimer_ += 1.0f / 60.0f;
		transitionSpriteColor_.w = Mathf::Lerp(transitionSpriteColor_.w, 1.0f, transitionTimer_);
		transitionSprite_->SetColor(transitionSpriteColor_);

		//完全に暗くなったらシーンを変える
		if (transitionSpriteColor_.w >= 1.0f)
		{
			switch (nextScene_)
			{
			case kGameClear:
				sceneManager_->ChangeScene("GameClearScene");
				break;
			case kGameOver:
				sceneManager_->ChangeScene("GameOverScene");
				break;
			}
			audio_->StopAudio(bgmHandle_);
		}
	}

	//トランジションが行われていないときに入力を受け付ける
	if (!isFadeOut_ && !isFadeIn_)
	{
		//ボスの体力が0になったらゲームクリア
		if (boss_->GetHP() <= 0.0f)
		{
			isFadeIn_ = true;
			nextScene_ = kGameClear;
		}

		//プレイヤーの体力が0になったらゲームオーバー
		if (player_->GetHP() <= 0.0f)
		{
			isFadeIn_ = true;
			nextScene_ = kGameOver;
		}

		//キーボード
		if (input_->IsPushKeyEnter(DIK_T))
		{
			isFadeIn_ = true;
		}
	}
}