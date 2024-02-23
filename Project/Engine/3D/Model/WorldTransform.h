#pragma once
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"
#include "Engine/Math/Quaternion.h"
#include <memory>

class WorldTransform
{
public:
	void Initialize();

	void TransferMatrix();

	void UpdateMatrixFromEuler();

	void UpdateMatrixFromQuaternion();

	void SetParent(const WorldTransform* parent);

	void UnsetParent();

	UploadBuffer* GetConstantBuffer() const { return constBuff_.get(); };

	WorldTransform& operator=(const WorldTransform& rhs)
	{
		if (this != &rhs)
		{
			scale_ = rhs.scale_;
			rotation_ = rhs.rotation_;
			quaternion_ = rhs.quaternion_;
			translation_ = rhs.translation_;
			matWorld_ = rhs.matWorld_;
			parent_ = rhs.parent_;
		}
		return *this;
	}

private:
	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

public:
	Vector3 scale_ = { 1.0f,1.0f,1.0f };

	Vector3 rotation_ = { 0.0f,0.0f,0.0f };

	Vector3 translation_ = { 0.0f,0.0f,0.0f };

	Quaternion quaternion_ = { 0.0f,0.0f,0.0f,1.0f };

	Matrix4x4 matWorld_{};

	const WorldTransform* parent_ = nullptr;
};

