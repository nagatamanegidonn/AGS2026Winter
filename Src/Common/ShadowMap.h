#pragma once

class ShadowMap
{

public:

	// コンストラクタ
	ShadowMap(void);

	// デストラクタ
	~ShadowMap(void);

	// 初期化処理
	void Init(void);

	// 描画処理
	void DrawStart(void);
	void DrawEnd(void);

private:

	int shadowMapHandle_;

};

