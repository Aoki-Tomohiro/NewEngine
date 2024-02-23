#pragma once
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"
#include <cstdint>

struct VertexDataPosUVNormal 
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct VertexDataPosUV
{
	Vector4 position;
	Vector2 texcoord;
};

struct ConstBuffDataMaterial
{
	Vector4 color;
	Matrix4x4 uvTransform;
	int32_t enableLighting;
	int32_t diffuseReflectionType;
	int32_t specularReflectionType;
	float shininess;
	Vector3 specularColor;
};

struct ConstBuffDataWorldTransform
{
	Matrix4x4 world;
	Matrix4x4 worldInverseTranspse;
};

struct ConstBuffDataCamera 
{
	Vector3 worldPosition;
	float padding;
	Matrix4x4 view;
	Matrix4x4 projection;
};

struct ConstBuffDataDirectionalLight 
{
	Vector4 color;
	Vector3 direction;
	float intensity;
	int32_t isEnable;
	float padding[3];
};

struct ConstBuffDataPointLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	int32_t isEnable;
	float padding;
};

struct ConstBuffDataSpotLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosFalloffStart;
	float cosAngle;
	int32_t isEnable;
};

struct ParticleForGPU
{
	Matrix4x4 world;
	Vector4 color;
};

struct ConstBuffDataGaussianBlur
{
	int32_t textureWidth;
	int32_t textureHeight;
	int32_t padding[2];
	float weight[8];
};

struct ConstBuffDataBloom
{
	int32_t isEnable;
	float intensity;
	float textureWeight;
	float highLumTextureWeight;
	float blurTextureWeight[4];
};

struct ConstBuffDataFog
{
	bool isEnable;
	float scale;
	float attenuationRate;
};

struct ConstBuffDataDoF
{
	int32_t isEnable;
	float focusDepth;
	float nFocusWidth;
	float fFocusWidth;
};

struct ConstBuffDataLensDistortion
{
	bool isEnable;
	float tightness;
	float strength;
};

struct ConstBuffDataVignette
{
	bool isEnable;
	float intensity;
};