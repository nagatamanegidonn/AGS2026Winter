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
	float3 Ranpate = (diffuse.rgb * dotColor) + g_ambient_color.rgb+ diffuse.rgb;

	//float3 reflectDir = reflect(-g_light_dir, PSInput.normal); // 入射ベクトルの反射方向
	float3 reflectDir = g_light_dir + PSInput.normal + PSInput.normal;
	float3 refVec = normalize(reflectDir);

	float3 cameraDir = normalize(g_camera_pos - PSInput.worldPos);
	float refDot = abs(dot(refVec, cameraDir));
	float refDotEx = pow(refDot, g_specular_pow);


	// 色の合成
	//float3 rgb = diffuse + (refDotEx * (1 - dotColor))/* + g_ambient_color*/;
	float3 lighting = (refDotEx * dotColor);
	float3 rgb = max(Ranpate, Ranpate + lighting);

	return float4(rgb, diffuse.a);
}