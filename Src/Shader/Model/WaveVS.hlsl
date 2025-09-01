// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#include "../Common/Vertex/VertexInputType.hlsli"
#define VERTEX_INPUT DX_MV1_VERTEX_TYPE_NMAP_1FRAME

// OUT
#define VS_OUTPUT VertexToPixelLit
#include "../Common/Vertex/VertexShader3DHeader.hlsli"

//個人用バッファ
// 時間を受け取る定数バッファを b7 に変更
cbuffer cbTimeBuffer : register(b7)
{
	float3 g_none;
	float g_time;
};

VS_OUTPUT main(VS_INPUT VSInput)
{
	VS_OUTPUT ret;

	// 頂点座標変換 +++++++++++++++++++++++++++++++++++++( 開始 )
	float4 lLocalPosition;
	float4 lWorldPosition;
	float4 lViewPosition;

	// float3 → float4
	lLocalPosition.xyz = VSInput.pos;



	float3 localPos = VSInput.pos;

	// 波のパラメータ
	float waveHeight = 0.2f;     // 波の高さ
	float waveLength = 4.0f;     // 波の密度（小さいとギザギザ、大きいとゆるやか）
	float waveSpeed = 2.0f;     // 波の進み速度

	// X座標に応じたYの変化（sin波）
	localPos.y += sin(localPos.x * waveLength + g_time * waveSpeed) * waveHeight;

	lLocalPosition.xyz = localPos;
	lLocalPosition.w = 1.0f;



	// ローカル座標をワールド座標に変換(剛体)
	lWorldPosition.w = 1.0f;
	lWorldPosition.xyz = mul(lLocalPosition, g_base.localWorldMatrix);



	// ワールド座標をビュー座標に変換
	lViewPosition.w = 1.0f;
	lViewPosition.xyz = mul(lWorldPosition, g_base.viewMatrix);
	ret.vwPos.xyz = lViewPosition.xyz;

	// ビュー座標を射影座標に変換
	ret.svPos = mul(lViewPosition, g_base.projectionMatrix);

	// 頂点座標変換 +++++++++++++++++++++++++++++++++++++( 終了 )



	// その他、ピクセルシェーダへ引継&初期化 ++++++++++++( 開始 )
	// UV座標
	ret.uv.x = VSInput.uv0.x;
	ret.uv.y = VSInput.uv0.y;

	// 法線
	//ret.normal = VSInput.norm;
	// 法線をローカル空間からワールド空間へ変換
	ret.normal = normalize(
		mul(VSInput.norm, (float3x3)g_base.localWorldMatrix));

	// ディフューズカラー
	ret.diffuse = VSInput.diffuse;
	// ライト方向(ローカル)
	ret.lightDir = float3(0.0f, 0.0f, 0.0f);
	// ライトから見た座標
	ret.lightAtPos = float3(0.0f, 0.0f, 0.0f);
	// その他、ピクセルシェーダへ引継&初期化 ++++++++++++( 終了 )


	// 出力パラメータを返す
	return ret;
}