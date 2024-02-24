#pragma once
#include "Engine/Components/Collision/Collider.h"
#include "Engine/3D/Camera/Camera.h"

class Boss;

class IBossState
{
public:
	virtual ~IBossState() = default;

	virtual void Initialize(Boss* pBoss) = 0;

	virtual void Update(Boss* pBoss) = 0;

	virtual void Draw(Boss* pBoss, const Camera& camera) = 0;

	virtual void OnCollision(Collider* collider) = 0;

	virtual const WorldTransform& GetWorldTransform() const = 0;

private:

};