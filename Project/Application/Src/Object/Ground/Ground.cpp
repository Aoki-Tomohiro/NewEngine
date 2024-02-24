#include "Ground.h"

void Ground::Initialize()
{
	worldTransform_.Initialize();
	worldTransform_.translation_.y = -2.0f;
	worldTransform_.scale_ = { 50.0f,2.0f,50.0f };
}

void Ground::Update()
{
	worldTransform_.UpdateMatrixFromEuler();
}

void Ground::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}

void Ground::DrawUI()
{

}