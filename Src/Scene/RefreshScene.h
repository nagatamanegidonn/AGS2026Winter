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

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:

	// ゲーム開始待機時間
	float stepCountDown_;

};

