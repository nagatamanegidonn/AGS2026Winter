#pragma once
#include<memory>  // shared_ptrを使うために必要
#include <vector>
#include "../Common/Vector2.h"

#include "SceneBase.h"

class Player;
class Boss;
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

	void DownCountPuls(void);
	const int GetDownCount(void)const { return downCnt_; }

	void CreateShot(int damage,const VECTOR birthPos, const VECTOR dir, int key);

private:

	std::unique_ptr<PixelMaterial> Material_;
	std::unique_ptr<PixelRenderer> Renderer_;
	int mapHandle_;
	int bossHandle_;
	int playerHandle_;

	std::unique_ptr<Timer> timer_;

	std::string textId_;
	int stepId_;

	std::vector<std::shared_ptr<Player>> players_;
	std::unique_ptr<Boss> boss_;
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
	int downCnt_;
	float soundRate_;

	// 衝突判定
	void Collision(void);

};

