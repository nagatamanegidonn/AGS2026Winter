#pragma once
#include <map>
#include <vector>
#include <mutex>
#include "NetManager.h"
#include "NetStructures.h"

class NetBase
{

public:

	static constexpr float TIME_SEND_USER = 1.0f;
	static constexpr float TIME_SEND_USERS = 10.0f / 60.0f;
	static constexpr float TIME_SEND_ACTIONS = 1.0f / 60.0f;

	// コンストラクタ
	NetBase(NetManager& manager);

	// デストラクタ
	virtual ~NetBase(void);

	// 更新
	virtual void UpdateConnecting(void) = 0;
	virtual void UpdateGotoGame(void) = 0;
	virtual void UpdateGamePlaying(void) = 0;

	// 受信処理
	virtual void UdpReceiveThreadConnecting(void) = 0;
	virtual void UdpReceiveThreadGotoGame(void) = 0;
	virtual void UdpReceiveThreadGamePlaying(void) = 0;

protected:

	// 管理クラスの参照
	NetManager& manager_;

	// 更新ステップ
	float step_;
	float stepAction_;

};
