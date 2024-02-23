#include "Fog.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

struct Fog
{
    bool isEnable;
    float scale;
    float attenuationRate;
};

Texture2D<float32_t> gLinearDepthTexture : register(t0);
Texture2D<float32_t4> gTexture : register(t1);
SamplerState gSampler : register(s0);

ConstantBuffer<Fog> gFogParameter : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t depthColor = gLinearDepthTexture.Sample(gSampler, input.texcoord);
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    float fogFactor = 1.0f;
    if (gFogParameter.isEnable)
    {
        float fogWeight = 0.0f;
        fogWeight += gFogParameter.scale * max(0.0f, 1.0f - exp(-gFogParameter.attenuationRate * depthColor));
        const float3 gbColor = textureColor.rgb;
        const float3 fogColor = 0.8f;
        output.color.rgb = lerp(gbColor, fogColor, fogWeight);
        output.color.a = textureColor.a;
    }
    else
    {
        output.color = textureColor;
    }
    
    return output;
}