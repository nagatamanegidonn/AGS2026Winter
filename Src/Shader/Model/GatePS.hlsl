// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

#define SUBTEXTUREMODE 1

// IN
#define PS_INPUT VertexToPixelLit


// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbD3D11_CONST_BUFFER_VS_LOCALWORLDMATRIX : register(b4)
{
	float4 g_color;
	float3 g_light_dir;
	float g_time;
	float4 g_ambient_color;
	float g_rate;
	float3 g_none;
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
	float2 uv = PSInput.uv;
	float3 normal = normalize(PSInput.normal);

	// 霧のゆらゆらスクロール
	{
		uv.y -= g_time * 0.05f;
		uv.x += sin(uv.y * 0.2f + g_time) * 0.05f;
		uv = frac(uv * 3.0f);
	}

	float4 diffuse = diffuseMapTexture.Sample(diffuseMapSampler, uv);
	float4 subDiffuse = subTexture.Sample(subSampler, uv); // これがノイズテクスチャ

	// --- ディゾルブのアルファ計算 ---
	// ノイズテクスチャのRGB平均値を取得（0.0～1.0の範囲）
	float noiseValue = (subDiffuse.r + subDiffuse.g + subDiffuse.b) / 3.0f;

	// 溶け始める境界の幅 (例: 0.1)
	float dissolveWidth = 0.1f;

	// アルファ値を計算
	// noiseValueがg_rateより小さいほど、アルファは0に近づく
	// smoothstepを使って滑らかなフェードを実現
	// noiseValue < g_rate の部分が先に透明になる
	float dissolveAlphaFactor = smoothstep(g_rate - dissolveWidth, g_rate, noiseValue);

	// 最終的なアルファ値は、元のdiffuse.aにこのdissolveAlphaFactorを掛け合わせる
	float alpha = diffuse.a * dissolveAlphaFactor;

	// 黒部分が先に透明になる例:
	//(g_rateが小さい) -> noiseValueが小さい部分(黒い部分)が g_rate を下回るので透明になる
	// (g_rateが大きい) -> より広い範囲(白い部分も含む)が g_rate を下回るので透明になる
	// 完全に溶かすには g_rate を 1.0 + dissolveWidth くらいまで増加させる
	// --- ディゾルブのアルファ計算終了 ---

	// アルファ値が0未満にならないようにクランプ（0～1の範囲に収める）
	alpha = saturate(alpha);

	// 光の計算 (既存のロジック)
	float dotColor = dot(PSInput.normal, -g_light_dir);
	//float3 rgb = (diffuse.rgb * dotColor) + g_ambient_color.rgb + (diffuse.rgb * 0.5);
	float3 rgb = diffuse.rgb + g_ambient_color.rgb + (diffuse.rgb * 0.5);

	//色の設定
	float3 finalRgb = rgb * float3(0.2, 0.2, 0.5); //本来の色+追加の色
	float4 edgeColor = float4(1.0, 0.0, 0.0, 1.0); //輪郭の色

	// 溶ける境界線（エッジ）の色をブレンド
	// noiseValueが閾値の付近にある場合に色を強める
	float edgeBlendFactor = 1.0 - abs(noiseValue - g_rate) / dissolveWidth;//画像のRGB値がg_rateと近いなら輪郭の色を付ける
	edgeBlendFactor = saturate(edgeBlendFactor * 2.0); // 鋭くするために調整（0~1に収める）

	// 完全に透明になる手前で色を付ける
	// alphaが低いほど（透明に近いほど）、この輪郭色が影響するようにする
	finalRgb += (edgeColor.rgb * edgeBlendFactor * (1.0  -dissolveAlphaFactor)); // dissolveAlphaFactorが小さいほど(透明に近いほど)輪郭が強く出る

	// 最終的な出力カラー
	float4 finalColor = float4(finalRgb, alpha);
	return finalColor;

}