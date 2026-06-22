#include<DxLib.h>
#include "../Application.h"
#include "../Utility/Utility.h"
#include "../Utility/Measure.h"

#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/Camera.h"

#include "../Net/NetStructures.h"
#include "../Net/NetManager.h"

#include "RefreshScene.h"

namespace
{
	// シーン遷移・時間設定
	constexpr float SCENE_TRANSITION_TIME = 3.0f; // タイトルに戻るまでの待機時間
	// UI配置・フォント設定
	constexpr int FONT_SIZE_LARGE = 64;		// 結果表示のフォントサイズ
	constexpr int FONT_SIZE_DEFFULT = 16;	// デフォルトのフォントサイズ
	constexpr int RESULT_TEXT_Y = 100;		// 結果テキストを描画するY座標
	// カラーコード (RGB)
	constexpr unsigned int COLOR_LIGHT_BLUE = 0x87cefa;
	// ゲーム結果テキスト
	const std::wstring TEXT_RESULT_ERROR = L"ERROR";
	const std::wstring TEXT_RESULT_CLEAR = L"GAME_CLEAR";
	const std::wstring TEXT_RESULT_OVER = L"GAME_OVER";
	const std::wstring TEXT_RESULT_TIME = L"TIME_UP";
}

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
	stepCountDown_ = SCENE_TRANSITION_TIME;

	// カメラ設定
	SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FIXED_POINT);
}

void RefreshScene::Update(void)
{
	auto& nIns = NetManager::GetInstance();
	if (nIns.GetMode() == NET_MODE::HOST)
	{
		// ゲーム時間進行
		SceneManager::GetInstance().ForwardGameTime();// 進めるゲーム時間(GameTotalTime += デルタタイム)
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

	SceneManager& sns = SceneManager::GetInstance();
	GameManager& gns = GameManager::GetInstance();

	sns.DrawCapturedScreen(0, 0);

	std::wstring msg3 = L"";
	int cx = Application::SCREEN_SIZE_X / 2;
	int cy = Application::SCREEN_SIZE_Y / 2;

	SetFontSize(FONT_SIZE_LARGE);

	// ゲーム結果の表示
	switch (gns.GetGameResult())
	{
	case GameManager::GAME_RESULT::NONE:
		msg3 = TEXT_RESULT_ERROR;;
		break;
	case GameManager::GAME_RESULT::GAME_CLEAR:
		msg3 = TEXT_RESULT_CLEAR;
		break;
	case GameManager::GAME_RESULT::GAME_OVER:
		msg3 = TEXT_RESULT_OVER;
		break;
	case GameManager::GAME_RESULT::TIME_OVER:
		msg3 = TEXT_RESULT_TIME;
		break;
	}

	int len = (int)wcslen(msg3.c_str());
	int width = GetDrawStringWidth(msg3.c_str(), len);
	DrawFormatString(cx - (width / 2), RESULT_TEXT_Y, COLOR_LIGHT_BLUE, msg3.c_str());

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetFontSize(FONT_SIZE_DEFFULT);
}

void RefreshScene::Release(void)
{

}