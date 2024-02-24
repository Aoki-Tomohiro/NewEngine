#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include <cmath>

namespace Mathf
{
	float Dot(const Vector3& v1, const Vector3& v2);

	float Length(const Vector3& v);

	float Lerp(const float& v1, const float& v2, float t);

	float LerpShortAngle(const float& a, const float& b, float t);

	float EaseInSine(float x);

	float EaseInOutSine(float x);

	float Norm(const Quaternion& quaternion);

	Vector3 Normalize(const Vector3& v);

	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	Vector3 Transform(const Vector3& v, const Matrix4x4& m);

	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

	Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

	Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion);

	Matrix4x4 Inverse(const Matrix4x4& m);

	Matrix4x4 Transpose(const Matrix4x4& m);

	Matrix4x4 MakeIdentity4x4();

	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	Matrix4x4 MakeRotateXMatrix(float radian);

	Matrix4x4 MakeRotateYMatrix(float radian);

	Matrix4x4 MakeRotateZMatrix(float radian);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& quaternion, const Vector3& translation);

	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);

	Matrix4x4 MakeRotateAxisAngle(Vector3 axis, float angle);

	Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);

	Quaternion IdentityQuaternion();

	Quaternion Conjugate(const Quaternion& quaternion);

	Quaternion Normalize(const Quaternion& quaternion);

	Quaternion Inverse(const Quaternion& quaternion);

	Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);

	Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);
}

