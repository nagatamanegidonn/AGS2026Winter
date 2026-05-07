#pragma once
#include <map>
#include <vector>
#include <queue>
#include <mutex>
#include "NetStructures.h"

class NetBase;
class NetSend;

class NetManager
{

public:

	// 最大受信回数
	static constexpr int MAX_RECEIVE_NUM = 10;

	// 受信待機時間(ミリ秒)
	static constexpr float RECEIVE_TERM_MSEC = 1.0f / 120.0f * 1000.0f;

	// 同期待機時間(ミリ秒)
	static constexpr float SYNC_TERM_MSEC = 1.0f / 120.0f * 1000.0f;

	// 明示的にインスタンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static NetManager& GetInstance(void);

	// インスタンスを破棄する
	static void Destroy(void);

	//通信内容
	struct NET_POOL
	{

		// ホストのネットユーザ情報
		NET_JOIN_USER hostJoinUser_;
		// 自分自身のネットユーザ情報
		NET_JOIN_USER selfJoinUser_;

		// 自分自身のアクション情報
		NET_ACTION selfAction_;
		// 自分自身のアクション情報履歴
		NET_ACTION_HIS selfActionHis_;


		// 接続相手(自分自身も含む)
		std::map<int, NET_JOIN_USER> joinUsers_;
		// 接続相手(自分自身も含む)のアクション情報履歴
		std::map<int, NET_ACTION_HIS> joinUserActionHis_;

		// 接続相手(自分自身も含む)の位置情報履歴

	};

	// モード取得
	NET_MODE GetMode(void) const;

	// ホストモードか判断
	bool IsHost(void) const;

	// 通信開始
	void Run(NET_MODE mode);

	// 更新
	void Init(void);
	void Update(void);
	void UpdateState(void);
	void DataReflection(void);
	void UpdateEndOfFrame(void);

	// 通信終了
	void Stop(void);

	// 通信を送る
	void Send(NET_DATA_TYPE type);

	// 状態遷移
	void ChangeGameState(GAME_STATE state);

	// 次のフレームへ
	void NextFrame(void);

#pragma region 他スレッドから読み書きしないこと

	// ホストのIPアドレス取得
	IPDATA GetHostIp(void) const;

	// ホストのIPアドレス設定
	void SetHostIp(IPDATA ip);

#pragma endregion

#pragma region 専用プール

	// 自分のネット情報を取得
	const NET_JOIN_USER GetSelf(void) const;

	// 接続相手を取得
	const std::map<int, NET_JOIN_USER> GetNetUsers(void) const;
	const NET_JOIN_USER GetHost(void) const;

	// ホストのゲーム進行状況
	GAME_STATE GetGameStateHost(void) const;

	// 自分自身のゲーム進行状況
	GAME_STATE GetGameStateSelf(void) const;

	// 全ての接続相手が指定されたゲーム進行状態か判断
	bool IsSameGameState(GAME_STATE state);

	// 自分のアクション情報を取得
	const NET_ACTION GetSelfAction(void) const;
	// 接続相手のアクション情報を取得
	const std::map<int, NET_ACTION_HIS> GetActionHis(void) const;

#pragma region アクションコマンド

	// アクション判定
	bool IsAction(int key, PLAYER_ACTION action);
	// アクション判定
	bool IsAction(int key, PLAYER_ACTION action, bool isAction);

	// アクション設定
	void SetAction(PLAYER_ACTION act);

#pragma endregion

	//新規
#pragma region ゲームシーンで使用

	// bool IsAction(int key, PLAYER_ACTION action, bool isAction);
	// ポジション設定
	VECTOR GetPostion(int key);
	void SetPostion(int key, VECTOR pos);
	//回転
	Quaternion GetPlayRot(int key);
	void SetPlayRot(int key, Quaternion rot);
	//アニメーション
	int GetAnimeType(int key);
	void SetAnimeType(int key, int anim);

	int GetNetHp(int key);
	void SetNetHp(int key, int hp);

	//ボス関係
	int GetNetBossDamage(int key);
	void SetNetBossDamage(int key, int damage);

	int GetNetBossHp(int key);
	void SetNetBossHp(int key, int hp);

#pragma endregion

	int GetWeapon(int key);
	void SetWeapon(int key, int id);



	// 全ての接続相手が指定されたゲームカウントか判断
	bool IsSameFrameNo(void);

	// 自分のアクション情報履歴
	const NET_ACTION_HIS GetSelfActionHis(void) const;

	// ポジション設定
	MONSTER_DATA GetBoss(int key);
	void SetBoss(int key, VECTOR pos, Quaternion rot, int anim, int  state);
	
	// ポジション設定
	MONSTER_DATA GetMonsData(int key,int No);
	void SetMonsData(int key, int No, VECTOR pos, Quaternion rot, int anim, int  state);
	int GetNetMonsDamage(int key, int No);
	void SetNetMonsDamage(int key, int No, int damage);
	int GetNetMonsHp(int key, int No);
	void SetNetMonsHp(int key, int No, int hp);


#pragma endregion

	// ホストのポート番号取得
	int GetHostPort(void) const;

	// アクション設定
	void ResetAction(void);

	unsigned int GetFrameNo(void) const;

	void MakeActionHis(NET_ACTION action);
	
	bool IsSync(void);

	void ResetSync(void);

private:

	const int RECEIVE_PORT_HOST			= 65000;
	const int RECEIVE_PORT_CLIENT_RANGE	= RECEIVE_PORT_HOST + 100;

	unsigned int timeoutCounter_ = 0;
	const unsigned int TIMEOUT_LIMIT = 3000; // 単位はミリ秒

	// 静的インスタンス
	static NetManager* instance_;

	// ホスト or クライアント
	NetBase* net_;

	// 送信用クラス
	NetSend* netSend_;

	// 通信開始フラグ
	bool isRunning_;

	// ソケットハンドル
	int sendSocketId_;
	int recvSocketId_;

	// スレッド
	std::thread recvThread_;

	// 送受信データの排他制御
	std::mutex poolMutex_;

	// 進行フレーム番号
	unsigned int frameNo_;

#pragma region 他スレッドから読み書きしないこと

	// 役割(ホスト、クライアント)
	NET_MODE mode_;

	// ホストのIPアドレス
	IPDATA hostIp_ = { 127, 0, 0, 1 };

#pragma endregion

	// 送受信時のデータ同期用
	// 排他制御外では絶対に読み書きしないこと
	NET_POOL poolShare_;

	// 排他制御外で使用
	NET_POOL pool_;

	bool isSync_;

	// コンストラクタ
	NetManager(void);

	// コピーコンストラクタ
	NetManager(const NetManager& ins);

	// デストラクタ
	~NetManager(void);

	// 受信スレッド
	void UdpReceiveThread(void);

	// データ受信処理
	void UdpReceiveData(void);

	// 送受信用の基本データをバッファから取得する
	NET_BASIC_DATA GetBasicData(const char* bufptr);

#pragma region 送受信データ書き込み

	// 接続相手を追加・変更
	void ReplaceUser(NET_JOIN_USER entity);

	// 接続相手を設定
	void ReplaceUsers(NET_JOIN_USERS entities);

	// アクション情報を追加・変更
	void ReplaceActionHis(NET_ACTION_HIS entity);

#pragma endregion


};
