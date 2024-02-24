#pragma once
#include "Engine/Framework/Scene/IScene.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/Components/Collision/CollisionManager.h"
#include "Engine/Components/Particle/ParticleManager.h"
#include "Engine/3D/Model/ModelManager.h"
#include "Engine/2D/Sprite.h"
#include "Engine/Math/MathFunction.h"

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
		kGameClear,
		kGameOver,
	};

	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

	void DrawUI() override;

	void UpdateTransition();

private:
	Renderer* renderer_ = nullptr;

	Input* input_ = nullptr;

	Audio* audio_ = nullptr;

	//ゲームオブジェクトマネージャー
	GameObjectManager* gameObjectManager_ = nullptr;

	//パーティクル
	ParticleManager* particleManager_ = nullptr;

	//衝突マネージャー
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;

	//カメラ
	Camera camera_{};

	//ロックオン
	std::unique_ptr<LockOn> lockOn_ = nullptr;

	//追従カメラ
	std::unique_ptr<FollowCamera> followCamera_ = nullptr;

	//プレイヤー
	std::unique_ptr<Model> playerModelHead_ = nullptr;
	std::unique_ptr<Model> playerModelBody_ = nullptr;
	std::unique_ptr<Model> playerModelL_Arm_ = nullptr;
	std::unique_ptr<Model> playerModelR_Arm_ = nullptr;
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

	//トランジション関連
	std::unique_ptr<Sprite> transitionSprite_ = nullptr;
	Vector4 transitionSpriteColor_{ 0.0f,0.0f,0.0f,1.0f };
	float transitionTimer_ = 0;
	bool isFadeIn_ = false;
	bool isFadeOut_ = true;

	//BGM
	uint32_t bgmHandle_ = 0;

	//次のシーン
	NextScene nextScene_ = kGameClear;

	//ヒットストップ関連
	bool isStop_ = false;
	uint32_t stopTime_ = 2;
	uint32_t stopTimer_ = 0;

	//カメラシェイク関連
	bool cameraShakeEnable_ = false;
	const uint32_t kShakeTime = 20;
	uint32_t shakeTimer_ = 0;
	float shakeIntensityX = 0.6f;
	float shakeIntensityY = 0.6f;

	//ガイドのスプライト
	std::unique_ptr<Sprite> guideSprite_ = nullptr;
};

