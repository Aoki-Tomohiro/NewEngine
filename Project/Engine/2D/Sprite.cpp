#include "Sprite.h"
#include "Engine/Base/GraphicsCore.h"
#include "Engine/Base/TextureManager.h"
#include "Engine/Math/MathFunction.h"

Sprite* Sprite::Create(const std::string& textureName, Vector2 position)
{
	Sprite* sprite = new Sprite();
	sprite->Initialize(textureName, position);
	return sprite;
}

void Sprite::Draw()
{
	if (isInvisible_) {
		return;
	}

	UpdateVertexBuffer();
	UpdateMaterialResource();
	UpdateWVPResource();
	CommandContext* commandContext = GraphicsCore::GetInstance()->GetCommandContext();
	TextureManager* textureManager = TextureManager::GetInstance();
	commandContext->SetVertexBuffer(vertexBufferView_);
	commandContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext->SetConstantBuffer(0, materialConstBuffer_->GetGpuVirtualAddress());
	commandContext->SetConstantBuffer(1, wvpResource_->GetGpuVirtualAddress());
	commandContext->SetDescriptorTable(2, texture_->GetSRVHandle());
	commandContext->DrawInstanced(kMaxVertices, 1);
}

void Sprite::Initialize(const std::string& textureName, Vector2 position)
{
	//座標を設定
	position_ = position;

	//テクスチャを設定
	SetTexture(textureName);

	//テクスチャの情報を取得
	resourceDesc_ = texture_->GetResourceDesc();

	//テクスチャの情報を基にサイズを初期化
	textureSize_ = { float(resourceDesc_.Width),float(resourceDesc_.Height) };
	size_ = { float(resourceDesc_.Width),float(resourceDesc_.Height) };

	//頂点バッファの作成
	CreateVertexBuffer();

	//マテリアル用のリソースを作成
	CreateMaterialResource();

	//WVP用のリソースを作成
	CreateWVPResource();
}

void Sprite::CreateVertexBuffer()
{
	//頂点リソースを作る
	vertexBuffer_ = std::make_unique<UploadBuffer>();
	vertexBuffer_->Create(sizeof(VertexDataPosUV) * 6);
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGpuVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexDataPosUV) * 6;
	//頂点1つあたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexDataPosUV);
	//リソースに書き込む
	UpdateVertexBuffer();
}

void Sprite::UpdateVertexBuffer()
{
	//テクスチャのサイズを合わせる
	AdjustTextureSize();

	//頂点データの更新
	float left = (0.0f - anchorPoint_.x) * size_.x;
	float right = (1.0f - anchorPoint_.x) * size_.x;
	float top = (0.0f - anchorPoint_.y) * size_.y;
	float bottom = (1.0f - anchorPoint_.y) * size_.y;
	float texLeft = textureLeftTop_.x / resourceDesc_.Width;
	float texRight = (textureLeftTop_.x + textureSize_.x) / resourceDesc_.Width;
	float texTop = textureLeftTop_.y / resourceDesc_.Height;
	float texBottom = (textureLeftTop_.y + textureSize_.y) / resourceDesc_.Height;

	//左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}

	//上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	//頂点データの設定
	vertices_[0].position = { left,bottom,0.0f,1.0f };
	vertices_[0].texcoord = { texLeft,texBottom };
	vertices_[1].position = { left,top,0.0f,1.0f };
	vertices_[1].texcoord = { texLeft,texTop };
	vertices_[2].position = { right,bottom,0.0f,1.0f };
	vertices_[2].texcoord = { texRight,texBottom };
	vertices_[3].position = { left,top,0.0f,1.0f };
	vertices_[3].texcoord = { texLeft,texTop };
	vertices_[4].position = { right,top,0.0f,1.0f };
	vertices_[4].texcoord = { texRight,texTop };
	vertices_[5].position = { right,bottom,0.0f,1.0f };
	vertices_[5].texcoord = { texRight,texBottom };

	//書き込み
	VertexDataPosUV* vertexData = static_cast<VertexDataPosUV*>(vertexBuffer_->Map());
	std::copy(std::begin(vertices_), std::end(vertices_), vertexData);
	vertexBuffer_->Unmap();
}

void Sprite::CreateMaterialResource()
{
	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialConstBuffer_ = std::make_unique<UploadBuffer>();
	materialConstBuffer_->Create(sizeof(ConstBuffDataMaterial));
	//リソースに書き込む
	UpdateMaterialResource();
}

void Sprite::UpdateMaterialResource()
{
	//書き込むためのアドレスを取得
	ConstBuffDataMaterial* materialData = static_cast<ConstBuffDataMaterial*>(materialConstBuffer_->Map());
	//データを書き込む
	materialData->color = color_;
	Matrix4x4 uvTransformMatrix = Mathf::MakeScaleMatrix(Vector3{ uvScale_.x,uvScale_.y,1.0f });
	uvTransformMatrix = uvTransformMatrix * Mathf::MakeRotateZMatrix(uvRotation_);
	uvTransformMatrix = uvTransformMatrix * Mathf::MakeTranslateMatrix(Vector3{ uvTranslation_.x,uvTranslation_.y,0.0f });
	materialData->uvTransform = uvTransformMatrix;
	materialConstBuffer_->Unmap();
}

void Sprite::CreateWVPResource()
{
	//WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource_ = std::make_unique<UploadBuffer>();
	wvpResource_->Create(sizeof(Matrix4x4));
	//リソースに書き込む
	UpdateWVPResource();
}

void Sprite::UpdateWVPResource()
{
	//ワールド行列の作成
	Matrix4x4 worldMatrix = Mathf::MakeAffineMatrix(Vector3(scale_.x, scale_.y, 1.0f), Vector3(0.0f, 0.0f, rotation_), Vector3(position_.x, position_.y, 0.0f));
	//ビュー行列の作成
	Matrix4x4 viewMatrix = Mathf::MakeIdentity4x4();
	//平行投影行列の作成
	Matrix4x4 matProjection_ = Mathf::MakeOrthographicMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);
	//WVPMatrixの作成
	Matrix4x4 worldViewProjectionMatrix = worldMatrix * viewMatrix * matProjection_;

	//データを書き込む
	Matrix4x4* wvpData = static_cast<Matrix4x4*>(wvpResource_->Map());
	*wvpData = worldViewProjectionMatrix;
	wvpResource_->Unmap();
}

void Sprite::AdjustTextureSize()
{
	//テクスチャの情報を取得
	resourceDesc_ = texture_->GetResourceDesc();
	//テクスチャサイズの初期化
	textureSize_ = { float(resourceDesc_.Width),float(resourceDesc_.Height) };
}

void Sprite::SetTexture(const std::string& textureName)
{
	//テクスチャを設定
	texture_ = TextureManager::GetInstance()->FindTexture(textureName);
	//テクスチャがなかったら止める
	assert(texture_);
}