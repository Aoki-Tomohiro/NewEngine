#pragma once
#include "Engine/Framework/Scene/IScene.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Base/Renderer.h"
#include "Engine/2D/Sprite.h"

class LoadScene : public IScene
{
public:
	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw() override;

	void DrawUI() override;

private:
	Renderer* renderer_ = nullptr;

	std::unique_ptr<Sprite> loadSprite_ = nullptr;

	int timer_ = 0;

	int animationCount_ = 0;
};

