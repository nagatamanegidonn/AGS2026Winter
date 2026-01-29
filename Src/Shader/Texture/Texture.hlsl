#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
	float g_none;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
	float2 uv = PSInput.uv;

	// UV座標とテクスチャを参照して、最適な色を取得する
	float4 srcCol = tex.Sample(texSampler, uv);
	// アルファ値が0.9未満の場合は描画しない
	if (srcCol.a < 0.9f)
	{
		// 描画しない(アルファテスト)
		discard;
	}

	return srcCol;
}