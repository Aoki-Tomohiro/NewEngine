#pragma once
#include "Engine/Framework/Object/IGameObject.h"

class Skydome : public IGameObject
{
public:
	void Initialize() override;

	void Update() override;

	void Draw(const Camera& camera) override;

	void DrawUI() override;
};

