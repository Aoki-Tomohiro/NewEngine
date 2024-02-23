#include "GamePlayScene.h"
#include "Engine/Framework/Scene/SceneManager.h"

void GamePlayScene::Initialize()
{
	renderer_ = Renderer::GetInstance();

	input_ = Input::GetInstance();

	audio_ = Audio::GetInstance();

	//カメラの初期化
	camera_.Initialize();

	//衝突マネージャーの作成
	collisionManager_ = std::make_unique<CollisionManager>();

	//プレイヤーの作成
	playerModel_.reset(ModelManager::CreateFromOBJ("Player", Opaque));
	std::vector<Model*> playerModels = { playerModel_.get() };
	player_ = std::make_unique<Player>();
	player_->Initialize(playerModels);
	player_->SetCamera(&camera_);

	//敵の作成
	modelEnemyBody_.reset(ModelManager::CreateFromOBJ("Enemy_Body", Opaque));
	modelEnemyL_arm_.reset(ModelManager::CreateFromOBJ("Enemy_L_arm", Opaque));
	modelEnemyR_arm_.reset(ModelManager::CreateFromOBJ("Enemy_R_arm", Opaque));
	std::vector<Model*> enemyModels = { modelEnemyBody_.get(),modelEnemyL_arm_.get(),modelEnemyR_arm_.get() };
	enemy_ = std::make_unique <Enemy>();
	enemy_->Initialize(enemyModels);

	//天球の作成
	skydomeModel_.reset(ModelManager::CreateFromOBJ("Skydome", Opaque));
	skydomeModel_->SetEnableLighting(false);
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(skydomeModel_.get());

	//追従カメラの作成
	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Initialize();
	followCamera_->SetTarget(&player_->GetWorldTransform());

	//床の作成
	floorModel_.reset(ModelManager::CreateFromOBJ("Floor", Opaque));
	for (uint32_t i = 0; i < kFloorMax; ++i) {
		Floor* floor = new Floor();
		if (i % 2 == 1) {
			floor->Initialize(floorModel_.get(), { 0.0f,0.0f,i * 20.0f }, { i * 0.1f,0.0f,0.0f });
		}
		else {
			floor->Initialize(floorModel_.get(), { 0.0f,0.0f,i * 20.0f }, { 0.0f,0.0f,0.0f });
		}
		floors_.push_back(std::unique_ptr<Floor>(floor));
	}
	//敵の親を設定
	enemy_->SetParent(&floors_[2]->GetWorldTransform());

	//ゴールの作成
	goalModel_.reset(ModelManager::CreateFromOBJ("Goal", Opaque));
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(goalModel_.get(), { 0.0f,0.0f,4 * 20.0f });
}

void GamePlayScene::Finalize()
{

}

void GamePlayScene::Update()
{
	//プレイヤーの更新
	player_->Update();

	//敵の更新
	enemy_->Update();

	//天球の更新
	skydome_->Update();

	//床の更新
	for (std::unique_ptr<Floor>& floor : floors_) {
		floor->Update();
	}

	//ゴールの更新
	goal_->Update();

	//追従カメラの更新
	followCamera_->Update();
	camera_.translation_ = followCamera_->GetCamera().translation_;
	camera_.rotation_ = followCamera_->GetCamera().rotation_;

	//カメラの更新
	camera_.UpdateMatrix();

	//衝突判定
	collisionManager_->ClearColliderList();
	collisionManager_->SetColliderList(player_.get());
	collisionManager_->SetColliderList(enemy_.get());
	for (std::unique_ptr<Floor>& floor : floors_) {
		collisionManager_->SetColliderList(floor.get());
	}
	collisionManager_->SetColliderList(goal_.get());
	collisionManager_->CheckAllCollisions();
}

void GamePlayScene::Draw() 
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
	//プレイヤーの描画
	player_->Draw(camera_);

	//敵の描画
	enemy_->Draw(camera_);

	//天球の描画
	skydome_->Draw(camera_);

	//床の描画
	for (std::unique_ptr<Floor>& floor : floors_) {
		floor->Draw(camera_);
	}

	//ゴールの描画
	goal_->Draw(camera_);

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

void GamePlayScene::DrawUI()
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}