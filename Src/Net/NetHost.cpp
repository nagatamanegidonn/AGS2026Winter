#include <iostream>
#include <thread>
#include <DxLib.h>
#include "NetManager.h"
#include "../Manager/SceneManager.h"
#include "NetHost.h"

NetHost::NetHost(NetManager& manager) : NetBase(manager)
{
}

NetHost::~NetHost(void)
{
}

#pragma region State‚²‚Æ‚ÌUpdate

void NetHost::UpdateConnecting(void)
{
	step_ += SceneManager::GetInstance().GetDeltaTime();
	if (step_ >= TIME_SEND_USERS)
	{
		step_ = 0.0f;
		manager_.Send(NET_DATA_TYPE::USERS);
	}
}
void NetHost::UpdateGotoGame(void)
{
	step_ += SceneManager::GetInstance().GetDeltaTime();
	if (step_ >= TIME_SEND_USERS)
	{
		step_ = 0.0f;
		manager_.Send(NET_DATA_TYPE::USERS);
	}
}
void NetHost::UpdateGamePlaying(void)
{
	// –ˆƒtƒŒ[ƒ€‘—M
	manager_.Send(NET_DATA_TYPE::ACTION_HIS_ALL);
}

#pragma endregion


void NetHost::UdpReceiveThreadConnecting(void)
{
}

void NetHost::UdpReceiveThreadGotoGame(void)
{
}

void NetHost::UdpReceiveThreadGamePlaying(void)
{
}
