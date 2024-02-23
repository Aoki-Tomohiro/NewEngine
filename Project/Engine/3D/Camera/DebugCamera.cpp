#include "DebugCamera.h"
#include "Engine/Base/ImGuiManager.h"
#include "Engine/Math/MathFunction.h"

void DebugCamera::Initialize()
{
	//入力クラスのインスタンスを取得
	input_ = Input::GetInstance();
	//ワールドトランスフォームの初期設定
	worldTransform_.translation_ = { 0.0f,0.0f,-50.0f };
}

void DebugCamera::Update() {
	if (target_) {
		//追従対象からカメラまでのオフセット(0度の時の値)
		Vector3 offset = offset_;
		//オフセットをカメラの回転に合わせて回転させる
		offset = Mathf::TransformNormal(offset, matRot_);
		//座標をコピーしてオフセット分ずらす
		worldTransform_.translation_ = target_->translation_ + offset;
	}
	//追従対象からカメラまでのオフセット(0度の時の値)
	Vector3 offset = offset_;
	//オフセットをカメラの回転に合わせて回転させる
	offset = Mathf::TransformNormal(offset, matRot_);
	//座標をコピーしてオフセット分ずらす
	worldTransform_.translation_ = offset;

	//座標から平行移動行列を計算する
	Matrix4x4 translateMatrix = Mathf::MakeTranslateMatrix(worldTransform_.translation_);
	//ワールド行列を計算
	worldTransform_.matWorld_ = matRot_ * translateMatrix;
	//ビュー行列の計算
	viewProjection_.matView_ = Mathf::Inverse(translateMatrix) * Mathf::Inverse(matRot_);
	//プロジェクション行列の計算
	viewProjection_.matProjection_ = Mathf::MakePerspectiveFovMatrix(viewProjection_.fov_, viewProjection_.aspectRatio_, viewProjection_.nearClip_, viewProjection_.farClip_);

	//前後移動
	if (input_->GetWheel() > 0) {
		//速さ
		const float speed = 1.0f;
		//カメラの移動ベクトル
		Vector3 move = { 0.0f,0.0f,speed };
		//移動ベクトル分だけ座標を加算する
		offset_ += move;
	}
	else if (input_->GetWheel() < 0) {
		//速さ
		const float speed = -1.0f;
		//カメラの移動ベクトル
		Vector3 move = { 0.0f,0.0f,speed };
		//移動ベクトル分だけ座標を加算する
		offset_ += move;
	}

	//左右移動
	if (input_->IsPushKey(DIK_D)) {
		//速さ
		const float speed = -0.2f;
		//カメラの移動ベクトル
		Vector3 move = { speed,0.0f,0.0f };
		//移動ベクトル分だけ座標を加算する
		offset_ += move;
	}
	else if (input_->IsPushKey(DIK_A)) {
		//速さ
		const float speed = 0.2f;
		//カメラの移動ベクトル
		Vector3 move = { speed,0.0f,0.0f };
		//移動ベクトル分だけ座標を加算する
		offset_ += move;
	}

	//上下移動
	if (input_->IsPushKey(DIK_W)) {
		//速さ
		const float speed = -0.2f;
		//カメラの移動ベクトル
		Vector3 move = { 0.0f,speed,0.0f };
		//移動ベクトル分だけ座標を加算する
		offset_ += move;
	}
	else if (input_->IsPushKey(DIK_S)) {
		//速さ
		const float speed = 0.2f;
		//カメラの移動ベクトル
		Vector3 move = { 0.0f,speed,0.0f };
		//移動ベクトル分だけ座標を加算する
		offset_ += move;
	}

	//X軸回転
	if (input_->IsPushKey(DIK_UP)) {
		//速さ
		const float speed = 0.02f;
		//追加回転分の回転行列を生成
		Matrix4x4 matRotDelta = Mathf::MakeIdentity4x4();
		matRotDelta *= Mathf::MakeRotateXMatrix(speed);
		//累積の回転行列を合成
		matRot_ *= matRotDelta;
	}
	else if (input_->IsPushKey(DIK_DOWN)) {
		//速さ
		const float speed = -0.02f;
		//追加回転分の回転行列を生成
		Matrix4x4 matRotDelta = Mathf::MakeIdentity4x4();
		matRotDelta *= Mathf::MakeRotateXMatrix(speed);
		//累積の回転行列を合成
		matRot_ *= matRotDelta;
	}

	//Y軸回転
	if (input_->IsPushKey(DIK_LEFT)) {
		//速さ
		const float speed = 0.02f;
		//追加回転分の回転行列を生成
		Matrix4x4 matRotDelta = Mathf::MakeIdentity4x4();
		matRotDelta *= Mathf::MakeRotateYMatrix(speed);
		//累積の回転行列を合成
		matRot_ *= matRotDelta;
	}
	else if (input_->IsPushKey(DIK_RIGHT)) {
		//速さ
		const float speed = -0.02f;
		//追加回転分の回転行列を生成
		Matrix4x4 matRotDelta = Mathf::MakeIdentity4x4();
		matRotDelta *= Mathf::MakeRotateYMatrix(speed);
		//累積の回転行列を合成
		matRot_ *= matRotDelta;
	}
}