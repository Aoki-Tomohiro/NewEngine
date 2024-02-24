#pragma once
#include "Engine/Framework/Scene/IScene.h"
#include "Engine/Framework/Object/GameObjectManager.h"
#include "Engine/Base/Renderer.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Components/Audio/Audio.h"
#include "Engine/3D/Model/ModelManager.h"
#include "Engine/2D/Sprite.h"
#include "Engine/Math/MathFunction.h"

#include "Application/Src/Object/Skydome/Skydome.h"
#include "Application/Src/Object/Ground/Ground.h"
#include "Application/Src/Object/Player/Player.h"
#include "Application/Src/Object/Boss/Boss.h"

class GameTitleScene : public IScene
{
public:
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

	//カメラ
	Camera camera_{};

	//プレイヤー
	std::unique_ptr<Model> playerModelHead_ = nullptr;
	std::unique_ptr<Model> playerModelBody_ = nullptr;
	std::unique_ptr<Model> playerModelL_Arm_ = nullptr;
	std::unique_ptr<Model> playerModelR_Arm_ = nullptr;
	WorldTransform playerWorldTransforms[5]{};

	//ボス
	std::unique_ptr<Model> bossModel_ = nullptr;
	WorldTransform bossWorldTransform_{};

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

	//タイトルのスプライト
	std::unique_ptr<Sprite> titleSprite_ = nullptr;
	std::unique_ptr<Sprite> pressASprite_ = nullptr;

	//BGM
	uint32_t bgmHandle_ = 0;
};

