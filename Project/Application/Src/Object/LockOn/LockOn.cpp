#include "LockOn.h"
#include <algorithm>

void LockOn::Initialize() {
	//インスタンスを取得
	input_ = Input::GetInstance();
	//テクスチャ読み込み
	TextureManager::Load("Reticle.png");
	//スプライトの生成
	lockOnMark_.reset(Sprite::Create("Reticle.png", {0.0f,0.0f}));
	lockOnMark_->SetAnchorPoint({ 0.5f,0.5f });
}

void LockOn::Update(const std::list<std::unique_ptr<Enemy>>& enemies,const Camera& camera) {
	if (isManualLockOn_) {
		ManualLockOn(enemies, camera);
	}
	else {
		AutoLockOn(enemies, camera);
	}

	//ロックオン状態なら
	if (target_) {
		//ロックオンマークの座標計算
		Vector3 positionWorld = target_->GetCenterPosition();
		// ビューポート行列
		Matrix4x4 matViewport = Mathf::MakeViewportMatrix(0, 0, Application::kClientWidth, Application::kClientHeight, 0, 1);
		// ビュー行列とプロジェクション行列、ビューポート行列を合成する
		Matrix4x4 matViewProjectionViewport = camera.matView_ * camera.matProjection_ * matViewport;
		//ワールド座標からスクリーン座標に変換
		Vector3 positionScreen = Mathf::Transform(positionWorld, matViewProjectionViewport);
		//Vector2に格納
		Vector2 positionScreenV2 = { positionScreen.x,positionScreen.y };
		//スプライトの座標を設定
		lockOnMark_->SetPosition(positionScreenV2);
	}

	ImGui::Begin("LockOn");
	ImGui::Checkbox("IsManualLockOn", &isManualLockOn_);
	ImGui::End();
}

void LockOn::Draw() {
	if (target_) {
		lockOnMark_->Draw();
	}
}

bool LockOn::InRange(const Camera& camera) {
	//敵のロックオン座標取得
	Vector3 positionWorld = target_->GetCenterPosition();
	//ワールド→ビュー座標変換
	Vector3 positionView = Mathf::Transform(positionWorld, camera.matView_);

	//距離条件チェック
	if (minDistance_ <= positionView.z && positionView.z <= maxDistance_) {
		////カメラ前方との角度を計算
		//float arcTangent = std::atan2(std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y), positionView.z);
		float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, positionView);
		float length = Mathf::Length(positionView);
		float angle = std::acos(dot / length);

		//角度条件チェック(コーンに収まっているか)
		if (std::abs(angle) <= angleRange_) {
			return false;
		}
	}

	return true;
}

Vector3 LockOn::GetTargetPosition() const {
	if (target_) {
		return target_->GetCenterPosition();
	}
	return Vector3();
}

void LockOn::ManualLockOn(const std::list<std::unique_ptr<Enemy>>& enemies, const Camera& camera) {
	//ロックオン状態なら
	if (target_) {
		//ロックオン解除処理
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_THUMB)) {
			//ロックオンを外す
			target_ = nullptr;
		}
		else if (InRange(camera)) {
			//ロックオンを外す
			target_ = nullptr;
		}
		else if (target_->GetIsDead()) {
			target_ = nullptr;
		}

		//ターゲットを変える
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_B)) {
			SearchLockOnTarget(enemies, camera);
			targetIndex_++;
			if (targetIndex_ >= targets_.size()) {
				targetIndex_ = 0;
				target_ = nullptr;
				target_ = targets_[targetIndex_].second;
			}
			else {
				target_ = nullptr;
				target_ = targets_[targetIndex_].second;
			}
		}
	}
	else {
		//ロックオン対象の検索
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_THUMB)) {
			SearchLockOnTarget(enemies, camera);
		}
	}
}

void LockOn::AutoLockOn(const std::list<std::unique_ptr<Enemy>>& enemies, const Camera& camera) {
	//ロックオン切り替え処理
	if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_THUMB)) {
		if (isLockOn_) {
			//ロックオンを外す
			isLockOn_ = false;
			target_ = nullptr;
		}
		else {
			isLockOn_ = true;
		}
	}

	if (target_ && InRange(camera)) {
		//ロックオンを外す
		target_ = nullptr;
	}
	else if (target_ && target_->GetIsDead()) {
		//ロックオンを外す
		target_ = nullptr;
	}

	if (isLockOn_) {
		//ロックオン対象の検索
		SearchLockOnTarget(enemies, camera);
	}
}

void LockOn::SearchLockOnTarget(const std::list<std::unique_ptr<Enemy>>& enemies, const Camera& camera) {
	//リストのリセット
	targets_.clear();

	//すべての敵に対して順にロックオン判定
	for (const std::unique_ptr<Enemy>& enemy : enemies) {
		//敵のロックオン座標取得
		Vector3 positionWorld = enemy->GetCenterPosition();
		//ワールド→ビュー座標変換
		Vector3 positionView = Mathf::Transform(positionWorld, camera.matView_);

		//距離条件チェック
		if (minDistance_ <= positionView.z && positionView.z <= maxDistance_) {
			//カメラ前方との角度を計算
			//float arcTangent = std::atan2(std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y), positionView.z);
			float dot = Mathf::Dot({ 0.0f,0.0f,1.0f }, positionView);
			float length = Mathf::Length(positionView);
			float angle = std::acos(dot / length);

			//角度条件チェック(コーンに収まっているか)
			if (std::abs(angle) <= angleRange_ && enemy->GetIsDead() == false) {
				targets_.emplace_back(std::make_pair(positionView.z, enemy.get()));
			}
		}
	}

	//ターゲット対象をリセット
	target_ = nullptr;
	if (targets_.size() != 0) {
		//距離で昇順にソート
		//targets.sort([](auto& pair1, auto& pair2) {return pair1.first < pair2.first; });
		std::sort(targets_.begin(), targets_.end(), [](auto& pair1, auto& pair2) {return pair1.first < pair2.first; });
		//ソートの結果一番近い敵をロックオン対象とする
		target_ = targets_.front().second;
	}
}