#include "LoadScene.h"

void LoadScene::Initialize()
{
	renderer_ = Renderer::GetInstance();

	TextureManager::Load("LoadTexture1.png");
	TextureManager::Load("LoadTexture2.png");
	TextureManager::Load("LoadTexture3.png");
	TextureManager::Load("LoadTexture4.png");

	loadSprite_.reset(Sprite::Create("LoadTexture4.png", { 0.0f,0.0f }));
}

void LoadScene::Finalize()
{

}

void LoadScene::Update()
{
	if (++timer_ > 10)
	{
		timer_ = 0;
		animationCount_++;
	}

	if (animationCount_ > 3)
	{
		animationCount_ = 0;
	}

	switch (animationCount_)
	{
	case 0:
		loadSprite_->SetTexture("LoadTexture4.png");
		break;
	case 1:
		loadSprite_->SetTexture("LoadTexture3.png");
		break;
	case 2:
		loadSprite_->SetTexture("LoadTexture2.png");
		break;
	case 3:
		loadSprite_->SetTexture("LoadTexture1.png");
		break;
	}
	
}

void LoadScene::Draw()
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

void LoadScene::DrawUI()
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	//ロード画面の描画
	loadSprite_->Draw();

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}