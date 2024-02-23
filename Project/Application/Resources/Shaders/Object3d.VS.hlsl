#include "Object3d.hlsli"

struct WorldTransform
{
    float32_t4x4 world;
    float32_t4x4 worldInverseTranspose;
};

struct Camera
{
    float32_t3 worldPosition;
    float32_t4x4 view;
    float32_t4x4 projection;
};

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);
ConstantBuffer<Camera> gCamera : register(b1);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(gWorldTransform.world, mul(gCamera.view, gCamera.projection)));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gWorldTransform.worldInverseTranspose));
    output.worldPosition = mul(input.position, gWorldTransform.world).xyz;
    output.toEye = normalize(gCamera.worldPosition - mul(input.position, gWorldTransform.world).xyz);
    output.depth = (output.position.z - 0.1f) / (1000.0f - 0.1f);

    return output;
}