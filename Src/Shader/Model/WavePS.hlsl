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
	float g_time; // 経過時間（秒）
	float3 g_camera_pos;
	float g_specular_pow;
};

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
	float2 uv = PSInput.uv;
	// 法線（ワールド空間）を取得して、上向きか判定
	float3 normal = normalize(PSInput.normal);

	float4 demoCol = (1.0f, 1.0f, 1.0f, 1.0f);

	// 上方向かどうか（Y軸成分が 0.9 以上のとき＝上面）
	if (abs(normal.y) > 0.9f) // Y軸にほぼ垂直な面だけ
	{
		// fracでタイル状に繰り返す
		uv = frac(uv * 3.0f);

		// 水流っぽくUVを動かす（x方向にスクロール）
		uv.x += g_time * 0.05f;

		// 波っぽく歪ませる
		uv.y += sin(g_time + uv.x * 10.0f) * 0.02f;
	}
	else
	{
		// fracでタイル状に繰り返す
		//uv.y /= frac(uv.y * 0.5f);
		//uv.y /= 2.0f;
		demoCol.rgb = 0.0f;


	}

	// テクスチャサンプリング
	float4 col = diffuseMapTexture.Sample(diffuseMapSampler, uv);
	demoCol = (col.b, col.g, col.r, col.a);

	// 上面以外（側面）だけ処理
	if (abs(normal.y) < 0.9f)
	{
		// ライン位置（時間で移動可能）
		float lineCenter = fmod(g_time * 0.2f, 1.0f); // 0～1 でループするライン位置
		float lineWidth = 0.2f; // ラインの幅

		// ラインに近ければ色を加算
		float dist = abs(uv.x - lineCenter);
		float lineMask = smoothstep(lineWidth, 0.0f, dist); // 柔らかい境界

		float rainbowVal = uv.y * 6.2831f; // 0~2π
		float3 rainbowColor = float3(
			sin(rainbowVal + 0.0f),
			sin(rainbowVal + 2.094f), // +2PI/3
			sin(rainbowVal + 4.188f)  // +4PI/3
			) * 0.5f + 0.5f; // 0~1に補正

		col.rgb += lineMask * rainbowColor * 0.5f;
	}

	return col;

}