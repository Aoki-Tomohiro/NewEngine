#pragma once
#include "IGameObject.h"
#include <vector>

class GameObjectManager
{
public:
	static GameObjectManager* GetInstance();

	static void Destroy();

	void Update();

	void Draw(const Camera& camera);

	void DrawUI();

	void Clear();

	template <typename Type>
	static Type* CreateGameObject();

	template <typename Type>
	const Type* GetGameObject(const std::string& tag) const;

	template <typename Type>
	const std::vector<Type*> GetGameObjects(const std::string& tag) const;

private:
	GameObjectManager() = default;
	~GameObjectManager() = default;
	GameObjectManager(const GameObjectManager&) = delete;
	const GameObjectManager& operator=(const GameObjectManager&) = delete;

	template <typename Type>
	Type* CreateGameObjectInternal();

private:
	static GameObjectManager* instance_;

	std::vector<std::unique_ptr<IGameObject>> gameObjects_{};
};

template <typename Type>
Type* GameObjectManager::CreateGameObject() {
	//ゲームオブジェクトを作成
	Type* newObject = GameObjectManager::GetInstance()->CreateGameObjectInternal<Type>();
	return newObject;
}

template <typename Type>
Type* GameObjectManager::CreateGameObjectInternal() {
	Type* newObject = new Type();
	newObject->Initialize();
	newObject->SetGameObjectManager(this);
	gameObjects_.push_back(std::unique_ptr<IGameObject>(newObject));
	return newObject;
}

template <typename Type>
const Type* GameObjectManager::GetGameObject(const std::string& tag) const
{
	//ゲームオブジェクトを探す
	for (const std::unique_ptr<IGameObject>& gameObject : gameObjects_)
	{
		if (gameObject->GetTag() == tag)
		{
			return dynamic_cast<Type*>(gameObject.get());
		}
	}
	return nullptr;
}

template <typename Type>
const std::vector<Type*> GameObjectManager::GetGameObjects(const std::string& tag) const
{
	//ゲームオブジェクトを探す
	std::vector<Type*> gameObjects{};
	for (const std::unique_ptr<IGameObject>& gameObject : gameObjects_)
	{
		if (gameObject->GetTag() == tag) {
			gameObjects.push_back(dynamic_cast<const Type*>(gameObject.get()));
		}
	}
	return gameObjects;
}