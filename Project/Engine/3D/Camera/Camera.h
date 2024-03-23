#pragma once
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"
#include <memory>

class Camera
{
public:
	void Initialize();

	void UpdateViewMatrix();

	void UpdateProjectionMatrix();

	void UpdateMatrix();

	void TransferMatrix();

	UploadBuffer* GetConstantBuffer() const { return constBuff_.get(); };

	Camera& operator=(const Camera& rhs)
	{
		if (this != &rhs)
		{
			rotation_ = rhs.rotation_;
			translation_ = rhs.translation_;
			matView_ = rhs.matView_;
			matProjection_ = rhs.matProjection_;
			fov_ = rhs.fov_;
			aspectRatio_ = rhs.aspectRatio_;
			nearClip_ = rhs.nearClip_;
			farClip_ = rhs.farClip_;
		}
		return *this;
	}

private:
	std::unique_ptr<UploadBuffer> constBuff_ = nullptr;

public:
	Vector3 rotation_ = { 0.0f,0.0f,0.0f };

	Vector3 translation_ = { 0.0f,0.0f,-50.0f };

	Matrix4x4 matView_{};

	Matrix4x4 matProjection_{};

	float fov_ = 45.0f * 3.141592654f / 180.0f;

	float aspectRatio_ = 1280.0f / 720.0f;

	float nearClip_ = 0.1f;

	float farClip_ = 1000.0f;
};

