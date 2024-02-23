#pragma once
#include "AbstractSceneFactory.h"
#include "IScene.h"

class SceneManager
{
public:
	static SceneManager* GetInstance();

	static void Destroy();

	void Update();

	void Draw();

	void DrawUI();

	void ChangeScene(const std::string& sceneName);

	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; };

private:
	SceneManager() = default;
	~SceneManager();
	SceneManager(const SceneManager&) = delete;
	const SceneManager& operator=(const SceneManager&) = delete;

private:
	static SceneManager* instance_;

	IScene* currentScene_ = nullptr;

	IScene* nextScene_ = nullptr;

	AbstractSceneFactory* sceneFactory_ = nullptr;
};

