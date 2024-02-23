#pragma once
#include "Engine/3D/Model/WorldTransform.h"
#include "Engine/Components/Input/Input.h"
#include "Camera.h"

class DebugCamera {
public:
	void Initialize();

	void Update();

	const Camera& GetViewProjection() { return viewProjection_; };

	void SetTarget(const WorldTransform* target) { target_ = target; };

private:
	//入力クラス
	Input* input_ = nullptr;

	//ワールドトランスフォーム
	WorldTransform worldTransform_{};

	//カメラ
	Camera viewProjection_{};

	//オフセット
	Vector3 offset_ = { 0.0f,0.0f,-50.0f };

	//累積回転行列
	Matrix4x4 matRot_{};

	//追従対象
	const WorldTransform* target_ = nullptr;
};

