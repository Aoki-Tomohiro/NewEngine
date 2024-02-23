#pragma once
#include "Engine/Base/StructuredBuffer.h"
#include "Engine/Base/DescriptorHandle.h"
#include "Engine/3D/Model/ModelManager.h"
#include "ParticleEmitterBuilder.h"

class ParticleSystem
{
public:
	//パーティクルの最大数
	const uint32_t kMaxInstance = 1000;

	void Initialize();

	void Update();

	void Draw(const Camera& camera);

	void Clear();

	void AddParticleEmitter(ParticleEmitter* particleEmitter) { particleEmitters_.push_back(std::unique_ptr<ParticleEmitter>(particleEmitter)); };

	ParticleEmitter* GetParticleEmitter(const std::string& name);

	std::list<ParticleEmitter*> GetParticleEmitters(const std::string& name);

	const bool GetIsBillBoard() const { return isBillboard_; };

	void SetIsBillBoard(const bool isBillboard) { isBillboard_ = isBillboard; };

	void SetModel(Model* model) { model_ = model; };

	void SetTexture(const std::string& name) { model_ ? model_->SetTexture(name) : defaultModel_->SetTexture(name); };

private:
	void CreateInstancingResource();

	void UpdateInstancingResource(const Camera& camera);

private:
	std::unique_ptr<StructuredBuffer> instancingResource_ = nullptr;

	uint32_t numInstance_ = 0;

	std::list<std::unique_ptr<ParticleEmitter>> particleEmitters_{};

	std::unique_ptr<Model> defaultModel_ = nullptr;

	Model* model_ = nullptr;

	bool isBillboard_ = true;
};

