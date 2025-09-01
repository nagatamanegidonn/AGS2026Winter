#include <iostream>
#include <thread>
#include <DxLib.h>
#include "NetManager.h"
#include "../Manager/SceneManager.h"
#include "NetClient.h"

NetClient::NetClient(NetManager& manager) : NetBase(manager)
{
}

NetClient::~NetClient(void)
{
}


#pragma region StateごとのUpdate

void NetClient::UpdateConnecting(void)
{
	step_ += SceneManager::GetInstance().GetDeltaTime();
	if (step_ >= TIME_SEND_USER)
	{
		step_ = 0.0f;
		manager_.Send(NET_DATA_TYPE::USER);
	}
}
void NetClient::UpdateGotoGame(void)
{
	step_ += SceneManager::GetInstance().GetDeltaTime();
	if (step_ >= TIME_SEND_USER)
	{
		step_ = 0.0f;
		manager_.Send(NET_DATA_TYPE::USER);
	}
}
void NetClient::UpdateGamePlaying(void)
{
	// 毎フレーム送信
	manager_.Send(NET_DATA_TYPE::ACTION_HIS_ALL);
}

#pragma endregion


void NetClient::UdpReceiveThreadConnecting(void)
{
	// ホストがゲーム画面へ遷移したら、
	// クライアントも一斉に遷移する
	if (manager_.GetGameStateHost() >= GAME_STATE::GOTO_GAME)
	{
		manager_.ChangeGameState(GAME_STATE::GOTO_GAME);
	}
}

void NetClient::UdpReceiveThreadGamePlaying(void)
{
}

void NetClient::UdpReceiveThreadGotoGame(void)
{
}
