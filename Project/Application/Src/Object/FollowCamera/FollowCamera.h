#pragma once
#include "Engine/Components/Input/Input.h"
#include "Engine/3D/Model/WorldTransform.h"
#include "Engine/3D/Camera/Camera.h"
#include "Engine/Base/ImGuiManager.h"
#include "Engine/Math/MathFunction.h"

class LockOn;

class FollowCamera
{
public:
	void Initialize();

	void Update();

	void SetTarget(const WorldTransform* target);

	const Camera& GetCamera() { return camera_; };

	void SetLockOn(const LockOn* lockOn) { lockOn_ = lockOn; };

private:
	Vector3 Offset();

	void Reset();

private:
	//入力クラス
	Input* input_ = nullptr;

	//カメラ
	Camera camera_{};

	//目標角度
	float destinationAngleX_ = 0.0f;
	float destinationAngleY_ = 0.0f;

	//追従対象の残像座標
	Vector3 interTarget_{};

	//追従対象
	const WorldTransform* target_{};

	//ロックオン
	const LockOn* lockOn_ = nullptr;
};

