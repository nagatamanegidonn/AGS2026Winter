// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

//個人用バッファ
cbuffer cbD3D11_CONST_BUFFER_VS_LOCALWORLDMATRIX : register(b4)
{
	float3 g_light_dir;
	float g_none;
	float3 g_camera_pos;
	float g_specular_pow;
	float4 g_ambient_color;
};

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
	// テクスチャの色取得
	float4 diffuse = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
	if (diffuse.a < 0.01f)
	{
		discard;
	}

	//ライトの方向と法線の内積　反射率
	float dotColor = dot(PSInput.normal, -g_light_dir);
	float3 Ranpate = (diffuse.rgb * dotColor) + g_ambient_color.rgb + diffuse.rgb;
	diffuse.rgb = Ranpate;

	float3 camera_dir= normalize(g_camera_pos - PSInput.worldPos);
	float cDot = dot(PSInput.normal, camera_dir);
	cDot = abs(cDot);
	cDot = 1.0f - cDot;

	float rimDot = pow(cDot, 2.0f); // 強弱を強める
	float4 rimColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	diffuse = diffuse + (rimColor * rimDot);

	return diffuse;
}