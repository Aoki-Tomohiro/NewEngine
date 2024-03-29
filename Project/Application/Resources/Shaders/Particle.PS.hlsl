#include "Particle.hlsli"

struct Material
{
    float32_t4 color;
    float32_t4x4 uvTransform;
    int32_t enableLighting;
    int32_t diffuseReflectionType;
    int32_t specularReflectionType;
    float32_t shininess;
    float32_t3 specularColor;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformUV.xy);
    output.color = gMaterial.color * textureColor;
    
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    
    output.color = gMaterial.color * textureColor * input.color;
    
    return output;
}