#include "PostEffects.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct LensDistortion
{
    int32_t isEnable;
    float tightness;
    float strength;
};

struct Vignette
{
    int32_t isEnable;
    float intensity;
};

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t4> gRenderedEffectsTexture : register(t1);
SamplerState gSampler : register(s0);

ConstantBuffer<LensDistortion> gLensDistortionParameter : register(b0);
ConstantBuffer<Vignette> gVignetteParameter : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    //通常テクスチャ
    float32_t4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    //ポストエフェクトをかけたテクスチャ
    float32_t4 effectsColor = gRenderedEffectsTexture.Sample(gSampler, input.texcoord);
    //ポストエフェクトをかけたテクスチャが真っ黒の場合はoriginalColorを使う
    if (effectsColor.r == 0 && effectsColor.g == 0 && effectsColor.b == 0 && effectsColor.a == 0)
    {
        output.color = originalColor;
    }
    else
    {
        output.color = effectsColor;

    }
    
    //レンズディストーション
    if (gLensDistortionParameter.isEnable)
    {
        const float2 uvNormalized = input.texcoord * 2 - 1;
        const float distortionMagnitude = abs(uvNormalized.x * uvNormalized.y);
        const float smoothDistortionMagnitude = pow(distortionMagnitude, gLensDistortionParameter.tightness);
		//const float smoothDistortionMagnitude = 1 - sqrt(1 - pow(distortionMagnitude, gLensDistortionParameter.tightness));
		//const float smoothDistortionMagnitude = pow(sin(1.57079632679f * distortionMagnitude), gLensDistortionParameter.tightness);
        float2 uvDistorted = input.texcoord + uvNormalized * smoothDistortionMagnitude * gLensDistortionParameter.strength;
        if (uvDistorted[0] < 0 || uvDistorted[0] > 1 || uvDistorted[1] < 0 || uvDistorted[1] > 1)
        {
            originalColor = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
            effectsColor = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        else
        {
            originalColor = gTexture.Sample(gSampler, uvDistorted);
            effectsColor = gRenderedEffectsTexture.Sample(gSampler, uvDistorted);
            //ポストエフェクトをかけたテクスチャが真っ黒の場合はoriginalColorを使う
            if (effectsColor.r == 0 && effectsColor.g == 0 && effectsColor.b == 0 && effectsColor.a == 0)
            {
                output.color = originalColor;
            }
            else
            {
                output.color = effectsColor;

            }
        }
    }
    
    //ビネット
    if (gVignetteParameter.isEnable)
    {
        float2 uv = input.texcoord;
        uv = gVignetteParameter.intensity * uv - gVignetteParameter.intensity / 2;
        output.color *= 1.0 - dot(uv, uv);
    }
    
    return output;
}