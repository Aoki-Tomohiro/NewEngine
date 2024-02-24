#include "GameManager.h"

void GameManager::Initialize()
{
	//基底クラスの初期化
	GameCore::Initialize();

	//シーンの生成
	sceneFactory_ = std::make_unique<SceneFactory>();
	sceneManager_->SetSceneFactory(sceneFactory_.get());
	sceneManager_->ChangeScene("GameTitleScene");

	//Bloomの有効化
	postEffects_->SetIsEnable(true);
	postEffects_->GetBloom()->SetIsEnable(true);
	postEffects_->GetBloom()->SetBlurCount(2);
}

void GameManager::Finalize()
{
	//基底クラスの終了処理
	GameCore::Finalize();
}

void GameManager::Update()
{
	//基底クラスの更新
	GameCore::Update();
}

void GameManager::Draw()
{
	//基底クラスの描画
	GameCore::Draw();
}