#pragma once
#include "Engine/2D/Sprite.h"
#include "Engine/Components/Input/Input.h"
#include "Application/Src/Object/Boss/Boss.h"

class LockOn
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="boss"></param>
	/// <param name="camera"></param>
	void Update(const Boss* boss, const Camera& camera);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ロックオン対象の座標を取得
	/// </summary>
	/// <returns></returns>
	Vector3 GetTargetPosition() const;

	/// <summary>
	/// ロックオン中かどうか
	/// </summary>
	/// <returns></returns>
	bool ExistTarget() const { return target_ ? true : false; };

private:
	bool InRange(const Camera& camera);

	void SearchLockOnTarget(const Boss* boss, const Camera& camera);

private:
	Input* input_ = nullptr;

	std::unique_ptr<Sprite> lockOnMark_ = nullptr;

	const Boss* target_ = nullptr;

	float minDistance_ = 10.0f;

	float maxDistance_ = 80.0f;

	const float kDegreeToRadian = 3.14159265358979323846f / 180.0f;
	float angleRange_ = 20.0f * kDegreeToRadian;
};

