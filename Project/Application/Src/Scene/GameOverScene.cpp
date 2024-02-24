#include "GameOverScene.h"
#include "Engine/Framework/Scene/SceneManager.h"
#include "Engine/Base/TextureManager.h"

void GameOverScene::Initialize() 
{
	renderer_ = Renderer::GetInstance();

	input_ = Input::GetInstance();

	audio_ = Audio::GetInstance();

	//カメラの初期化
	camera_.Initialize();

	//ゲームオブジェクトをクリア
	gameObjectManager_ = GameObjectManager::GetInstance();
	gameObjectManager_->Clear();

	//天球の作成
	skydomeModel_.reset(ModelManager::CreateFromOBJ("Skydome", Opaque));
	skydomeModel_->SetEnableLighting(false);
	skydome_ = GameObjectManager::CreateGameObject<Skydome>();
	skydome_->SetModel(skydomeModel_.get());

	//トランジションの初期化
	transitionSprite_.reset(Sprite::Create("white.png", { 0.0f,0.0f }));
	transitionSprite_->SetSize({ 1280.0f,720.0f });
	transitionSprite_->SetColor(transitionSpriteColor_);

	//ゲームオーバーのスプライトの生成
	TextureManager::Load("GameOver.png");
	gameOverSprite_.reset(Sprite::Create("GameOver.png", { 0.0f,0.0f }));
}

void GameOverScene::Finalize() 
{

}

void GameOverScene::Update()
{
	//トランジションの更新
	UpdateTransition();

	//ゲームオブジェクトの更新
	gameObjectManager_->Update();

	//カメラの更新
	camera_.UpdateMatrix();
}

void GameOverScene::Draw()
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

	//パーティクル描画後処理
	renderer_->PostDrawParticles();
#pragma endregion
}

void GameOverScene::DrawUI() 
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	//ゲームオーバーのスプライトの描画
	gameOverSprite_->Draw();

	//トランジション用のスプライトの描画
	transitionSprite_->Draw();

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}

void GameOverScene::UpdateTransition()
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
			sceneManager_->ChangeScene("GameTitleScene");
		}
	}

	//トランジションが行われていないときに入力を受け付ける
	if (!isFadeOut_ && !isFadeIn_)
	{
		//コントローラー
		if (input_->IsControllerConnected())
		{
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A))
			{
				isFadeIn_ = true;
			}
		}

		//キーボード
		if (input_->IsPushKeyEnter(DIK_SPACE))
		{
			isFadeIn_ = true;
		}
	}
}