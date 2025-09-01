#include "../Common/Pixel/PixelShader2DHeader.hlsli"

//Texture2D frameTex : register(t0);
//SamplerState frameSampler : register(s0);

Texture2D hpBarTex : register(t1);
SamplerState hpBarSampler : register(s1);

Texture2D iconTex : register(t2);
SamplerState iconSampler : register(s2);

cbuffer cbParam : register(b4)
{
    float2 hpUVPos;      // HPバーの左上
    float2 hpUVSize;     // HPバーサイズ（横長）
    float hpRate;        // 0.0 ～ 1.0
    float3 g_none;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;
    float4 color = float4(0, 0, 0, 0);

    // --- フレーム（常に描画ベースにする） ---
    float4 frameColor = tex.Sample(texSampler, uv);
    color = frameColor;

    // --- アイコン（フレームの上に描画） ---
    float4 iconColor = iconTex.Sample(iconSampler, uv);
    if (iconColor.a > 0.01f) {
        color = iconColor; // 上書き
    }

     // --- HPバー（さらに上に描画） ---
     if (uv.x >= hpUVPos.x && uv.x <= hpUVPos.x + hpUVSize.x * hpRate &&
         uv.y >= hpUVPos.y && uv.y <= hpUVPos.y + hpUVSize.y)
     {
         float2 localUV = (uv - hpUVPos) / hpUVSize;
         float4 hpColor = hpBarTex.Sample(hpBarSampler, localUV);
         if (hpColor.a > 0.01f) {
             color = hpColor; // 最優先で上書き
         }
     }

     if (color.a < 0.01f)
         discard;

     return color;

}
