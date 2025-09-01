#pragma once
#include "NetBase.h"

class NetHost : public NetBase
{

public:

	// コンストラクタ
	NetHost(NetManager& manager);

	// デストラクタ
	~NetHost(void);

	// 更新
	void UpdateConnecting(void) override;
	void UpdateGotoGame(void) override;
	void UpdateGamePlaying(void) override;

protected:

	void UdpReceiveThreadConnecting(void) override;
	void UdpReceiveThreadGotoGame(void) override;
	void UdpReceiveThreadGamePlaying(void) override;

private:


};

