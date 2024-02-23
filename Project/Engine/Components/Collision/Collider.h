#pragma once
#include "Engine/3D/Model/WorldTransform.h"
#include "Engine/Math/Sphere.h"
#include "Engine/Math/AABB.h"
#include "Engine/Math/OBB.h"

class Collider
{
public:
	virtual void OnCollision(Collider* collider) = 0;

	virtual const Vector3 GetWorldPosition() const = 0;

	virtual const WorldTransform& GetWorldTransform() const = 0;

	const float GetRadius() const { return radius_; };

	void SetRadius(float radius) { radius_ = radius; };

	const AABB& GetAABB() const { return aabb_; };

	void SetAABB(AABB& aabb) { aabb_ = aabb; };

	const OBB& GetOBB() const { return obb_; };

	void SetOBB(OBB& obb) { obb_ = obb; };

	const uint32_t GetCollisionAttribute() const { return collisionAttribute_; };

	void SetCollisionAttribute(uint32_t collisionAttribute) { collisionAttribute_ = collisionAttribute; };

	const uint32_t GetCollisionMask() const { return collisionMask_; };

	void SetCollisionMask(uint32_t collisionMask) { collisionMask_ = collisionMask; };

	const uint32_t GetCollisionPrimitive() const { return collisionPrimitive_; };

	void SetCollisionPrimitive(uint32_t collisionPrimitive) { collisionPrimitive_ = collisionPrimitive; };

private:
	float radius_ = 1.0f;

	AABB aabb_{ {-1.0f,-1.0f,-1.0f},{1.0f,1.0f,1.0f} };

	OBB obb_{ {0.0f,0.0f,0.0f},{{1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}},{1.0f,1.0f,1.0f} };

	uint32_t collisionAttribute_ = 0xffffffff;

	uint32_t collisionMask_ = 0xffffffff;

	uint32_t collisionPrimitive_ = 0b1;
};

