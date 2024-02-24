#pragma once
#include "Engine/Framework/Scene/IScene.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/3D/Model/ModelManager.h"
#include "Engine/2D/Sprite.h"
#include "Engine/Components/Collision/CollisionManager.h"

#include "Application/Src/Object/Character/Player.h"
#include "Application/Src/Object/Character/Enemy.h"
#include "Application/Src/Object/Skydome/Skydome.h"
#include "Application/Src/Object/FollowCamera/FollowCamera.h"
#include "Application/Src/Object/Floor/Floor.h"
#include "Application/Src/Object/Goal/Goal.h"
#include "Application/Src/Object/LockOn/LockOn.h"

class GamePlayScene : public IScene
{
public:
	//床の数
	static const int kFloorMax = 7;
	//敵の数
	static const int kEnemyMax = 5;

	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

	void DrawUI() override;

private:
	void AddEnemy(const Vector3& position, const Vector3& velocity);

	void Reset();

private:
	Renderer* renderer_ = nullptr;

	Input* input_ = nullptr;

	Audio* audio_ = nullptr;

	//カメラ
	Camera camera_{};

	//衝突マネージャー
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;

	//プレイヤー
	std::unique_ptr<Model> playerModel_ = nullptr;
	std::unique_ptr<Model> weaponModel_ = nullptr;
	std::unique_ptr<Player> player_ = nullptr;

	//敵
	std::unique_ptr<Model> modelEnemyBody_ = nullptr;
	std::unique_ptr<Model> modelEnemyL_arm_ = nullptr;
	std::unique_ptr<Model> modelEnemyR_arm_ = nullptr;
	std::list<std::unique_ptr<Enemy>> enemies_{};

	//天球
	std::unique_ptr<Model> skydomeModel_ = nullptr;
	std::unique_ptr<Skydome> skydome_ = nullptr;

	//追従カメラ
	std::unique_ptr<FollowCamera> followCamera_ = nullptr;

	//床
	std::unique_ptr<Model> floorModel_ = nullptr;
	std::vector<std::unique_ptr<Floor>> floors_{};

	//ゴール
	std::unique_ptr<Model> goalModel_ = nullptr;
	std::unique_ptr<Goal> goal_ = nullptr;

	//ロックオン
	std::unique_ptr<LockOn> lockOn_ = nullptr;

	//敵の座標
	Vector3 enemyPositions_[kEnemyMax] = {
		{ 0.0f,0.0f,35.0f },
		{ -5.0f,0.0f,45.0f },
		{ 5.0f,0.0f,75.0f },
		{ 5.0f,0.0f,85.0f },
		{ -5.0f,0.0f,115.0f },
	};

	//敵の速度
	Vector3 enemyVelocities_[kEnemyMax] = {
		{ 0.0f,0.0f,0.0f },
		{ 0.06f,0.0f,0.0f },
		{ 0.06f,0.0f,0.0f },
		{ 0.04f,0.0f,0.0f },
		{ 0.04f,0.0f,0.0f },
	};
};

