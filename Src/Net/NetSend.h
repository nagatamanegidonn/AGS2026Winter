#pragma once
#include "NetStructures.h"

class NetSend
{

public :

	NetSend(NetManager& manager, int& sendSocketId);
	~NetSend(void);

	// 送信処理
	void Send(NET_DATA_TYPE type);

private:

	NetManager& manager_;

	// 送信用ソケットハンドル
	int& sendSocketId_;

	// 送信情報
	NET_DATA_TYPE type_;

	// 送受信用の基本データを作成する
	NET_BASIC_DATA MakeBasicData(NET_DATA_TYPE type, std::uint32_t crc);

	// ホストへ送信
	void SendUDP_Host(const char* bufPtr, int bufSize);

	// クライアントへ送信
	void SendUDP_Clients(const char* bufPtr, int bufSize);

	// 各種送信処理
	void SendJoinPlayer(const NET_JOIN_USER& entity);
	void SendSelfJoinPlayer(void);
	void SendJoinPlayers(void);
	void SendActionHisAll(void);

};
