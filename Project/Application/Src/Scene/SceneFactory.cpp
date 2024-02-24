#include "SceneFactory.h"

IScene* SceneFactory::CreateScene(const std::string& sceneName) 
{
	//次のシーンを生成
	IScene* newScene = nullptr;

	if (sceneName == "GameTitleScene") 
	{
		newScene = new GameTitleScene();
	}
	else if (sceneName == "GamePlayScene")
	{
		newScene = new GamePlayScene();
	}
	else if (sceneName == "GameClearScene") 
	{
		newScene = new GameClearScene();
	}
	else if (sceneName == "GameOverScene") 
	{
		newScene = new GameOverScene();
	}
	else if (sceneName == "LoadScene")
	{
		newScene = new LoadScene();
	}

	return newScene;
}