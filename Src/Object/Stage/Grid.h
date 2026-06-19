#pragma once

#include "../Common/Transform.h"

class SpaceDome;

class Grid
{

public:

#pragma region デバッグ用定数

	// 線の長さ
	static constexpr float LEN = 1200.0f;
	// 線の長さの半分
	static constexpr float HLEN = LEN / 2.0f;
	// 線の間隔
	static constexpr float TERM = 50.0f;
	// 線の数
	static constexpr int NUM = static_cast<int>(LEN / TERM);
	// 線の数の半分
	static constexpr int HNUM = NUM / 2;

#pragma endregion

	// 大きさ
	static constexpr float BACKGROUND_SCALE = 7.5f;
	// Y座標初期値
	static constexpr float BACKGROUND_POS_Y = -100.0f;
	// 回転X
	static constexpr float BACKGROUND_ANGLE_X = 180.0f;
	// 回転Y
	static constexpr float BACKGROUND_ANGLE_Y = 90.0f;
	// ステージの半径
	static constexpr float STAGE_RADIUS = 800.0f;

	// コンストラクタ
	Grid(void);
	// デストラクタ
	~Grid(void);

	// 初期化処理
	void Init(void);
	
	// 更新処理
	void Update(void);
	
	// 描画処理
	void Draw(void);
	void DrawDebug(void);// デバッグ描画
	
	// 解放処理
	void Release(void);

};