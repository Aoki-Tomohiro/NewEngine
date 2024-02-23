#include "ParticleManager.h"

ParticleManager* ParticleManager::instance_ = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new ParticleManager();
	}
	return instance_;
}

void ParticleManager::Destroy()
{
	if (instance_)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

ParticleSystem* ParticleManager::Create(const std::string& name)
{
	ParticleSystem* particleSystem = ParticleManager::GetInstance()->CreateInternal(name);
	return particleSystem;
}

void ParticleManager::Update()
{
	for (auto& particleSystem : particleSystems_)
	{
		particleSystem.second->Update();
	}
}

void ParticleManager::Draw(const Camera& camera)
{
	for (auto& particleSystem : particleSystems_)
	{
		particleSystem.second->Draw(camera);
	}
}

void ParticleManager::Clear()
{
	for (auto& particleSystem : particleSystems_)
	{
		particleSystem.second->Clear();
	}
}

ParticleSystem* ParticleManager::CreateInternal(const std::string& name)
{
	auto it = particleSystems_.find(name);

	if (it != particleSystems_.end())
	{
		return it->second.get();
	}

	ParticleSystem* particleSystem = new ParticleSystem();
	particleSystem->Initialize();
	particleSystems_[name] = std::unique_ptr<ParticleSystem>(particleSystem);
	return particleSystem;
}