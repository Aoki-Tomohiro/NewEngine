#include "LockOn.h"
#include "Engine/Base/Application.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Math/MathFunction.h"

void LockOn::Initialize()
{
	//インスタンスを取得
	input_ = Input::GetInstance();
	//テクスチャ読み込み
	TextureManager::Load("Reticle.png");
	//スプライトの生成
	lockOnMark_.reset(Sprite::Create("Reticle.png", { 0.0f,0.0f }));
	lockOnMark_->SetAnchorPoint({ 0.5f,0.5f });
}

void LockOn::Update(const Boss* boss, const Camera& camera)
{
	//ロックオン状態なら
	if (target_)
	{
		//ロックオンマークの座標計算
		Vector3 positionWorld = target_->GetWorldPosition();
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

		//ロックオン解除処理
		if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_THUMB))
		{
			//ロックオンを外す
			target_ = nullptr;
		}
		else if (InRange(camera))
		{
			//ロックオンを外す
			target_ = nullptr;
		}
	}
	else
	{
		//ロックオン対象の検索
		if (input_->IsControllerConnected())
		{
			if (input_->IsPressButtonEnter(XINPUT_GAMEPAD_RIGHT_THUMB))
			{
				SearchLockOnTarget(boss, camera);
			}
		}
	}
}

void LockOn::Draw()
{
	if (target_)
	{
		lockOnMark_->Draw();
	}
}

Vector3 LockOn::GetTargetPosition() const
{
	if (target_)
	{
		return target_->GetWorldPosition();
	}
	return Vector3();
}

bool LockOn::InRange(const Camera& camera)
{
	//敵のロックオン座標取得
	Vector3 positionWorld = target_->GetWorldPosition();
	//ワールド→ビュー座標変換
	Vector3 positionView = Mathf::Transform(positionWorld, camera.matView_);

	//距離条件チェック
	if (minDistance_ <= positionView.z && positionView.z <= maxDistance_)
	{
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

void LockOn::SearchLockOnTarget(const Boss* boss, const Camera& camera) {
	//敵のロックオン座標取得
	Vector3 positionWorld = boss->GetWorldPosition();
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
		if (std::abs(angle) <= angleRange_) {
			target_ = nullptr;
			target_ = boss;
		}
	}
}