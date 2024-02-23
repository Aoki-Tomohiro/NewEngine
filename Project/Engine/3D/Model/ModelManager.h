#pragma once
#include "Model.h"
#include <filesystem>
#include <unordered_map>

class ModelManager
{
public:
	static const std::string kBaseDirectory;

	static ModelManager* GetInstance();

	static void Destroy();

	static Model* Create();

	static Model* CreateFromOBJ(const std::string& modelName, DrawPass drawPass);

	void Initialize();

private:
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	Model* CreateInternal(const std::string& modelName, DrawPass drawPass);

	Model::ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	Model::MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	//Model::ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	//Model::Node ReadNode(aiNode* node);

private:
	static ModelManager* instance_;

	std::unordered_map<std::string, Model::ModelData> modelDatas_;
};

