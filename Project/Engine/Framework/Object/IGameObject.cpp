#include "IGameObject.h"

void IGameObject::Initialize() 
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
}

void IGameObject::Update()
{
	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();
}

void IGameObject::Draw(const Camera& camera)
{
	//モデルの描画
	if (model_ != nullptr) {
		model_->Draw(worldTransform_, camera);
	}
}