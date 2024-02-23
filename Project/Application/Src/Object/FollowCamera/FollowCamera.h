#pragma once
#include "Engine/3D/Camera/Camera.h"
#include "Engine/3D/Model/WorldTransform.h"
#include "Engine/Components/Input/Input.h"
#include "Engine/Math/MathFunction.h"

/// <summary>
/// 追従カメラ
/// </summary>
class FollowCamera {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 追従対象をセット
	/// </summary>
	/// <param name="target"></param>
	void SetTarget(const WorldTransform* target);

	/// <summary>
	/// カメラを取得
	/// </summary>
	/// <returns></returns>
	const Camera& GetCamera() const { return camera_; };

private:
	/// <summary>
	/// オフセット計算
	/// </summary>
	/// <returns></returns>
	Vector3 Offset();

	/// <summary>
	/// リセット
	/// </summary>
	void Reset();

	/// <summary>
	/// 追従対象のワールド座標を取得
	/// </summary>
	/// <returns></returns>
	Vector3 GetTargetWorldPosition();

private:
	//ビュープロジェクション
	Camera camera_{};
	//追従対象
	const WorldTransform* target_ = nullptr;
	//追従対象の残像座標
	Vector3 interTarget_{};
	//目標角度
	float destinationAngleX_ = 0.0f;
	float destinationAngleY_ = 0.0f;
};