#pragma once
#include "NetBase.h"

class NetClient : public NetBase
{

public:

	// コンストラクタ
	NetClient(NetManager& manager);

	// デストラクタ
	~NetClient(void);

	// 更新
	void UpdateConnecting(void) override;
	void UpdateGotoGame(void) override;
	void UpdateGamePlaying(void) override;

protected:

	void UdpReceiveThreadGotoGame(void) override;
	void UdpReceiveThreadConnecting(void) override;
	void UdpReceiveThreadGamePlaying(void) override;

private:

};

