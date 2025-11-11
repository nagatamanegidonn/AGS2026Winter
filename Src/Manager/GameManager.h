#pragma once
#include <memory>
#include <DxLib.h>

#include "../Lib/nlohmann/json.hpp"

// 長いのでnamespaceの省略
using json = nlohmann::json;

class GameManager
{
public:

	// ゲーム結果
	enum class GAME_RESULT
	{
		NONE,
		GAME_CLEAR,
		GAME_OVER,
		TIME_OVER,
		DRAW
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static GameManager& GetInstance(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Destroy(void);

	// ゲーム結果の取得
	GAME_RESULT GetGameResult(void);
	// ゲーム結果の設定
	void SetGameResult(GAME_RESULT result);
	
	const int GetControllId(void)const { return ControllerId_; }
	const int GetWeponId(void)const { return weponId_; }
	const void SetWeponId(int weponId) { weponId_ = weponId; }

	//接続モードについて
	const bool IsHost(void)const { return IsHost_; }
	void SetHost(bool value);

private:

	// 静的インスタンス
	static GameManager* instance_;

	// コンストラクタ
	GameManager(void);
	// デストラクタ
	~GameManager(void);


	// ゲーム結果
	GAME_RESULT gameResult_;

	// プレイヤー設定
	int charId_; // キャラクターID
	int weponId_;// 武器ID

	// 接続モード
	bool IsHost_;

	// コントローラーID
	int ControllerId_;

	//コントローラーの読み込み、保存
	void LoadController(void);
	void SaveController(bool _isPlay);

	void Load(void);
	void Save(bool isUse);
};

