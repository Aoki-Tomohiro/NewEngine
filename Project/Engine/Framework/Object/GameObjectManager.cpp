#include "GameObjectManager.h"

//実体定義
GameObjectManager* GameObjectManager::instance_ = nullptr;

GameObjectManager* GameObjectManager::GetInstance() 
{
	if (instance_ == nullptr)
	{
		instance_ = new GameObjectManager();
	}
	return instance_;
}

void GameObjectManager::Destroy() 
{
	if (instance_)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

void GameObjectManager::Update()
{
	//破壊フラグが立ったゲームオブジェクトを削除
	std::vector<std::unique_ptr<IGameObject>>::iterator it = std::remove_if(gameObjects_.begin(), gameObjects_.end(),
		[](std::unique_ptr<IGameObject>& gameObject)
		{
			return gameObject->GetIsDestroy();
		});
	gameObjects_.erase(it, gameObjects_.end());

	//ゲームオブジェクトの更新
	for (std::unique_ptr<IGameObject>& gameObject : gameObjects_)
	{
		if (gameObject->GetIsActive())
		{
			gameObject->Update();
		}
	}
}

void GameObjectManager::Draw(const Camera& camera)
{
	//ゲームオブジェクトの描画
	for (std::unique_ptr<IGameObject>& gameObject : gameObjects_)
	{
		if (gameObject->GetIsVisible())
		{
			gameObject->Draw(camera);
		}
	}
}

void GameObjectManager::DrawUI()
{
	//ゲームオブジェクトのUI描画
	for (std::unique_ptr<IGameObject>& gameObject : gameObjects_)
	{
		gameObject->DrawUI();
	}
}

void GameObjectManager::Clear()
{
	//ゲームオブジェクトをクリア
	gameObjects_.clear();
}