#include "../Common/Pixel/PixelShader2DHeader.hlsli"


Texture2D rotatingNeedleTex : register(t1);         // ‰ñ“]j
SamplerState rotatingNeedleTexSampler : register(s1);

Texture2D needleTex : register(t2);                 // ƒS[ƒ‹j
SamplerState needleTexSampler : register(s2);

cbuffer cbParam : register(b4)
{
    float2 rotateCenterUV; // ‰ñ“]’†SiUVÀ•Wj
    float rotateRad;       // ‰ñ“]’†‚Ìj‚ÌŠp“xiƒ‰ƒWƒAƒ“j
    float endRad;          // ƒS[ƒ‹j‚ÌŠp“xiƒ‰ƒWƒAƒ“j
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;
    float4 color = float4(0, 0, 0, 0);

    // --- ƒtƒŒ[ƒ€•`‰æiŒÅ’èj ---
    float4 frameColor = tex.Sample(texSampler, uv);
    if (frameColor.a > 0.01f) {
        color = frameColor;
    }

    // --- ƒS[ƒ‹j‚ÌUV‰ñ“] ---
    {
        float2 relative = uv - rotateCenterUV;
        float cosEnd = cos(-endRad);
        float sinEnd = sin(-endRad);
        float2 rotated;
        rotated.x = relative.x * cosEnd - relative.y * sinEnd;
        rotated.y = relative.x * sinEnd + relative.y * cosEnd;
        float2 rotatedUV = rotated + rotateCenterUV;

        if (rotatedUV.x >= 0.0f && rotatedUV.x <= 1.0f &&
            rotatedUV.y >= 0.0f && rotatedUV.y <= 1.0f)
        {
            float4 goalColor = needleTex.Sample(needleTexSampler, rotatedUV);
            if (goalColor.a > 0.01f) {
                color = goalColor;
            }
        }
    }

    // --- Œ»Ý‚Ì‰ñ“]j ---
    {
        float2 relative = uv - rotateCenterUV;

        float cosRot = cos(-rotateRad);
        float sinRot = sin(-rotateRad);
        float2 rotated;
        rotated.x = relative.x * cosRot - relative.y * sinRot;
        rotated.y = relative.x * sinRot + relative.y * cosRot;

        // š ‰ñ“]Œã‚ÉƒXƒP[ƒŠƒ“ƒO
        float scale = 1.0; // ‚±‚±‚ð’²®F1.0‚æ‚è‘å‚«‚­‚·‚é‚Æ’Z‚­‚È‚é
        rotated *= scale;
        rotated.y *= 1.7;

        float2 rotatedUV = rotated + rotateCenterUV;

        if (rotatedUV.x >= 0.0f && rotatedUV.x <= 1.0f &&
            rotatedUV.y >= 0.0f && rotatedUV.y <= 1.0f)
        {
            float4 rotNeedleColor = rotatingNeedleTex.Sample(rotatingNeedleTexSampler, rotatedUV);
            if (rotNeedleColor.a > 0.01f) {
                color = rotNeedleColor;
            }
        }
    }


    if (color.a < 0.01f)
    {
        discard;
    }

    return color;
}
