#include "VerticalBlur.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Blur
{
    int32_t textureWidth;
    int32_t textureHeight;
    float4 bkWeight[2];
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<Blur> gBlur : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    float32_t4 color = gTexture.Sample(gSampler, input.texcoord);
    float w, h, level;
    //画像の大きさを取得
    gTexture.GetDimensions(0, w, h, level);
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    output.color += gBlur.bkWeight[0] * color;
    output.color += gBlur.bkWeight[0][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 1));
    output.color += gBlur.bkWeight[0][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 1));
    output.color += gBlur.bkWeight[0][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 2));
    output.color += gBlur.bkWeight[0][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 2));
    output.color += gBlur.bkWeight[0][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 3));
    output.color += gBlur.bkWeight[0][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 3));
    output.color += gBlur.bkWeight[1][0] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 4));
    output.color += gBlur.bkWeight[1][0] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 4));
    output.color += gBlur.bkWeight[1][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 5));
    output.color += gBlur.bkWeight[1][1] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 5));
    output.color += gBlur.bkWeight[1][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 6));
    output.color += gBlur.bkWeight[1][2] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 6));
    output.color += gBlur.bkWeight[1][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, dy * 7));
    output.color += gBlur.bkWeight[1][3] * gTexture.Sample(gSampler, input.texcoord + float2(0, -dy * 7));
    output.color.a = color.a;
    
    return output;
}