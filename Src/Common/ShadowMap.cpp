#include <DxLib.h>
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "ShadowMap.h"

namespace
{
	//シャドウマップの描画サイズ
	constexpr int kShadowMapWidth = 2048;
	constexpr int kShadowMapHeight = 2048;
	//シャドウマップの範囲
	constexpr float kShadowMapHorizon = 1000.0f;
	constexpr float kShadowMapVertical = 1000.0f;
	//ライトの向き
	const VECTOR kLightDir = VGet(0.3f, -0.7f, 0.8f);
}

ShadowMap::ShadowMap(void)
	:
	shadowMapHandle_(-1)
{
}

ShadowMap::~ShadowMap(void)
{
	//シャドウマップの削除
	DeleteShadowMap(shadowMapHandle_);
}

void ShadowMap::Init(void)
{
	//シャドウマップ作成
	shadowMapHandle_ = MakeShadowMap(kShadowMapWidth, kShadowMapHeight);
}

void ShadowMap::DrawStart(void)
{
	if (shadowMapHandle_ >= 0)
	{
		//描画に使用するシャドウマップの設定を解除
		SetUseShadowMap(0, -1);

		// ライトの向きを一致させる
		SetLightDirection(kLightDir);
		SetShadowMapLightDirection(shadowMapHandle_, kLightDir);

		//シャドウマップ位置を計算(カメラ基準)
		auto cameraPos = SceneManager::GetInstance().GetCamera().lock()->GetPos();
		//最も低い位置を出す
		auto minPos = cameraPos;
		minPos.x -= kShadowMapHorizon;
		minPos.z -= kShadowMapHorizon;
		minPos.y -= kShadowMapVertical;
		//最も高い位置
		auto maxPos = cameraPos;
		maxPos.x += kShadowMapHorizon;
		maxPos.z += kShadowMapHorizon;
		maxPos.y += kShadowMapVertical;
		SetShadowMapDrawArea(shadowMapHandle_, minPos, maxPos);

		//シャドウマップへの描画の準備
		ShadowMap_DrawSetup(shadowMapHandle_);
	}
}

void ShadowMap::DrawEnd(void)
{
	if (shadowMapHandle_ >= 0)
	{
		//シャドウマップへの描画を終了
		ShadowMap_DrawEnd();
		//描画に使用するシャドウマップを設定
		SetUseShadowMap(0, shadowMapHandle_);
	}
}
