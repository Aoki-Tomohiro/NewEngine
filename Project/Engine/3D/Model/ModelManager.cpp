#include "ModelManager.h"
#include <cassert>
#include <fstream>
#include <sstream>

//実体定義
ModelManager* ModelManager::instance_ = nullptr;
const std::string ModelManager::kBaseDirectory = "Application/Resources/Models";

ModelManager* ModelManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new ModelManager();
	}
	return instance_;
}

void ModelManager::Destroy()
{
	if (instance_)
	{
		delete instance_;
		instance_ = nullptr;
	}
}

Model* ModelManager::Create()
{
	Model* model = ModelManager::GetInstance()->CreateInternal("Cube", Opaque);
	return model;
}

Model* ModelManager::CreateFromOBJ(const std::string& modelName, DrawPass drawPass)
{
	Model* model = ModelManager::GetInstance()->CreateInternal(modelName, drawPass);
	return model;
}

Model* ModelManager::CreateInternal(const std::string& modelName,DrawPass drawPass)
{
	auto it = modelDatas_.find(modelName);

	if (it != modelDatas_.end())
	{
		Model* model = new Model();
		model->Create(it->second, drawPass);
		return model;
	}

	//モデルデータの読み込み
	std::string directoryPath = kBaseDirectory + "/" + modelName;
	std::string filename = modelName + ".obj";
	Model::ModelData modelData = LoadObjFile(directoryPath, filename);
	modelDatas_[modelName] = modelData;

	//モデルの生成
	Model* model = new Model();
	model->Create(modelData, drawPass);

	return model;
}

void ModelManager::Initialize()
{
	Model::ModelData modelData = LoadObjFile("Application/Resources/Models/Cube", "Cube.obj");
	modelDatas_["Cube"] = modelData;
}

Model::ModelData ModelManager::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	Model::ModelData modelData;//構築するModelData
	std::vector<Vector4> positions;//位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む

		//identifierに応じた処理
		if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.z *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt")
		{
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.z *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f")
		{
			VertexDataPosUVNormal triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/');// /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position,texcoord,normal };
			}
			//頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			//materialTempalteLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

Model::MaterialData ModelManager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	Model::MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

//Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename)
//{
//	ModelData modelData;
//	Assimp::Importer importer;
//	std::string filePath = directoryPath + "/" + filename;
//	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
//	assert(scene->HasMeshes());//メッシュがないのは対応しない
//
//	//Meshの解析
//	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
//	{
//		aiMesh* mesh = scene->mMeshes[meshIndex];
//		assert(mesh->HasNormals());//法線がないMeshは今回は非対応
//		assert(mesh->HasTextureCoords(0));//TexcoordがないMeshは今回は非対応
//		//ここからMeshの中身(Face)の解析を行っていく
//		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
//		{
//			aiFace& face = mesh->mFaces[faceIndex];
//			assert(face.mNumIndices == 3);//三角形のみサポート
//			//ここからFaceの中身(Vertex)の解析を行っていく
//			for (uint32_t element = 0; element < face.mNumIndices; ++element)
//			{
//				uint32_t vertexIndex = face.mIndices[element];
//				aiVector3D& position = mesh->mVertices[vertexIndex];
//				aiVector3D& normal = mesh->mNormals[vertexIndex];
//				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
//				VertexDataPosUVNormal vertex{};
//				vertex.position = { position.x,position.y,position.z,1.0f };
//				vertex.normal = { normal.x,normal.y,normal.z };
//				vertex.texcoord = { texcoord.x,texcoord.y };
//				//aiProcess_MakeLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
//				vertex.position.x *= -1.0f;
//				vertex.normal.x *= -1.0f;
//				modelData.vertices.push_back(vertex);
//			}
//		}
//	}
//
//	//Materialの解析
//	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
//	{
//		aiMaterial* material = scene->mMaterials[materialIndex];
//		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
//		{
//			aiString textureFilePath;
//			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
//			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
//		}
//	}
//
//	//Nodeの解析
//	modelData.rootNode = ReadNode(scene->mRootNode);
//
//	return modelData;
//}
//
//Model::Node Model::ReadNode(aiNode* node) 
//{
//	Node result{};
//	aiMatrix4x4 aiLocalMatrix = node->mTransformation;//nodeのlocalMatrixを取得
//	aiLocalMatrix.Transpose();//列ベクトル形式を行ベクトル形式に転置
//	for (uint32_t i = 0; i < 4; i++)
//	{
//		for (uint32_t j = 0; j < 4; j++)
//		{
//			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
//		}
//	}
//	result.name = node->mName.C_Str();//Node名を格納
//	result.children.resize(node->mNumChildren);//子供の数だけ確保
//	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
//	{
//		//再帰的に読んで階層構造を作っていく
//		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
//	}
//	return result;
//}