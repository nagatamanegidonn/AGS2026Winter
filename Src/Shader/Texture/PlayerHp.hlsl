#include "../Common/Pixel/PixelShader2DHeader.hlsli"


Texture2D MaskTex : register(t1);
SamplerState MaskSampler : register(s1);


cbuffer cbParam : register(b4)
{
   float4 g_color;
    float hpRate;        // 0.0 ` 1.0
    float dameRate;        // 0.0 ` 1.0
    float2 g_none;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;
    float4 color = float4(0, 0, 0, 0);

    //•`‰æ‚µ‚È‚¢‚Æ‚±‚ë
    float4 maskColor = MaskTex.Sample(MaskSampler, uv);
    if (maskColor.a < 0.01f) {
        discard;
    }

   
    // --- HPƒo[i‚³‚ç‚Éã‚É•`‰æj ---
    if ( uv.x <=  dameRate )
    {
        float2 localUV = (uv ) ;
        float4 hpColor = float4(1, 0, 0, 1);
        if (hpColor.a > 0.01f) {
            color = hpColor; // Å—Dæ‚Åã‘‚«
        }
    }
    if ( uv.x <=  hpRate )
    {
        float2 localUV = (uv);
        float4 hpColor = g_color;
        if (hpColor.a > 0.01f) {
            color = hpColor; // Å—Dæ‚Åã‘‚«
        }
    }
    
    // HPƒtƒŒ[ƒ€
    float4 freamColor = tex.Sample(texSampler, uv);
    if (freamColor.a > 0.01f) {
        color = freamColor; // Å—Dæ‚Åã‘‚«
    }

    if (color.a < 0.01f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    return color;

}
