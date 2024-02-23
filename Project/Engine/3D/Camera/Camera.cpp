#include "Camera.h"
#include "Engine/Math/MathFunction.h"

void Camera::Initialize()
{
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataCamera));
	UpdateMatrix();
}

void Camera::UpdateViewMatrix()
{
	Matrix4x4 translateMatrix = Mathf::MakeTranslateMatrix(translation_);
	Matrix4x4 rotateXMatrix = Mathf::MakeRotateXMatrix(rotation_.x);
	Matrix4x4 rotateYMatrix = Mathf::MakeRotateYMatrix(rotation_.y);
	Matrix4x4 rotateZMatrix = Mathf::MakeRotateZMatrix(rotation_.z);
	Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
	matView_ = Mathf::Inverse(translateMatrix) * Mathf::Inverse(rotateMatrix);
}

void Camera::UpdateProjectionMatrix()
{
	matProjection_ = Mathf::MakePerspectiveFovMatrix(fov_, aspectRatio_, nearClip_, farClip_);
}

void Camera::UpdateMatrix()
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();
	TransferMatrix();
}

void Camera::TransferMatrix()
{
	ConstBuffDataCamera* cameraData = static_cast<ConstBuffDataCamera*>(constBuff_->Map());
	cameraData->worldPosition = translation_;
	cameraData->view = matView_;
	cameraData->projection = matProjection_;
	constBuff_->Unmap();
}