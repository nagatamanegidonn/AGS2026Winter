// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
	float4 g_color;
	float3 g_light_dir;
	float g_time;
	float4 g_ambient_color;
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{

	// テクスチャの色取得
   float4 diffuse = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
   if (diffuse.a < 0.01f)
   {
	   discard;
   }

   float dotColor = dot(PSInput.normal, -g_light_dir);
   /*float dotColor = dot(PSInput.normal, g_light_dir);
   dotColor += 1.0f;
   dotColor /= 2.0f;*/

   
   float3 rgb = (diffuse.rgb * dotColor) + g_ambient_color.rgb + (diffuse.rgb * 0.5);
   // float3 rgb=diffuse.rgb * (1 - dotColor);

   return float4(rgb, diffuse.a);


	// ③法線がワールド空間になっているか確認
	/*return float4(
		PSInput.normal.x, PSInput.normal.x, PSInput.normal.x, 1.0f);*/


	//青色になるはず
	/*return float4(g_light_dir.x, g_light_dir.y, g_light_dir.z, 1.0f);*/



	//方向で色が変わる
	/*float3 lightDir = g_common.light[0].direction;
	return float4(lightDir.x, lightDir.y, lightDir.z, 1.0f);*/

	return float4(1.0f,0.0f,0.0f,1.0f);

}