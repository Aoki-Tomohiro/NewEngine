#pragma once
#include "Collider.h"
#include <list>

class CollisionManager
{
public:
	void ClearColliderList();

	void SetColliderList(Collider* collider);

	void CheckAllCollisions();

private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

	bool CheckCollisionSphere(const Sphere& sphereA, const Sphere& sphereB);

	bool CheckCollisionSphereAABB(const Sphere& sphere, const AABB& aabb);

	bool CheckCollisionAABB(const AABB& aabbA, const AABB& aabbB);

	bool CheckCollisionAABBOBB(const AABB& aabb, const OBB& obb);

	float LenSegOnSeparateAxis(Vector3* Sep, Vector3* e1, Vector3* e2, Vector3* e3);

private:
	std::list<Collider*> colliders_{};
};

