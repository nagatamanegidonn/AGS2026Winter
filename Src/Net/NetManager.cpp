//#include <iostream>
#include <thread>
#include <DxLib.h>
#include "../Common/CRC.h"
#include "../Manager/SceneManager.h"
#include "NetHost.h"
#include "NetSend.h"
#include "NetClient.h"
#include "NetManager.h"

NetManager* NetManager::instance_ = nullptr;

void NetManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new NetManager();
	}
}

NetManager& NetManager::GetInstance(void)
{
	return *instance_;
}

void NetManager::Destroy(void)
{
	instance_->Stop();
}

NET_MODE NetManager::GetMode(void) const
{
	return mode_;
}

bool NetManager::IsHost(void) const
{
	return mode_ == NET_MODE::HOST;
}

void NetManager::Run(NET_MODE mode)
{

	isRunning_ = true;

	// ホスト or クライアント
	mode_ = mode;

	int port = 0;
	switch (mode_)
	{
	case NET_MODE::HOST://モードがホストなら
		instance_->net_ = new NetHost(*instance_);
		port = RECEIVE_PORT_HOST;
		break;
	case NET_MODE::CLIENT://モードがクライアントなら
		instance_->net_ = new NetClient(*instance_);
		port = RECEIVE_PORT_CLIENT_RANGE + GetRand(899);
		break;
	}

	// 自分のネットユーザ情報
	auto& self = poolShare_.selfJoinUser_;
	self.mode = mode_;
	self.key = GetRand(999999);
	GetMyIPAddress(&self.ip);
	self.port = port;
	self.gameState = GAME_STATE::NONE;

	// 自分のアクション情報
	poolShare_.selfAction_.key = self.key;
	poolShare_.selfActionHis_.key = self.key;

	// UDPを使用した通信を行うソケットハンドルを作成する
	sendSocketId_ = MakeUDPSocket(-1);
	recvSocketId_ = MakeUDPSocket(port);

	// 自分自身をプレイヤーに追加する
	poolShare_.joinUsers_.emplace(self.key, self);

	// データのコピー
	pool_ = poolShare_;

	// 受信処理を行うスレッドを起動する
	recvThread_ = std::thread(&NetManager::UdpReceiveThread, this);

	// 送信用クラスの作成
	netSend_ = new NetSend(*instance_, sendSocketId_);

	frameNo_ = 0;
	isSync_ = false;

}

void NetManager::Init(void)
{
	Stop(); // 万が一前回のセッションが生きてたら止める

   // ソケット・ユーザー情報などを初期化
	if (instance_ != nullptr)
	{
		instance_ = new NetManager();
	}

	// ソケット再作成やポート設定などが必要ならここで行う
}

void NetManager::Update(void)
{

	if (net_ == nullptr)
	{
		// Runが実行されるまで処理しない
		return;
	}

	if (isSync_)//ゲーム通信中なら
	{

		// TODO 完全同期になっているので、ある程度非同期でも実行できるようにしたい
		while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
		{
			// 通信データ反映
			DataReflection();

			// 全ユーザの最新フレームが揃うまで待つ
			if (IsSameFrameNo())
			{
				timeoutCounter_ = 0; // 揃ったらリセット
				break;
			}
			else
			{
				timeoutCounter_ += SYNC_TERM_MSEC; // WaitTimer の時間分進める
				if (timeoutCounter_ >= TIMEOUT_LIMIT)
				{
#ifdef _DEBUG
					printfDx(L"[警告] 同期タイムアウト。進行を強制。\n");
#endif // DEBUG
					isSync_ = false; // 同期解除 or 状態遷移など
					NetManager::GetInstance().Stop();
					SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);

					break;
				}
			}

			// 状態別更新(主に送信処理)
			UpdateState();

			// 一定時間待機
			WaitTimer(static_cast<int>(SYNC_TERM_MSEC));
		}

	}
	else
	{
		// 通信データ反映
		DataReflection();
	}

	// 状態別更新(主に送信処理)
	UpdateState();

}

void NetManager::UpdateState(void)
{

	switch (pool_.selfJoinUser_.gameState)
	{
	case GAME_STATE::CONNECTING:
		net_->UpdateConnecting();
		break;
	case GAME_STATE::GOTO_GAME:
		net_->UpdateGotoGame();
		break;
	case GAME_STATE::GAME_PLAYING:
		net_->UpdateGamePlaying();
		break;
	}

}

void NetManager::DataReflection(void)
{

	// 排他制御
	std::lock_guard<std::mutex> lock(poolMutex_);

	// 自分自身をリスト更新

	// 固有データから共有データへコピー
	poolShare_.selfJoinUser_ = pool_.selfJoinUser_;
	poolShare_.selfAction_ = pool_.selfAction_;
	poolShare_.selfAction_.frameNo = frameNo_;
	poolShare_.selfActionHis_ = pool_.selfActionHis_;

	// アクション履歴を共有に反映
	ReplaceActionHis(poolShare_.selfActionHis_);

	// ホストならユーザー情報も上書き
	if (IsHost())
	{
		ReplaceUser(poolShare_.selfJoinUser_);
	}

	// これが必要！
	pool_.joinUserActionHis_ = poolShare_.joinUserActionHis_;

	// 最終コピー
	pool_ = poolShare_;

}

void NetManager::UpdateEndOfFrame(void)
{
	if (isSync_)//ゲームプレイ中なら
	{
		// 同期中のフレーム進行
		NextFrame();
	}
}

void NetManager::Stop(void)
{

	DeleteUDPSocket(sendSocketId_);
	DeleteUDPSocket(recvSocketId_);

	if (isRunning_) {

		isRunning_ = false;

		if (recvThread_.joinable()) {
			recvThread_.join();
		}

	}

}

void NetManager::Send(NET_DATA_TYPE type)
{
	netSend_->Send(type);
}

void NetManager::ChangeGameState(GAME_STATE state)
{

	if (pool_.selfJoinUser_.gameState == state)
	{
		return;
	}

	pool_.selfJoinUser_.gameState = state;

	switch (state)
	{
	case GAME_STATE::NONE:
		break;
	case GAME_STATE::CONNECTING:
		break;
	case GAME_STATE::GOTO_GAME:
		break;
	case GAME_STATE::GAME_PLAYING:
		frameNo_ = 1;
		isSync_ = true;
		pool_.selfAction_.frameNo = frameNo_;
		break;
	}

}

NetManager::NetManager(void)
{
	isRunning_ = false;
	frameNo_ = 0;
	mode_ = NET_MODE::NONE;
	net_ = nullptr;
	netSend_ = nullptr;
	isRunning_ = false;
	recvSocketId_ = -1;
	sendSocketId_ = -1;
	isSync_ = false;
}

NetManager::NetManager(const NetManager& ins)
{
	isRunning_ = false;
	frameNo_ = 0;
	mode_ = NET_MODE::NONE;
	net_ = nullptr;
	netSend_ = nullptr;
	isRunning_ = false;
	recvSocketId_ = -1;
	sendSocketId_ = -1;
	isSync_ = false;
}
NetManager::~NetManager(void)
{
	Stop();
}

void NetManager::UdpReceiveThread(void)
{

	while (isRunning_)
	{

		{
			// 排他制御
			std::lock_guard<std::mutex> lock(poolMutex_);

			// データの受信
			UdpReceiveData();
		}

		GAME_STATE state = GetGameStateSelf();

		switch (state)
		{
		case GAME_STATE::NONE:
			break;
		case GAME_STATE::CONNECTING:
			net_->UdpReceiveThreadConnecting();
			break;
		case GAME_STATE::GOTO_GAME:
			net_->UdpReceiveThreadGotoGame();
			break;
		case GAME_STATE::GAME_PLAYING:
			net_->UdpReceiveThreadGamePlaying();
			break;
		}

		// 受信ループの待機
		WaitTimer(static_cast<int>(NetManager::RECEIVE_TERM_MSEC));

	}

}

void NetManager::UdpReceiveData(void)
{

	const int SIZE_BASIC = sizeof(NET_BASIC_DATA);

	for (int i = 0; i < NetManager::MAX_RECEIVE_NUM; i++)
	{

		char buf[MAX_SEND_BYTES] = { 0 };
		char* bufptr = buf;

		// UDPデータを受信
		int bufSize = NetWorkRecvUDP(
			recvSocketId_, nullptr, nullptr, bufptr, MAX_SEND_BYTES, false);

		if (bufSize == -3)
		{
			// 受信データがない
			break;
		}
		if (bufSize == -1 || bufSize == -2)
		{
			// -1 エラー発生 －２：エラー発生（受信データよりバッファのサイズの方が小さい）
			continue;
		}

		// 基本データサイズチェック
		int headSize = SIZE_BASIC;
		if (bufSize >= headSize)
		{

			// 基本データを先に取得して通信種別で処理を振り分ける
			NET_BASIC_DATA data = GetBasicData(bufptr);
			bufptr += SIZE_BASIC;

			switch (data.type)
			{
			case NET_DATA_TYPE::USER:
			{
				if (bufSize == SIZE_BASIC + sizeof(NET_JOIN_USER))
				{
					NET_JOIN_USER* cast = reinterpret_cast<NET_JOIN_USER*>(bufptr);
					// CRCチェック
					std::uint32_t crc = CRC::Calculate(cast, sizeof(NET_JOIN_USER), CRC::CRC_32());
					if (crc == data.crc)
					{
						ReplaceUser(*cast);
					}
				}
				break;
			}
			case NET_DATA_TYPE::USERS:
			{
				if (bufSize == SIZE_BASIC + sizeof(NET_JOIN_USERS))
				{
					if (!IsHost() && data.key == pool_.hostJoinUser_.key)
					{
						SceneManager::GetInstance().SetTotalGameTime(data.gameTime);
					}
					NET_JOIN_USERS* cast = reinterpret_cast<NET_JOIN_USERS*>(bufptr);
					// CRCチェック
					std::uint32_t crc = CRC::Calculate(cast, sizeof(NET_JOIN_USERS), CRC::CRC_32());
					if (crc == data.crc)
					{
						ReplaceUsers(*cast);
					}
				}
				break;
			}
			case NET_DATA_TYPE::ACTION_HIS_ALL:
			{
				if (bufSize == SIZE_BASIC + sizeof(NET_ACTION_HIS))
				{
					if (!IsHost() && data.key == pool_.hostJoinUser_.key)
					{
						SceneManager::GetInstance().SetTotalGameTime(data.gameTime);
					}
					NET_ACTION_HIS* cast = reinterpret_cast<NET_ACTION_HIS*>(bufptr);
					// CRCチェック
					std::uint32_t crc = CRC::Calculate(cast, sizeof(NET_ACTION_HIS), CRC::CRC_32());
					if (crc == data.crc)
					{
						ReplaceActionHis(*cast);
					}
				}
				break;
			}
			}

		}

	}

}

NET_BASIC_DATA NetManager::GetBasicData(const char* bufptr)
{
	NET_BASIC_DATA ret;
	int size;

	// 通信種別
	size = sizeof(ret.type);
	unsigned char* pType = reinterpret_cast<unsigned char*>(&ret.type);
	memcpy(pType, bufptr, size);
	bufptr += size;

	// ユーザキー
	size = sizeof(ret.key);
	unsigned char* pKey = reinterpret_cast<unsigned char*>(&ret.key);
	memcpy(pKey, bufptr, size);
	bufptr += size;

	// ゲーム時間
	size = sizeof(ret.gameTime);
	unsigned char* pTime = reinterpret_cast<unsigned char*>(&ret.gameTime);
	memcpy(pTime, bufptr, size);
	bufptr += size;

	// CRC
	size = sizeof(ret.crc);
	unsigned char* pCRC = reinterpret_cast<unsigned char*>(&ret.crc);
	memcpy(pCRC, bufptr, size);
	bufptr += size;

	return ret;
}

void NetManager::NextFrame(void)
{
	frameNo_++;
}

const NET_JOIN_USER NetManager::GetSelf(void) const
{
	return pool_.selfJoinUser_;
}

const std::map<int, NET_JOIN_USER> NetManager::GetNetUsers(void) const
{
	return pool_.joinUsers_;
}
const NET_JOIN_USER NetManager::GetHost(void) const
{
	return pool_.hostJoinUser_;
}

//送信処理
void NetManager::ReplaceUser(NET_JOIN_USER entity)
{
	auto& users = poolShare_.joinUsers_;

	//
	if (users.find(entity.key) == users.end())
	{
		// 新規追加
		size_t size = users.size();
		entity.playerType = static_cast<PLAYER_TYPE>(size);
		users.emplace(entity.key, entity);
	}
	else
	{

		auto& current = users[entity.key];


		//ここHostしか通ってない気がする
#ifdef _DEBUG
		/*printfDx("[受信] key: %d | 旧位置: %.2f, %.2f → 新位置: %.2f, %.2f\n",
			entity.key,
			poolShare_.joinUserActionHis_[entity.key].selfPostion_.x, poolShare_.joinUserActionHis_[entity.key].selfPostion_.y,
			pool_.joinUserActionHis_[entity.key].selfPostion_.x, pool_.joinUserActionHis_[entity.key].selfPostion_.y);*/
#endif // DEBUG

		if (current.gameState <= entity.gameState)
		{
			// プレイヤータイプはホスト優先
			entity.playerType = current.playerType;

			// その他のデータも更新
			current.gameState = entity.gameState;
			current.ip = entity.ip;
			current.port = entity.port;
			current.mode = entity.mode;
			// 必要なら他の項目も
			current.weaponId_ = entity.weaponId_;
		}

	}
}
void NetManager::ReplaceUsers(NET_JOIN_USERS entities)
{

	poolShare_.joinUsers_.clear();
	
	for (const auto& user : entities.users)
	{

		if (user.mode == NET_MODE::HOST
			|| user.mode == NET_MODE::CLIENT)
		{
			poolShare_.joinUsers_.emplace(user.key, user);
		}

		if (user.key == poolShare_.selfJoinUser_.key)
		{
			poolShare_.selfJoinUser_ = user;
		}

		if (user.mode == NET_MODE::HOST)
		{
			poolShare_.hostJoinUser_ = user;
		}

	}

}

IPDATA NetManager::GetHostIp(void) const
{
	return hostIp_;
}

void NetManager::SetHostIp(IPDATA ip)
{
	hostIp_ = ip;
}

int NetManager::GetHostPort(void) const
{
	return RECEIVE_PORT_HOST;
}

GAME_STATE NetManager::GetGameStateHost(void) const
{
	return pool_.hostJoinUser_.gameState;
}

GAME_STATE NetManager::GetGameStateSelf(void) const
{
	return pool_.selfJoinUser_.gameState;
}
// 接続ユーザーのStateがあっているのか
bool NetManager::IsSameGameState(GAME_STATE state)
{
	// 一人だけなら常に一致とみなす
	/*if (pool_.joinUsers_.size() <= 1)
	{
		return true;
	}*/

	if (pool_.joinUsers_.size() == 0)
	{
		return false;
	}

	for (auto& user : pool_.joinUsers_)
	{
		if (user.second.gameState != state)
		{
			return false;
		}
	}

	return true;

}

void NetManager::ResetAction(void)
{
	pool_.selfAction_.actBits = 0;
}

unsigned int NetManager::GetFrameNo(void) const
{
	return frameNo_;
}

// ゲームシーンでの送信データ作成に利用
void NetManager::MakeActionHis(NET_ACTION action)
{

	bool isExists = false;
	for (int i = 0; i < NUM_FRAME; i++)
	{
		if (action.frameNo == pool_.selfActionHis_.actions[i].frameNo)
		{
			return;
		}
	}

	unsigned int min = 4294967295;
	int idx = -1;
	for (int i = 0; i < NUM_FRAME; i++)
	{
		if (min > pool_.selfActionHis_.actions[i].frameNo)
		{
			min = pool_.selfActionHis_.actions[i].frameNo;
			idx = i;
		}
	}

	pool_.selfActionHis_.actions[idx] = action;

}

bool NetManager::IsSync(void)
{
	return isSync_;
}

// 通信をやめてタイトルへ
void NetManager::ResetSync(void)
{
	isSync_ = false; // 同期解除 or 状態遷移など
	NetManager::GetInstance().Stop();
	SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);

}

void NetManager::SetAction(PLAYER_ACTION act)
{
	pool_.selfAction_.actBits |= static_cast<int>(act);
}

// 位置情報　新規
VECTOR NetManager::GetPostion(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].selfPostion_;
	}
	return{ 100.0f,100.0f,100.0f };
}
void NetManager::SetPostion(int key,VECTOR pos)
{
	pool_.selfActionHis_.selfPostion_ = pos;
}
// 回転情報
Quaternion NetManager::GetPlayRot(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].playerRot_;
	}
	return{ 100.0f,100.0f,100.0f ,100.0f };
}
void NetManager::SetPlayRot(int key, Quaternion rot)
{
	pool_.selfActionHis_.playerRot_ = rot;
}

int NetManager::GetAnimeType(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].playerAnim_;
	}
	return pool_.joinUserActionHis_[pool_.selfJoinUser_.key].playerAnim_;;
}
void NetManager::SetAnimeType(int key, int anim)
{
	pool_.selfActionHis_.playerAnim_ = anim;
}

int NetManager::GetNetHp(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].hp_;
	}
	return pool_.joinUserActionHis_[pool_.selfJoinUser_.key].hp_;
}
void NetManager::SetNetHp(int key, int hp)
{
	pool_.selfActionHis_.hp_ = hp;
}

int NetManager::GetNetBossDamage(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].bossDamage_;
	}
	return 0;
}
void NetManager::SetNetBossDamage(int key, int damage)
{
	pool_.selfActionHis_.bossDamage_ = damage;
}

int NetManager::GetNetBossHp(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].bossHp_;
	}
	return 0;
}

void NetManager::SetNetBossHp(int key, int hp)
{
	pool_.selfActionHis_.bossHp_ = hp;
}

// 武器IDの取得、設定
int NetManager::GetWeapon(int key)
{
	if (pool_.joinUsers_.find(key) != pool_.joinUsers_.end())
	{
		return pool_.joinUsers_[key].weaponId_;
	}
	return 0;
}
void NetManager::SetWeapon(int key, int id)////なんでこれでいけるか理解する/////
{
	// 自分自身の武器IDも変更する場合は selfJoinUser_ も
	if (pool_.selfJoinUser_.key == key)
	{
		pool_.selfJoinUser_.weaponId_ = id;
	}
	// 参加ユーザー一覧の武器IDを更新
	if (pool_.joinUsers_.find(key) != pool_.joinUsers_.end())
	{
		pool_.joinUsers_[key].weaponId_ = id;
	}
	else
	{
		// もし key が存在しなければ新規登録も検討
	}

	// 共有データにも反映するなら、lock_guard で保護しつつ
	{
		std::lock_guard<std::mutex> lock(poolMutex_);
		if (poolShare_.joinUsers_.find(key) != poolShare_.joinUsers_.end())
		{
			poolShare_.joinUsers_[key].weaponId_ = id;
		}
		if (poolShare_.selfJoinUser_.key == key)
		{
			poolShare_.selfJoinUser_.weaponId_ = id;
		}
	}
}



bool NetManager::IsSameFrameNo(void)
{

	bool ret = false;

	if (pool_.selfJoinUser_.gameState != GAME_STATE::GAME_PLAYING)
	{
		return true;
	}


	// プレイヤーが自分しかいない場合は即 true
	if (pool_.joinUsers_.size() <= 1)
	{
		return true;
	}


	if (pool_.joinUserActionHis_.size() != pool_.joinUsers_.size())
	{
		return false;
	}

	unsigned int frameNo = pool_.selfAction_.frameNo;
	if (frameNo == 0)
	{
		return false;
	}

	int cnt = 0;
	for (const auto& his : pool_.joinUserActionHis_)
	{
		for (const auto& act : his.second.actions)
		{
			if (frameNo == act.frameNo)
			{
				cnt += 1;
			}
		}
	}

	if (cnt == pool_.joinUsers_.size())
	{
		ret = true;
	}

	return ret;


}

const NET_ACTION_HIS NetManager::GetSelfActionHis(void) const
{
	return pool_.selfActionHis_;
}

#pragma region ボスの通信関数

MONSTER_DATA NetManager::GetBoss(int key)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())// アクションが最後のと違うなら？
	{
		return pool_.joinUserActionHis_[key].boss_;
	}
	return pool_.joinUserActionHis_[pool_.selfJoinUser_.key].boss_;
}
void NetManager::SetBoss(int key, VECTOR pos, Quaternion rot, int anim, int state)
{
	pool_.selfActionHis_.boss_.postion_ = pos;
	pool_.selfActionHis_.boss_.rot_ = rot;
	pool_.selfActionHis_.boss_.Anim_ = anim;
	pool_.selfActionHis_.boss_.state_ = state;
}

#pragma region 小型用の通信関数

MONSTER_DATA NetManager::GetMonsData(int key, int No)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].monsters_[No];
	}
	return pool_.joinUserActionHis_[pool_.selfJoinUser_.key].monsters_[No];
}
void NetManager::SetMonsData(int key, int No, VECTOR pos, Quaternion rot, int anim, int state)
{
	pool_.selfActionHis_.monsters_[No].postion_ = pos;
	pool_.selfActionHis_.monsters_[No].rot_ = rot;
	pool_.selfActionHis_.monsters_[No].Anim_ = anim;
	pool_.selfActionHis_.monsters_[No].state_ = state;
}

int NetManager::GetNetMonsDamage(int key, int No)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].smallDamage_[No];
	}
	return 0;
}
void NetManager::SetNetMonsDamage(int key, int No, int damage)
{
	pool_.selfActionHis_.smallDamage_[No] = damage;
}

int NetManager::GetNetMonsHp(int key, int No)
{
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		return pool_.joinUserActionHis_[key].smallHp_[No];
	}
	return 0;
}
void NetManager::SetNetMonsHp(int key, int No, int hp)
{
	pool_.selfActionHis_.smallHp_[No] = hp;
}

#pragma endregion

#pragma endregion



const NET_ACTION NetManager::GetSelfAction(void) const
{
	return pool_.selfAction_;
}
const std::map<int, NET_ACTION_HIS> NetManager::GetActionHis(void) const
{
	return pool_.joinUserActionHis_;
}


void NetManager::ReplaceActionHis(NET_ACTION_HIS entity)
{
	auto& map = poolShare_.joinUserActionHis_;
	if (map.find(entity.key) == map.end())
	{
		map.emplace(entity.key, entity);
	}
	else
	{
		map[entity.key] = entity;
	}
}


#pragma region IsAction

// アクションしてるのが自分ではないので他の人を調べる
bool NetManager::IsAction(int key, PLAYER_ACTION action)
{
	// joinUserActionHis_=接続されているほかのユーザー
	if (pool_.joinUserActionHis_.find(key) != pool_.joinUserActionHis_.end())
	{
		for (const auto& actionHis : pool_.joinUserActionHis_[key].actions)
		{
			if (frameNo_ == actionHis.frameNo)
			{
				return actionHis.actBits & static_cast<int>(action);
			}
		}		
	}
	return false;
}

// 再帰処理？Key入力がネットマネージャーで確認するまで回してる？
bool NetManager::IsAction(int key, PLAYER_ACTION action, bool isAction)
{
	// selfJoinUser_ = 自身
	if (pool_.selfJoinUser_.key == key)// 該当するキーを自分が押してるなら
	{
		return isAction;// 処理をそのまま通す　操作をややこしくしない
	}
	return IsAction(key, action);
}

#pragma endregion

