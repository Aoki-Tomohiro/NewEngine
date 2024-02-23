#pragma once
#include "Texture.h"

class TextureManager
{
public:
	static const std::string kBaseDirectory;

	static TextureManager* GetInstance();

	static void Destroy();

	static void Load(const std::string& filename);

	void Initialize();

	const Texture* FindTexture(const std::string& name) const;

private:
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	void LoadInternal(const std::string& filePath);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

private:
	static TextureManager* instance_;

	std::unordered_map<std::string, std::unique_ptr<Texture>> textures_{};
};

