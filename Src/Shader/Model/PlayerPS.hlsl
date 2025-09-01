// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
	float3 g_light_dir;
	float g_time;
	float3 g_camera_pos;
	float g_specular_pow;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
	// テクスチャの色取得
	float4 diffuse = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
	if (diffuse.a < 0.01f)
	{
		discard;
	}
	diffuse = float4(0.0f, 0.0f, 0.0f, 1.0f);

	//return diffuse;

	float3 cameraDir = normalize(PSInput.worldPos - g_camera_pos);
	float cDot = dot(PSInput.normal, cameraDir);
	cDot = abs(cDot);
	cDot = 1.0f - cDot;

	float rimDot = pow(cDot, 2.0f); // 強弱を強める

	// 色相 (Hue)：0.0~1.0 を周期的に変化させる
	float hue = frac(g_time * 0.2f); // 少し速めに回転

    // HSV to RGB (S=1, V=1)
	float h = hue * 6.0f;
	float3 rgb;
	float c = 1.0f;
	float x = c * (1.0f - abs(fmod(h, 2.0f) - 1.0f));

	if (h < 1.0f)      rgb = float3(c, x, 0);
	else if (h < 2.0f) rgb = float3(x, c, 0);
	else if (h < 3.0f) rgb = float3(0, c, x);
	else if (h < 4.0f) rgb = float3(0, x, c);
	else if (h < 5.0f) rgb = float3(x, 0, c);
	else              rgb = float3(c, 0, x);

	// 明度を調整（少し控えめにすると発色が良く見える）
	rgb = lerp(float3(0.2f, 0.2f, 0.2f), rgb, 0.8f);

	float4 rimColor = float4(rgb, 1.0f);
	//float4 rimColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	diffuse = diffuse + rimColor * rimDot;

	return diffuse;


	 return float4(cDot, cDot, cDot, 1.0f);
}