#include <queue>
#include "../Common/CRC.h"
#include "../Manager/SceneManager.h"
#include "../Net/NetManager.h"
#include "NetSend.h"

NetSend::NetSend(NetManager& manager, int& sendSocketId) 
	: manager_(manager), sendSocketId_(sendSocketId)
{
	type_ = NET_DATA_TYPE::NONE;
}

NetSend::~NetSend(void)
{
}

void NetSend::Send(NET_DATA_TYPE type)
{

	type_ = type;

	switch (type_)
	{
	case NET_DATA_TYPE::USER:
		SendSelfJoinPlayer();
		break;
	case NET_DATA_TYPE::USERS:
		SendJoinPlayers();
		break;
	case NET_DATA_TYPE::ACTION_HIS_ALL:
		SendActionHisAll();
		break;
	}

}

NET_BASIC_DATA NetSend::MakeBasicData(NET_DATA_TYPE type, std::uint32_t crc)
{
	NET_BASIC_DATA ret;
	ret.type = type;
	ret.key = manager_.GetSelf().key;
	ret.gameTime = SceneManager::GetInstance().GetTotalGameTime();
	ret.crc = crc;
	return ret;
}

#pragma region --へUDP通信

void NetSend::SendUDP_Host(const char* bufPtr, int bufSize)//ホストへデータ送信
{
	// ホストへ送信
	NetWorkSendUDP(sendSocketId_, manager_.GetHostIp(), manager_.GetHostPort(), bufPtr, bufSize);
}

void NetSend::SendUDP_Clients(const char* bufPtr, int bufSize)//clientへデータ送信
{

	// 自分以外へ送信
	NET_JOIN_USER user;

	auto& users = manager_.GetNetUsers();
	for (const auto& userPair : users)
	{//Hostだから自分以外の全員に自分の情報を送る

		user = userPair.second;
		if (user.key == manager_.GetSelf().key)
		{
			// 自分宛てには送信しない
			continue;
		}

		// 自分以外へ送信(送る場所のソケット,Ipアドレス,ポート番号,送る情報,情報の大きさ)
		NetWorkSendUDP(sendSocketId_, user.ip, user.port, bufPtr, bufSize);

	}

}

#pragma endregion


#pragma region リソースManagerとリソースの関係に近く,typeにより送るデータを変える

void NetSend::SendJoinPlayer(const NET_JOIN_USER& entity)
{

	const size_t SIZE_BASIC		= sizeof(NET_BASIC_DATA);//送る情報の大きさ
	const size_t SIZE_ENTITY	= sizeof(NET_JOIN_USER);//ユーザーの大きさ
	const size_t SIZE_ALL		= SIZE_BASIC + SIZE_ENTITY;

	// 送信データのCRC
	std::uint32_t crc = CRC::Calculate(&entity, SIZE_ENTITY, CRC::CRC_32());

	// バッファ確保
	char buf[SIZE_ALL] = { 0 };
	char* bufPtr = buf;

	// 基本データを頭に付けるため作成
	NET_BASIC_DATA basicData = MakeBasicData(type_, crc);

	// 基本データをメモリコピー
	memcpy(bufPtr, &basicData, SIZE_BASIC);
	bufPtr += SIZE_BASIC;

	// 送信データをメモリコピー
	memcpy(bufPtr, &entity, SIZE_ENTITY);

#pragma region 送信データをバラバラに設定する場合

	//// 送信データ
	//memcpy(bufPtr, &entity.mode, sizeof(NET_MODE));
	//bufPtr += sizeof(NET_MODE);

	//memcpy(bufPtr, &entity.key, sizeof(int));
	//bufPtr += sizeof(int);

	//memcpy(bufPtr, &entity.ip, sizeof(IPDATA));
	//bufPtr += sizeof(IPDATA);

	//memcpy(bufPtr, &entity.port, sizeof(int));
	//bufPtr += sizeof(int);

	//memcpy(bufPtr, &entity.playerType, sizeof(PLAYER_TYPE));
	//bufPtr += sizeof(PLAYER_TYPE);

	//memcpy(bufPtr, &entity.gameState, sizeof(GAME_STATE));
	//bufPtr += sizeof(GAME_STATE);

#pragma endregion

	// 送信
	SendUDP_Host(buf, SIZE_ALL);

}

void NetSend::SendSelfJoinPlayer(void)
{
	SendJoinPlayer(manager_.GetSelf());
}

void NetSend::SendJoinPlayers(void)//接続シーンの通信
{

	const size_t SIZE_BASIC = sizeof(NET_BASIC_DATA);
	const size_t SIZE_ENTITY = sizeof(NET_JOIN_USERS);
	const size_t SIZE_ALL = SIZE_BASIC + SIZE_ENTITY;

	// 送信データの作成
	NET_JOIN_USERS entity;
	auto& users = manager_.GetNetUsers();
	int cnt = 0;
	for (const auto& userPair : users)
	{
		if (cnt >= users.size()) break; // 配列サイズ以上はコピーしない！
		entity.users[cnt] = userPair.second;
		cnt++;
	}

	// 送信データのCRC
	std::uint32_t crc = CRC::Calculate(&entity, SIZE_ENTITY, CRC::CRC_32());

	// バッファ確保
	char buf[SIZE_ALL] = { 0 };
	char* bufPtr = buf;

	// 基本データを頭に付けるため作成
	NET_BASIC_DATA basicData = MakeBasicData(type_, crc);

	// 基本データをメモリコピー
	memcpy(bufPtr, &basicData, SIZE_BASIC);
	bufPtr += SIZE_BASIC;//コピーしたデータをバッファに追加？

	// 送信データをメモリコピー
	memcpy(bufPtr, &entity, SIZE_ENTITY);

	// 送信
	SendUDP_Clients(buf, SIZE_ALL);



	//// 基本データを頭に付ける
	//const int size = sizeof(NET_BASIC_DATA) + sizeof(NET_JOIN_USERS);
	//char buf[size];
	//char* bufPtr = buf;

	//// 基本データを作成する
	//NET_BASIC_DATA basicData = MakeBasicData(type_);
	//memcpy(bufPtr, &basicData, sizeof(NET_BASIC_DATA));
	//bufPtr += sizeof(NET_BASIC_DATA);

	//// 送信データ
	//auto& users = manager_.GetNetUsers();
	//for (const auto& userPair : users)
	//{
	//	memcpy(bufPtr, &userPair.second, sizeof(NET_JOIN_USER));
	//	bufPtr += sizeof(NET_JOIN_USER);
	//}

	//// 送信
	//SendUDP_Clients(buf, size);

}

void NetSend::SendActionHisAll(void)//ゲームシーンの通信
{

	const size_t SIZE_BASIC = sizeof(NET_BASIC_DATA);
	const size_t SIZE_ENTITY = sizeof(NET_ACTION_HIS);

	const size_t SIZE_ALL = SIZE_BASIC + SIZE_ENTITY;

	// 送信データの作成
	auto self = manager_.GetSelfAction();
	manager_.MakeActionHis(self);
	NET_ACTION_HIS entity = manager_.GetSelfActionHis();

	// 送信データのCRC
	std::uint32_t crc = CRC::Calculate(&entity, SIZE_ENTITY, CRC::CRC_32());

	// バッファ確保
	char buf[SIZE_ALL] = { 0 };
	char* bufPtr = buf;

	// 基本データを頭に付けるため作成
	NET_BASIC_DATA basicData = MakeBasicData(type_, crc);

	// 基本データをメモリコピー
	memcpy(bufPtr, &basicData, SIZE_BASIC);
	bufPtr += SIZE_BASIC;

	// 送信データをメモリコピー
	memcpy(bufPtr, &entity, SIZE_ENTITY);


	/*printfDx("[送信] key: %d | 位置: %.2f, %.2f\n",
		entity.key,
		entity.selfPostion_.x, entity.selfPostion_.y);*/


	// 送信
	SendUDP_Clients(buf, SIZE_ALL);






	//// 基本データを頭に付ける
	//const int size = sizeof(NET_BASIC_DATA) + sizeof(NET_ACTION_HIS);
	//char buf[size];
	//char* bufPtr = buf;

	//// 基本データを作成する
	//NET_BASIC_DATA basicData = MakeBasicData(type_);
	//memcpy(bufPtr, &basicData, sizeof(NET_BASIC_DATA));
	//bufPtr += sizeof(NET_BASIC_DATA);

	//// 送信データ
	//memcpy(bufPtr, &entity, sizeof(NET_ACTION_HIS));

	//// 送信
	//SendUDP_Clients(buf, size);






	//auto entity = manager_.GetSelfAction();
	//manager_.MakeActionHis(entity);
	//auto his = manager_.GetSelfActionHis();

	//// 基本データを頭に付ける
	//const int size = sizeof(NET_BASIC_DATA) + sizeof(NET_ACTION_HIS);
	//char buf[size];
	//char* bufPtr = buf;

	//// 基本データを作成する
	//NET_BASIC_DATA basicData = MakeBasicData(type_);
	//memcpy(bufPtr, &basicData, sizeof(NET_BASIC_DATA));
	//bufPtr += sizeof(NET_BASIC_DATA);

	//// 送信データ
	//memcpy(bufPtr, &his, sizeof(NET_ACTION_HIS));

	//// 送信
	//SendUDP_Clients(buf, size);

}

#pragma endregion
