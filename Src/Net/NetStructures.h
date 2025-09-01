#pragma once
#include <DxLib.h>
#include "../Object/Common/Transform.h"


static constexpr int MAX_PLAYERS = 4;
static constexpr int MAX_SEND_BYTES = 1024;
static constexpr int NUM_FRAME = 10;

// 通信モード
enum class NET_MODE
{
	NONE,
	HOST,
	CLIENT,
};

// ゲーム進行状態
enum class GAME_STATE
{
	NONE,
	CONNECTING,
	GOTO_GAME,
	GAME_PLAYING,
};

// ゲームプレイヤー種別
enum class PLAYER_TYPE
{
	PLAYER_1,
	PLAYER_2,
	PLAYER_3,
	PLAYER_4,
};

// 通信種別
enum class NET_DATA_TYPE
{
	NONE,
	USER,
	USERS,
	ACTION_HIS_ALL,
};

// 基本通信データ//通信の基本的なデータ
struct NET_BASIC_DATA
{
	// 通信種別
	NET_DATA_TYPE type = NET_DATA_TYPE::NONE;
	// ユーザの任意キー
	int key = -1;
	// ゲームの進行時間
	float gameTime = 0.0f;
	uint32_t crc;
};

// ユーザデータ//接続の参照などに使用する,接続ユーザーデータ
struct NET_JOIN_USER
{
	// 通信モード(ホスト or クライアント)
	NET_MODE mode = NET_MODE::NONE;
	// ユーザの任意キー
	int key = -1;
	// ユーザのIPアドレス
	IPDATA ip = { 0, 0, 0, 0 };
	// ユーザのポート
	int port = -1;
	// ユーザ1、2、3、4...
	PLAYER_TYPE playerType = PLAYER_TYPE::PLAYER_1;
	// 全体のゲーム進行状態
	GAME_STATE gameState = GAME_STATE::CONNECTING;

	//武器情報
	int weaponId_ = 0;

};

struct NET_JOIN_USERS
{
	NET_JOIN_USER users[MAX_PLAYERS];
};

//ネットにおくるクラス？
enum class PLAYER_ACTION
{
	NONE = 0,
	BOSS_ATTRCK_A = 1,
	BOSS_ATTRCK_B = 2,
	JUMP = 4,
	JUMP_TRG = 8,
	JUMP_CANCEL = 16,
	SHOT = 32,
	IS_CLEAR = 64,
	IS_BATTLE = 128,
	IS_ATTRCK = 256,
};

struct NET_ACTION
{
	// ユーザの任意キー
	int key = -1;
	// ゲーム進行フレーム
	unsigned int frameNo = 0;
	// アクションキー
	unsigned int actBits;
};

struct BOSS_DATA
{
	VECTOR bossPostion_ = { 0.0f,0.0f,0.0f };
	Quaternion bossRot_ = { 0.0f,0.0f,0.0f,0.0f };
	int bossAnim_ = 0;
	int bossState_ = 0;
};

//ゲームシーンで使用する通信データ
struct NET_ACTION_HIS
{
	// ユーザの任意キー
	int key = -1;
	// 過去フレーム分のアクション情報//bool型みたいなもの
	NET_ACTION actions[NUM_FRAME];

	//自分の位置情報		新規
	VECTOR selfPostion_ = { 0.0f,0.0f,0.0f };
	Quaternion playerRot_ = { 0.0f,0.0f,0.0f,0.0f };

	int playerAnim_ = 0;

	int hp_ = 100;
	int bossDamage_ = 0;//Bossに与えたdamage毎フレームリセット
	int bossHp_ = 0;//Boss


	VECTOR bossPostion_ = { 0.0f,0.0f,0.0f };
	Quaternion bossRot_ = { 0.0f,0.0f,0.0f,0.0f };
	int bossAnim_ = 0;

	BOSS_DATA boss_;
};

struct NET_USERS_ACTION_HIS
{
	// 全員分の過去フレーム分のアクション情報
	NET_ACTION_HIS actionsHistories[MAX_PLAYERS];
};

struct NET_USERS_POS//新規
{
	// 全員分の過去フレーム分のアクション情報
	VECTOR userPos;
};
