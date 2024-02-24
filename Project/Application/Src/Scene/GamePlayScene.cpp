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

	//ロックオンの初期化
	lockOn_ = std::make_unique<LockOn>();
	lockOn_->Initialize();

	//プレイヤーの作成
	playerModel_.reset(ModelManager::CreateFromOBJ("Player", Opaque));
	weaponModel_.reset(ModelManager::CreateFromOBJ("Weapon", Opaque));
	std::vector<Model*> playerModels = { playerModel_.get(),weaponModel_.get() };
	player_ = std::make_unique<Player>();
	player_->Initialize(playerModels);
	player_->SetCamera(&camera_);
	player_->SetLockOn(lockOn_.get());

	//敵の作成
	modelEnemyBody_.reset(ModelManager::CreateFromOBJ("Enemy_Body", Opaque));
	modelEnemyL_arm_.reset(ModelManager::CreateFromOBJ("Enemy_L_arm", Opaque));
	modelEnemyR_arm_.reset(ModelManager::CreateFromOBJ("Enemy_R_arm", Opaque));
	for (int i = 0; i < kEnemyMax; i++) {
		AddEnemy(enemyPositions_[i], enemyVelocities_[i]);
	}

	//天球の作成
	skydomeModel_.reset(ModelManager::CreateFromOBJ("Skydome", Opaque));
	skydomeModel_->SetEnableLighting(false);
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(skydomeModel_.get());

	//追従カメラの作成
	followCamera_ = std::make_unique<FollowCamera>();
	followCamera_->Initialize();
	followCamera_->SetTarget(&player_->GetWorldTransform());
	followCamera_->SetLockOn(lockOn_.get());

	//床の作成
	floorModel_.reset(ModelManager::CreateFromOBJ("Floor", Opaque));
	for (uint32_t i = 0; i < kFloorMax; ++i) {
		Floor* floor = new Floor();
		if (i % 2 == 1) {
			floor->Initialize(floorModel_.get(), { 0.0f,0.0f,i * 20.0f }, { i * 0.06f,0.0f,0.0f });
		}
		else {
			floor->Initialize(floorModel_.get(), { 0.0f,0.0f,i * 20.0f }, { 0.0f,0.0f,0.0f });
		}
		floors_.push_back(std::unique_ptr<Floor>(floor));
	}

	//ゴールの作成
	goalModel_.reset(ModelManager::CreateFromOBJ("Goal", Opaque));
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(goalModel_.get(), { 0.0f,0.0f,6 * 20.0f + 5.0f });
}

void GamePlayScene::Finalize()
{

}

void GamePlayScene::Update()
{
	//プレイヤーの更新
	player_->Update();

	//敵の更新
	for (const std::unique_ptr<Enemy>& enemy : enemies_) {
		enemy->SetIsPlayerAttack(player_->GetIsAttack());
		enemy->Update();
	}

	//天球の更新
	skydome_->Update();

	//床の更新
	for (std::unique_ptr<Floor>& floor : floors_) {
		floor->Update();
	}

	//ゴールの更新
	goal_->Update();

	//ロックオン更新
	lockOn_->Update(enemies_, camera_);

	//追従カメラの更新
	followCamera_->Update();
	camera_ = followCamera_->GetCamera();
	//カメラの更新
	camera_.TransferMatrix();

	//衝突判定
	collisionManager_->ClearColliderList();
	collisionManager_->SetColliderList(player_.get());
	if (player_->GetWeapon()->GetIsAttack()) {
		collisionManager_->SetColliderList(player_->GetWeapon());
	}
	for (const std::unique_ptr<Enemy>& enemy : enemies_) {
		if (enemy->GetIsDead() == false) {
			collisionManager_->SetColliderList(enemy.get());
		}
	}
	for (std::unique_ptr<Floor>& floor : floors_) {
		collisionManager_->SetColliderList(floor.get());
	}
	collisionManager_->SetColliderList(goal_.get());
	collisionManager_->CheckAllCollisions();

	//リスタート処理
	if (player_->GetIsDead()) {
		Reset();
	}

	ImGui::Begin("GameScene");
	ImGui::Text("Left Stick : Move");
	ImGui::Text("Right Stick : CameraMove");
	ImGui::Text("Push Right Stick : LockOn");
	ImGui::Text("RB : Attack");
	ImGui::Text("A : Jump");
	ImGui::Text("X : Dash");
	ImGui::Text("B : TargetChange");
	ImGui::End();
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
	for (const std::unique_ptr<Enemy>& enemy : enemies_) {
		if (enemy->GetIsDeathAnimationEnd() == false) {
			enemy->Draw(camera_);
		}
	}

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

	player_->DrawParticle(camera_);

	//パーティクル描画後処理
	renderer_->PostDrawParticles();
#pragma endregion
}

void GamePlayScene::DrawUI()
{
#pragma region 前景スプライト描画
	//前景スプライト描画前処理
	renderer_->PreDrawSprites(kBlendModeNormal);

	lockOn_->Draw();

	//前景スプライト描画後処理
	renderer_->PostDrawSprites();
#pragma endregion
}

void GamePlayScene::AddEnemy(const Vector3& position, const Vector3& velocity) {
	std::vector<Model*> enemyModels = { modelEnemyBody_.get(),modelEnemyL_arm_.get(),modelEnemyR_arm_.get() };
	Enemy* enemy = new Enemy();
	enemy->SetStartPosition(position);
	enemy->SetVelocity(velocity);
	enemy->Initialize(enemyModels);
	enemies_.push_back(std::unique_ptr<Enemy>(enemy));
}

void GamePlayScene::Reset() {
	player_->SetIsDead(false);
	for (std::unique_ptr<Enemy>& enemy : enemies_) {
		enemy->Reset();
	}
}