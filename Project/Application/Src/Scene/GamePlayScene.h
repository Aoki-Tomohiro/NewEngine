#pragma once
#include "Engine/Framework/Scene/IScene.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/Components/Particle/ParticleManager.h"
#include "Engine/Components/Collision/CollisionManager.h"
#include "Engine/3D/Model/ModelManager.h"
#include "Engine/2D/Sprite.h"

#include "Application/Src/Object/Skydome/Skydome.h"
#include "Application/Src/Object/Ground/Ground.h"
#include "Application/Src/Object/Player/Player.h"
#include "Application/Src/Object/Boss/Boss.h"
#include "Application/Src/Object/FollowCamera/FollowCamera.h"
#include "Application/Src/Object/LockOn/LockOn.h"

class GamePlayScene : public IScene
{
public:
	enum NextScene
	{
		GameClearScene,
		GameOverScene,
	};

	//トランジションの時間
	static const int kTransitionTime = 60;
	//static const uint32_t kStopTime = 2;
	static const uint32_t kShakeTime = 20;

	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

	void DrawUI() override;

private:
	Renderer* renderer_ = nullptr;

	Input* input_ = nullptr;

	Audio* audio_ = nullptr;

	Camera camera_{};
	bool cameraShakeEnable_ = false;
	uint32_t shakeTimer_ = 0;

	//シェイクの強さ
	float shakeIntensityX = 0.6f;
	float shakeIntensityY = 0.6f;


	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;

	//ロックオン
	std::unique_ptr<LockOn> lockOn_ = nullptr;

	//追従カメラ
	std::unique_ptr<FollowCamera> followCamera_ = nullptr;

	//プレイヤー
	std::unique_ptr<Model> playerModel_ = nullptr;
	Player* player_ = nullptr;

	//ボス
	std::unique_ptr<Model> bossModel_ = nullptr;
	Boss* boss_ = nullptr;

	//天球
	std::unique_ptr<Model> skydomeModel_ = nullptr;
	Skydome* skydome_ = nullptr;

	//地面
	std::unique_ptr<Model> groundModel_ = nullptr;
	Ground* ground_ = nullptr;

	//UI
	std::unique_ptr<Sprite> UISprite_ = nullptr;

	//オーディオハンドル
	uint32_t audioHandle_ = 0;

	//ヒットストップ関連
	bool isStop_ = false;
	uint32_t kStopTime = 2;
	uint32_t stopTimer_ = 0;

	//トランジション
	std::unique_ptr<Sprite> sprite_ = nullptr;
	Vector4 spriteColor_{ 0.0f,0.0f,0.0f,1.0f };
	bool isTransition_ = false;
	bool isTransitionEnd_ = false;
	float transitionTimer_ = 0.0f;

	//次のシーン
	uint32_t nextScene = GameClearScene;
};

