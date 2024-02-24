#include "BaseCharacter.h"

void BaseCharacter::Initialize(const std::vector<Model*>& models) {
	models_ = models;
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
}

void BaseCharacter::Update() {
	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrixFromEuler();
}

void BaseCharacter::Draw(const Camera& camera) {
	//モデル描画
	for (Model* model : models_) {
		model->Draw(worldTransform_, camera);
	}
}