#include "HighLum.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float32_t y = textureColor.r * 0.299 + textureColor.g * 0.587 + textureColor.b * 0.114;
    float32_t extract = smoothstep(0.6f, 0.9f, y);
    output.color = textureColor * extract;
    return output;
}