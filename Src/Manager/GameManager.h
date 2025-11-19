#pragma once
#include <memory>
#include <list>
#include <DxLib.h>

#include "../Lib/nlohmann/json.hpp"

// 長いのでnamespaceの省略
using json = nlohmann::json;

class GameManager
{
public:

	// クリアパラメータ
	struct ClearParam
	{
		int type;
		float sValue;
		float eValue;
		float speed;
	};

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
	void ClearDraw(void);
	void Destroy(void);

	// ゲーム結果の取得
	GAME_RESULT GetGameResult(void);
	// ゲーム結果の設定
	void SetGameResult(GAME_RESULT result);
	
	// クエストの初期化
	void InitQuest(int _clearMaxCount) {
		clearMaxCount_ = _clearMaxCount;
		clearCount_ = 0;
		clearFlag_ = false;//いらないかも
		clearTime_ = 0.0f;
	}

	// クリアフラグの取得、設定
	const bool GetClearFlag(void)const { return clearFlag_; }
	void SetClearFlag(bool flag) { clearFlag_ = flag; }

	// クリア時間の更新
	void UpdateClearTime(float _deltaTime) { clearTime_ -= _deltaTime; }
	// クリア時間の取得、設定
	void SetClearTime(float _time) { clearTime_ = _time; }
	const float GetClearTime(void)const { return clearTime_; }

	// クリア回数の取得、設定
	const int GetClearCount(void)const { return clearCount_; }
	void SetClearCount(int _count) { clearCount_ = _count; }
	void PlusClearCount(int _count) { clearCount_ += _count; }
	// クリア判定
	bool IsClear(void)const { return clearCount_ >= clearMaxCount_; }

	// プレイヤー設定について
	const int GetControllId(void)const { return ControllerId_; }
	const int GetWeponId(void)const { return weponId_; }
	const void SetWeponId(int weponId) { weponId_ = weponId; }

	// 接続モードについて
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
	// クリアフラグ
	bool clearFlag_;
	//ゲーム内容
	int clearCount_; 
	int clearMaxCount_;
	float clearTime_;

	
	float paramRate_;
	std::list<ClearParam> clearPramList_;
	ClearParam currentParam_;
	int clearImg_;

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

