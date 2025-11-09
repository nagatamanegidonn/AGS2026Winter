#pragma once
#include<memory>  // shared_ptrを使うために必要
#include <vector>
#include "../Common/Vector2.h"
#include "../Object/Common/Transform.h"
#include "../Object/Shot/ShotBase.h"

#include "SceneBase.h"

class Fader;
class Player;
class Boss;
class SmallMonster;
class EnemyBase;
class ShotBase;
class Stage;
class SkyDome;
class Grid;
class Timer;
class PixelMaterial;
class PixelRenderer;

class GameScene : public SceneBase
{

public:

	//ゲームプレイヤー数
	static constexpr int GAME_PLAYER_NUM = 2;

	//プレイヤーダウンカウント限界数
	static constexpr int MAX_DOWN_COUNT = 2;

	//コンストラクタ
	GameScene(void);

	//デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	void DownCountPuls(void) { downCnt_ += 1; }
	const int GetDownCount(void)const { return downCnt_; }

	//弾の生成
	void CreateShot(ShotBase::TYPE type,int damage
		,const VECTOR& birthPos, const VECTOR& dir, int key);

	//ステージオブジェクトの生成
	void CreateObject(const Transform& _trans);

private:

	// フェード
	std::unique_ptr<Fader> fader_;

	//ミニマップシェーダ
	std::unique_ptr<PixelMaterial> Material_;
	std::unique_ptr<PixelRenderer> Renderer_;
	int mapHandle_;
	int bossHandle_;
	int playerHandle_;

	std::unique_ptr<Timer> timer_;

	std::string textId_;
	int stepId_;

	//Objectクラス
	std::vector<std::shared_ptr<Player>> players_;

	std::unique_ptr<Boss> boss_;
	std::unique_ptr<SmallMonster> Monsters_[3];

	std::vector<std::unique_ptr<ShotBase>> shots_;

	//背景インスタンス
	// スカイドーム
	std::unique_ptr<SkyDome> skyDome_;
	std::unique_ptr<Stage> stage_;
	std::unique_ptr<Grid> grid_;

	//ゲーム開始待機時間
	float stepCountDown_;
	
	float minDist_ = 10000.0f;

	//ゲームシーン用変数
	int downCnt_;		//ダウンした回数
	float soundRate_;	//音量倍率

	// 衝突判定
	void Collision(void);

	void ShotHitEnemy(ShotBase& shot, EnemyBase& enemy);

};

