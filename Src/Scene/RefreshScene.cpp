#include<DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Utility/Measure.h"

#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/Camera.h"

#include "../Net/NetStructures.h"
#include "../Net/NetManager.h"

#include "RefreshScene.h"

RefreshScene::RefreshScene(void)
{
	stepCountDown_ = 0;
}

RefreshScene::~RefreshScene(void)
{
	NetManager::GetInstance().Stop();  // 通信スレッド停止 & ソケットクローズ
}

void RefreshScene::Init(void)
{
	// シーン遷移の待機時間
	stepCountDown_ = 3.0f;

	// カメラ設定
	SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FIXED_POINT);
}

void RefreshScene::Update(void)
{
	auto& nIns = NetManager::GetInstance();
	if (nIns.GetMode() == NET_MODE::HOST)
	{
		// ゲーム時間進行
		SceneManager::GetInstance().ForwardGameTime();// 進めるゲーム時間(GameTotalTime　+=　デルタタイム)
	}

	float limit = stepCountDown_ - SceneManager::GetInstance().GetTotalGameTime();
	if (limit > 0.0f)
	{
		return;
	}
	// ゲーム結果の初期化
	else
	{
		NetManager::GetInstance().ChangeGameState(GAME_STATE::NONE);
	}

	// シーンへ遷移
	SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);

	// マウスポインタを表示さ	せる
	SetMouseDispFlag(true);
}

void RefreshScene::Draw(void)
{

#ifdef _DEBUG
#endif

	SceneManager& sns = SceneManager::GetInstance();
	GameManager& gns = GameManager::GetInstance();

	sns.DrawCapturedScreen(0, 0);

	std::wstring msg3 = L"";
	int cx = Application::SCREEN_SIZE_X / 2;
	int cy = Application::SCREEN_SIZE_Y / 2;

	SetFontSize(64);

	// ゲーム結果の表示
	switch (gns.GetGameResult())
	{
	case GameManager::GAME_RESULT::NONE:
		msg3 = L"ERROR";
		break;
	case GameManager::GAME_RESULT::GAME_CLEAR:
		msg3 = L"GAME_CLEAR";
		break;
	case GameManager::GAME_RESULT::GAME_OVER:
		msg3 = L"GAME_OVER";
	case GameManager::GAME_RESULT::TIME_OVER:
		msg3 = L"TIME_UP";
		break;
	}

	int len = (int)wcslen(msg3.c_str());
	int width = GetDrawStringWidth(msg3.c_str(), len);
	DrawFormatString(cx - (width / 2), 100, 0x87cefa, msg3.c_str());

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetFontSize(16);
}

void RefreshScene::Release(void)
{

}