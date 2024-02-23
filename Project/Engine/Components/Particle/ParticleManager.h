#pragma once
#include "ParticleSystem.h"
#include <unordered_map>

class ParticleManager
{
public:
	static ParticleManager* GetInstance();

	static void Destroy();

	static ParticleSystem* Create(const std::string& name);

	void Update();

	void Draw(const Camera& camera);

	void Clear();

private:
	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

private:
	ParticleSystem* CreateInternal(const std::string& name);

private:
	static ParticleManager* instance_;

	std::unordered_map<std::string, std::unique_ptr<ParticleSystem>> particleSystems_;
};

