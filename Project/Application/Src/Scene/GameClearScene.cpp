#include "GameClearScene.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Framework/Scene/SceneManager.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Engine/Math/MathFunction.h"

void GameClearScene::Initialize() 
{
	renderer_ = Renderer::GetInstance();

	input_ = Input::GetInstance();

	audio_ = Audio::GetInstance();

	GameObjectManager::GetInstance()->Clear();

	//スプライトの生成
	TextureManager::Load("white.png");
	TextureManager::Load("GameClear.png");
	sprite_.reset(Sprite::Create("white.png", { 0.0f,0.0f }));
	backGroundSprite_.reset(Sprite::Create("GameClear.png", { 0.0f,0.0f }));
	sprite_->SetSize({ 1280.0f,720.0f });
	sprite_->SetColor(spriteColor_);

	camera_.Initialize();

	skydomeModel_.reset(ModelManager::CreateFromOBJ("Skydome", Opaque));
	skydomeModel_->SetEnableLighting(false);
	skydome_ = GameObjectManager::CreateGameObject<Skydome>();
	skydome_->SetModel(skydomeModel_.get());
}

void GameClearScene::Finalize() 
{

}

void GameClearScene::Update() 
{
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
			sceneManager_->ChangeScene("GameTitleScene");
		}
	}

	if (isTransitionEnd_)
	{
		if (input_->IsControllerConnected())
		{
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_A))
			{
				isTransition_ = true;
			}
		}
	}

	//ゲームオブジェクトの更新
	GameObjectManager::GetInstance()->Update();
}

void GameClearScene::Draw() 
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
	GameObjectManager::GetInstance()->Draw(camera_);

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

void GameClearScene::DrawUI() 
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	backGroundSprite_->Draw();

	sprite_->Draw();

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}