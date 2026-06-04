#pragma once
#include "SceneBase.h"

class RefreshScene : 
	public SceneBase
{

public:

	// コンストラクタ
	RefreshScene(void);

	// デストラクタ
	~RefreshScene(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

	// 解放処理
	void Release(void) override;

private:

	// ゲーム開始待機時間
	float stepCountDown_;

};

