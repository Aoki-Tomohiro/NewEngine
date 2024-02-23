#include "Bloom.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Bloom
{
    int32_t isEnable;
    float32_t intensity;
    float32_t textureWeight;
    float32_t highLumTextureWeight;
    float32_t4 blurTextureWeight;
};

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t4> gHighLumTexture : register(t1);
Texture2D<float32_t4> gBlurTexture1 : register(t2);
Texture2D<float32_t4> gBlurTexture2 : register(t3);
Texture2D<float32_t4> gBlurTexture3 : register(t4);
Texture2D<float32_t4> gBlurTexture4 : register(t5);
SamplerState gSampler : register(s0);

ConstantBuffer<Bloom> gBloomParameter : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float32_t4 highLumTextureColor = gHighLumTexture.Sample(gSampler, input.texcoord);
    float32_t4 blurTextureColor1 = gBlurTexture1.Sample(gSampler, input.texcoord);
    float32_t4 blurTextureColor2 = gBlurTexture2.Sample(gSampler, input.texcoord);
    float32_t4 blurTextureColor3 = gBlurTexture3.Sample(gSampler, input.texcoord);
    float32_t4 blurTextureColor4 = gBlurTexture4.Sample(gSampler, input.texcoord);
    
    if(gBloomParameter.isEnable)
    {        
        //テクスチャの色を設定
        float32_t3 textureColor1 = textureColor.rgb * gBloomParameter.textureWeight;
        float32_t3 textureColor2 = highLumTextureColor.rgb * gBloomParameter.highLumTextureWeight;
        float32_t3 textureColor3 = blurTextureColor1.rgb * gBloomParameter.blurTextureWeight.x;
        float32_t3 textureColor4 = blurTextureColor2.rgb * gBloomParameter.blurTextureWeight.y;
        float32_t3 textureColor5 = blurTextureColor3.rgb * gBloomParameter.blurTextureWeight.z;
        float32_t3 textureColor6 = blurTextureColor4.rgb * gBloomParameter.blurTextureWeight.w;
        
        //すべて合成
        output.color.rgb = (textureColor1 + textureColor2 + textureColor3 + textureColor4 + textureColor5 + textureColor6) * gBloomParameter.intensity;
        output.color.a = textureColor.a;
    }
    else
    {
        output.color = textureColor;
    }
 
    return output;
}