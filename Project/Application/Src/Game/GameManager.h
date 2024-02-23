#pragma once
#include "Engine/Framework/Game/GameCore.h"
#include "Application/Src/Scene/SceneFactory.h"

class GameManager : public GameCore
{
public:
	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

private:

};

