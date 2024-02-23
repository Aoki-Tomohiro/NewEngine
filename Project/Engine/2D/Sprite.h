#pragma once
#include "Engine/Base/Texture.h"
#include "Engine/Base/UploadBuffer.h"
#include "Engine/Base/ConstantBuffers.h"
#include <array>
#include <memory>
#include <string>

class Sprite
{
public:
	static const uint32_t kMaxVertices = 6;

	static Sprite* Create(const std::string& textureName, Vector2 position);

	void Draw();

	const Vector2& GetPosition() const { return position_; };

	void SetPosition(const Vector2& position) { position_ = position; };

	float GetRotation() const { return rotation_; };

	void SetRotation(float rotation) { rotation_ = rotation; };

	const Vector4& GetColor() const { return color_; };

	void SetColor(const Vector4& color) { color_ = color; };

	const Vector2& GetSize() const { return size_; };

	void SetSize(const Vector2& size) { size_ = size; };

	const Vector2& GetAnchorPoint() const { return anchorPoint_; };

	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; };

	bool GetIsFlipX() const { return isFlipX_; };

	void SetIsFlipX(bool isFlipX) { isFlipX_ = isFlipX; };

	bool GetIsFlipY() const { return isFlipY_; };

	void SetIsFlipY(bool isFlipY) { isFlipY_ = isFlipY; };

	bool GetIsInvisible() const { return isInvisible_; };

	void SetIsInvisible(bool isInvisible) { isInvisible_ = isInvisible; };

	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; };

	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; };

	const Vector2& GetTextureSize() const { return textureSize_; };

	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; };

	const Vector2 GetUVTranslation() const { return uvTranslation_; };

	void SetUVTranslation(const Vector2& uvTranslation) { uvTranslation_ = uvTranslation; };

	float GetUVRotation() const { return uvRotation_; };

	void SetUVRotation(float uvRotation) { uvRotation_ = uvRotation; };

	const Vector2 GetUVScale() const { return uvScale_; };

	void SetUVScale(const Vector2& uvScale) { uvScale_ = uvScale; };

	void SetTexture(const std::string& textureName);

private:
	void Initialize(const std::string& textureName, Vector2 position);

	void CreateVertexBuffer();

	void UpdateVertexBuffer();

	void CreateMaterialResource();

	void UpdateMaterialResource();

	void CreateWVPResource();

	void UpdateWVPResource();

	void AdjustTextureSize();

private:
	std::unique_ptr<UploadBuffer> vertexBuffer_ = nullptr;

	std::unique_ptr<UploadBuffer> materialConstBuffer_ = nullptr;

	std::unique_ptr<UploadBuffer> wvpResource_ = nullptr;

	std::array<VertexDataPosUV, kMaxVertices> vertices_{};

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	D3D12_RESOURCE_DESC resourceDesc_{};

	Vector2 position_ = { 0.0f,0.0f };

	float rotation_ = 0.0f;

	Vector2 scale_ = { 1.0f,1.0f };

	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

	Vector2 size_ = { 100.0f,100.0f };

	Vector2 anchorPoint_{ 0.0f,0.0f };

	bool isFlipX_ = false;

	bool isFlipY_ = false;

	bool isInvisible_ = false;

	Vector2 textureLeftTop_ = { 0.0f,0.0f };

	Vector2 textureSize_ = { 100.0f,100.0f };

	Vector2 uvTranslation_ = { 0.0f,0.0f };

	float uvRotation_ = 0.0f;

	Vector2 uvScale_ = { 1.0f,1.0f };

	const Texture* texture_ = nullptr;
};

