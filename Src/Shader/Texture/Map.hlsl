#include "../Common/Pixel/PixelShader2DHeader.hlsli"

Texture2D BossTex : register(t1);
SamplerState BossTexSampler : register(s1);

Texture2D PlayerTex : register(t2);
SamplerState PlayerTexSampler : register(s2);

cbuffer cbParam : register(b4)
{
    float2 bossUVPos;      // ボスのUV位置
    float2 bossUVSize;     // ボスのサイズ
    float2 playerUVRot;    // プレイヤーの向き（XZ正面）
    float2 playerUVSize;   // プレイヤーのサイズ
    float2 CameraRot;      // カメラ向き {cosθ, sinθ}
    float2 g_none;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;

    // --- カメラ回転を適用（ミニマップ全体を回転） ---
    float2 center = float2(0.5f, 0.5f);
    float2 offset = uv - center;

    float2 rotatedUV;
    rotatedUV.x = dot(offset, float2(CameraRot.x, -CameraRot.y)); // cosθ * dx - sinθ * dy
    rotatedUV.y = dot(offset, float2(CameraRot.y,  CameraRot.x)); // sinθ * dx + cosθ * dy
    rotatedUV += center;

    // --- マスク ---
    float4 maskColor = tex.Sample(texSampler, uv);
    if (maskColor.a < 0.01f) discard;

    float4 color = tex.Sample(texSampler, rotatedUV);

    // --- ボス描画 ---
    float2 bossTopLeft = bossUVPos - bossUVSize * 0.5;
    if (rotatedUV.x >= bossTopLeft.x && rotatedUV.x <= bossTopLeft.x + bossUVSize.x &&
        rotatedUV.y >= bossTopLeft.y && rotatedUV.y <= bossTopLeft.y + bossUVSize.y)
    {
        float2 localUV = (rotatedUV - bossTopLeft) / bossUVSize;
        float4 bossColor = BossTex.Sample(BossTexSampler, localUV);
        if (bossColor.a > 0.01f) color = bossColor;
    }

    // --- プレイヤー描画（回転あり） ---
    float2 playerTopLeft = float2(0.5f, 0.5f) - playerUVSize * 0.5;
    float2 playerCenter = playerTopLeft + playerUVSize * 0.5;

    float2 dir = normalize(playerUVRot);
    float2 right = float2(dir.y, -dir.x);
    float2 localUV = (rotatedUV - playerCenter) / playerUVSize;

    float2 rotatedPlayerUV;
    rotatedPlayerUV.x = dot(localUV, right);
    rotatedPlayerUV.y = dot(localUV, dir);
    rotatedPlayerUV = rotatedPlayerUV * 0.5 + 0.5;

    if (rotatedPlayerUV.x >= 0 && rotatedPlayerUV.x <= 1 &&
        rotatedPlayerUV.y >= 0 && rotatedPlayerUV.y <= 1)
    {
        float4 playerColor = PlayerTex.Sample(PlayerTexSampler, rotatedPlayerUV);
        if (playerColor.a > 0.01f) color = playerColor;
    }

    if (color.a < 0.01f) discard;

    return color;
}
