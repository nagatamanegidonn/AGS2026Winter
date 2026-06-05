#pragma once
#include <memory> 
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

class GameScene : 
	public SceneBase
{

public:

	// ゲームプレイヤー数
	static constexpr int GAME_PLAYER_NUM = 2;

	// プレイヤーダウンカウント限界数
	static constexpr int MAX_DOWN_COUNT = 2;

	// 最小距離の初期値
	static constexpr float START_MIN_DIST = 10000.0f;

	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

	// 解放処理
	void Release(void) override;

	// ダウンカウントの加算
	void DownCountPuls(void) { downCnt_ += 1; }

	// ダウンカウントの取得
	inline const int GetDownCount(void)const { return downCnt_; }

	// 弾の生成
	void CreateShot(ShotBase::TYPE type, int damage, const VECTOR& birthPos, const VECTOR& dir, int key);

	// ステージオブジェクトの生成
	void CreateObject(const Transform& _trans);

private:

	// フェード（フラッシュ用）
	std::unique_ptr<Fader> fader_;

	// タイマー
	std::unique_ptr<Timer> timer_;

	// ボス移動用テキストID
	std::string textId_;

	// ステップID
	int stepId_;

	// Objectクラス
	std::vector<std::shared_ptr<Player>> players_;		// プレイヤー
	std::unique_ptr<Boss> boss_;						// ボス
	std::vector<std::shared_ptr<EnemyBase>> monsters_;	// モンスター
	std::vector<std::unique_ptr<ShotBase>> shots_;		// 投げ物

	// 背景インスタンス	
	std::unique_ptr<SkyDome> skyDome_;	// スカイドーム
	std::unique_ptr<Stage> stage_;		// ステージ
	std::unique_ptr<Grid> grid_;		// グリッド

	// ゲーム開始待機時間
	float stepCountDown_;
	
	// 最小距離
	float minDist_;

	// ゲームシーン用変数
	int downCnt_;		// ダウンした回数
	float soundRate_;	// 音量倍率

	// 衝突判定
	void Collision(void);

	// 投げ物と敵の当たり判定
	void ShotHitEnemy(ShotBase& shot, EnemyBase& enemy);

};