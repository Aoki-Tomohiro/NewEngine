#include "WorldTransform.h"
#include "Engine/Math/MathFunction.h"

void WorldTransform::Initialize()
{
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(sizeof(ConstBuffDataWorldTransform));
	UpdateMatrixFromEuler();
}

void WorldTransform::TransferMatrix()
{
	ConstBuffDataWorldTransform* worldTransformData = static_cast<ConstBuffDataWorldTransform*>(constBuff_->Map());
	worldTransformData->world = matWorld_;
	worldTransformData->worldInverseTranspse = Mathf::Transpose(Mathf::Inverse(matWorld_));
	constBuff_->Unmap();
}

void WorldTransform::UpdateMatrixFromEuler()
{
	matWorld_ = Mathf::MakeAffineMatrix(scale_, rotation_, translation_);

	if (parent_) 
	{
		matWorld_ = matWorld_ * parent_->matWorld_;
	}

	TransferMatrix();
}

void WorldTransform::UpdateMatrixFromQuaternion()
{
	matWorld_ = Mathf::MakeAffineMatrix(scale_, quaternion_, translation_);

	if (parent_)
	{
		matWorld_ = matWorld_ * parent_->matWorld_;
	}

	TransferMatrix();
}

void WorldTransform::SetParent(const WorldTransform* parent) {
	//親を設定
	parent_ = parent;
	//親基準のローカル座標を計算
	if (parent_) {
		translation_ -= parent->translation_;
	}
}

void WorldTransform::UnsetParent() {
	if (parent_) {
		//新しいポジションを設定
		translation_ = { Vector3{matWorld_.m[3][0],matWorld_.m[3][1],matWorld_.m[3][2]} };
	}
	//親子付けを外す
	parent_ = nullptr;
}