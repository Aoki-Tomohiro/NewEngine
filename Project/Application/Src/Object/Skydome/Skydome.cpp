#include "Skydome.h"

void Skydome::Initialize()
{
	worldTransform_.Initialize();
	worldTransform_.scale_ = { 500.0f,500.0f,500.0f };
}

void Skydome::Update()
{
	worldTransform_.UpdateMatrixFromEuler();
}

void Skydome::Draw(const Camera& camera)
{
	model_->Draw(worldTransform_, camera);
}

void Skydome::DrawUI()
{

}