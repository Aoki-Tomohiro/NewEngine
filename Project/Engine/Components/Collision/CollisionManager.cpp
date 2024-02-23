#include "CollisionManager.h"
#include "CollisionConfig.h"
#include "Engine/Math/MathFunction.h"
#include <algorithm>

void CollisionManager::ClearColliderList()
{
	//コライダーリストをクリア
	colliders_.clear();
}

void CollisionManager::SetColliderList(Collider* collider)
{
	//コライダーリストに登録
	colliders_.push_back(collider);
}

void CollisionManager::CheckAllCollisions()
{
	//リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA)
	{
		//イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;
		//イテレータBはイテレータAの次の要素から回す(重複判定を回避)
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;
		for (; itrB != colliders_.end(); ++itrB)
		{
			//イテレータBからコライダーBを取得する
			Collider* colliderB = *itrB;
			//ベアの当たり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB)
{
	//衝突フィルタリング
	if ((colliderA->GetCollisionAttribute() & colliderB->GetCollisionMask()) == 0 ||
		(colliderB->GetCollisionAttribute() & colliderA->GetCollisionMask()) == 0)
	{
		return;
	}

	//球と球の判定
	if ((colliderA->GetCollisionPrimitive() & kCollisionPrimitiveSphere) && (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveSphere))
	{
		//コライダーAのワールド座標を取得
		Vector3 posA = colliderA->GetWorldPosition();
		//コライダーBのワールド座標を取得
		Vector3 posB = colliderB->GetWorldPosition();
		//コライダーAのSphereを作成
		Sphere sphereA = { .center{posA},.radius{colliderA->GetRadius()} };
		//コライダーBのSphereを作成
		Sphere sphereB = { .center{posB},.radius{colliderB->GetRadius()} };
		//衝突判定
		if (CheckCollisionSphere(sphereA, sphereB))
		{
			//コライダーAの衝突時コールバックを呼び出す
			colliderA->OnCollision(colliderB);
			//コライダーBの衝突時コールバックを呼び出す
			colliderB->OnCollision(colliderA);
		}
	}

	//AABBとAABBの判定
	if ((colliderA->GetCollisionPrimitive() & kCollisionPrimitiveAABB) && (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveAABB))
	{
		//コライダーAのワールド座標を取得
		Vector3 posA = colliderA->GetWorldPosition();
		//コライダーBのワールド座標を取得
		Vector3 posB = colliderB->GetWorldPosition();
		//コライダーAのAABBを取得
		AABB aabbA = { .min{posA + colliderA->GetAABB().min},.max{posA + colliderA->GetAABB().max} };
		//コライダーBのAABBを取得
		AABB aabbB = { .min{posB + colliderB->GetAABB().min},.max{posB + colliderB->GetAABB().max} };
		//衝突判定
		if (CheckCollisionAABB(aabbA, aabbB))
		{
			//コライダーAの衝突時コールバックを呼び出す
			colliderA->OnCollision(colliderB);
			//コライダーBの衝突時コールバックを呼び出す
			colliderB->OnCollision(colliderA);
		}
	}

	//球とAABBの判定
	if (((colliderA->GetCollisionPrimitive() & kCollisionPrimitiveSphere) != 0 && (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveAABB) != 0) ||
		((colliderA->GetCollisionPrimitive() & kCollisionPrimitiveAABB) != 0 && (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveSphere) != 0))
	{
		//コライダーAのワールド座標を取得
		Vector3 posA = colliderA->GetWorldPosition();
		//コライダーBのワールド座標を取得
		Vector3 posB = colliderB->GetWorldPosition();

		//コライダーAがSphereの場合
		if (colliderA->GetCollisionPrimitive() & kCollisionPrimitiveSphere)
		{
			//コライダーAのSphereを作成
			Sphere sphere = { .center{posA},.radius{colliderA->GetRadius()} };
			//コライダーBのAABBを取得
			AABB aabb = { .min{posB + colliderB->GetAABB().min},.max{posB + colliderB->GetAABB().max} };
			//衝突判定
			if (CheckCollisionSphereAABB(sphere, aabb))
			{
				//コライダーAの衝突時コールバックを呼び出す
				colliderA->OnCollision(colliderB);
				//コライダーBの衝突時コールバックを呼び出す
				colliderB->OnCollision(colliderA);
			}
		}
		else if (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveSphere)
		{
			//コライダーAのSphereを作成
			Sphere sphere = { .center{posB},.radius{colliderB->GetRadius()} };
			//コライダーBのAABBを取得
			AABB aabb = { .min{posA + colliderA->GetAABB().min},.max{posA + colliderA->GetAABB().max} };
			//衝突判定
			if (CheckCollisionSphereAABB(sphere, aabb))
			{
				//コライダーAの衝突時コールバックを呼び出す
				colliderA->OnCollision(colliderB);
				//コライダーBの衝突時コールバックを呼び出す
				colliderB->OnCollision(colliderA);
			}
		}
	}

	//OBBとAABBの判定
	if (((colliderA->GetCollisionPrimitive() & kCollisionPrimitiveOBB) != 0 && (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveAABB) != 0) ||
		((colliderA->GetCollisionPrimitive() & kCollisionPrimitiveAABB) != 0 && (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveOBB) != 0))
	{
		//コライダーAがAABBの場合
		if (colliderA->GetCollisionPrimitive() & kCollisionPrimitiveAABB)
		{
			//コライダーAのAABBを取得
			AABB aabb = { .min{colliderA->GetWorldPosition() + colliderA->GetAABB().min},.max{colliderA->GetWorldPosition() + colliderA->GetAABB().max}, };
			//コライダーBのOBBを取得
			OBB obb = colliderB->GetOBB();

			//衝突判定
			if (CheckCollisionAABBOBB(aabb, obb))
			{
				//コライダーAの衝突時コールバックを呼び出す
				colliderA->OnCollision(colliderB);
				//コライダーBの衝突時コールバックを呼び出す
				colliderB->OnCollision(colliderA);
			}
		}
		//ColliderBがAABBの場合
		else if (colliderB->GetCollisionPrimitive() & kCollisionPrimitiveAABB)
		{
			//コライダーBのAABBを取得
			AABB aabb = { .min{colliderB->GetWorldPosition() + colliderB->GetAABB().min},.max{colliderB->GetWorldPosition() + colliderB->GetAABB().max}, };
			//コライダーBのOBBを取得
			OBB obb = colliderA->GetOBB();

			//衝突判定
			if (CheckCollisionAABBOBB(aabb, obb))
			{
				//コライダーAの衝突時コールバックを呼び出す
				colliderA->OnCollision(colliderB);
				//コライダーBの衝突時コールバックを呼び出す
				colliderB->OnCollision(colliderA);
			}
		}
	}
}

bool CollisionManager::CheckCollisionSphere(const Sphere& sphereA, const Sphere& sphereB)
{
	//コライダーAとコライダーBの距離を計算
	float distance = Mathf::Length(sphereA.center - sphereB.center);
	//球と球の交差判定
	if (distance <= sphereA.radius + sphereB.radius)
	{
		return true;
	}
	return false;
}

bool CollisionManager::CheckCollisionSphereAABB(const Sphere& sphere, const AABB& aabb)
{
	//最近接点を求める
	Vector3 closestPoint{
		std::clamp(sphere.center.x,aabb.min.x,aabb.max.x),
		std::clamp(sphere.center.y,aabb.min.y,aabb.max.y),
		std::clamp(sphere.center.z,aabb.min.z,aabb.max.z)
	};
	//最近接点と球の中心との距離を求める
	float distance = Mathf::Length(closestPoint - sphere.center);
	//距離が半径よりも小さければ衝突
	if (distance <= sphere.radius)
	{
		return true;
	}
	return false;
}

bool CollisionManager::CheckCollisionAABB(const AABB& aabbA, const AABB& aabbB)
{
	if (aabbA.min.x <= aabbB.max.x && aabbA.max.x >= aabbB.min.x &&
		aabbA.min.y <= aabbB.max.y && aabbA.max.y >= aabbB.min.y &&
		aabbA.min.z <= aabbB.max.z && aabbA.max.z >= aabbB.min.z)
	{
		return true;
	}
	return false;
}

bool CollisionManager::CheckCollisionAABBOBB(const AABB& aabb, const OBB& obb)
{
	Vector3 aabbCenter = (aabb.min + aabb.max) * 0.5f;

	float aabbHalfSize[3] = {
	0.5f * (aabb.max.x - aabb.min.x),
	0.5f * (aabb.max.y - aabb.min.y),
	0.5f * (aabb.max.z - aabb.min.z),
	};

	Vector3 NAe1 = { 1.0f,0.0f,0.0f };
	Vector3 Ae1 = NAe1 * aabbHalfSize[0];
	Vector3 NAe2 = { 0.0f,1.0f,0.0f };
	Vector3 Ae2 = NAe2 * aabbHalfSize[1];
	Vector3 NAe3 = { 0.0f,0.0f,1.0f };
	Vector3 Ae3 = NAe3 * aabbHalfSize[2];

	Vector3 NBe1 = obb.orientations[0];
	Vector3 Be1 = NBe1 * obb.size.x;
	Vector3 NBe2 = obb.orientations[1];
	Vector3 Be2 = NBe2 * obb.size.y;
	Vector3 NBe3 = obb.orientations[2];
	Vector3 Be3 = NBe3 * obb.size.z;

	Vector3 Interval = aabbCenter - obb.center;

	//文理軸 Ae1
	float rA = Mathf::Length(Ae1);
	float rB = LenSegOnSeparateAxis(&NAe1, &Be1, &Be2, &Be3);
	float L = fabs(Mathf::Dot(Interval, NAe1));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : Ae2
	rA = Mathf::Length(Ae2);
	rB = LenSegOnSeparateAxis(&NAe2, &Be1, &Be2, &Be3);
	L = fabs(Mathf::Dot(Interval, NAe2));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : Ae3
	rA = Mathf::Length(Ae3);
	rB = LenSegOnSeparateAxis(&NAe3, &Be1, &Be2, &Be3);
	L = fabs(Mathf::Dot(Interval, NAe3));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : Be1
	rA = LenSegOnSeparateAxis(&NBe1, &Ae1, &Ae2, &Ae3);
	rB = Mathf::Length(Be1);
	L = fabs(Mathf::Dot(Interval, NBe1));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : Be2
	rA = LenSegOnSeparateAxis(&NBe2, &Ae1, &Ae2, &Ae3);
	rB = Mathf::Length(Be2);
	L = fabs(Mathf::Dot(Interval, NBe2));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : Be3
	rA = LenSegOnSeparateAxis(&NBe3, &Ae1, &Ae2, &Ae3);
	rB = Mathf::Length(Be3);
	L = fabs(Mathf::Dot(Interval, NBe3));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C11
	Vector3 Cross = Mathf::Cross(NAe1, NBe1);
	rA = LenSegOnSeparateAxis(&Cross, &Ae2, &Ae3, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be2, &Be3, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C12
	Cross = Mathf::Cross(NAe1, NBe2);
	rA = LenSegOnSeparateAxis(&Cross, &Ae2, &Ae3, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be3, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C13
	Cross = Mathf::Cross(NAe1, NBe3);
	rA = LenSegOnSeparateAxis(&Cross, &Ae2, &Ae3, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be2, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C21
	Cross = Mathf::Cross(NAe2, NBe1);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae3, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be2, &Be3, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C22
	Cross = Mathf::Cross(NAe2, NBe2);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae3, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be3, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C23
	Cross = Mathf::Cross(NAe2, NBe3);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae3, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be2, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C31
	Cross = Mathf::Cross(NAe3, NBe1);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae2, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be2, &Be3, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C32
	Cross = Mathf::Cross(NAe3, NBe2);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae2, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be3, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	// 分離軸 : C33
	Cross = Mathf::Cross(NAe3, NBe3);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae2, 0);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be2, 0);
	L = fabs(Mathf::Dot(Interval, Cross));
	if (L > rA + rB)
	{
		return false;
	}

	return true;
}

float CollisionManager::LenSegOnSeparateAxis(Vector3* Sep, Vector3* e1, Vector3* e2, Vector3* e3)
{
	float r1 = fabs(Mathf::Dot(*Sep, *e1));
	float r2 = fabs(Mathf::Dot(*Sep, *e2));
	float r3 = e3 ? (fabs(Mathf::Dot(*Sep, *e3))) : 0;
	return r1 + r2 + r3;
}