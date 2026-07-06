#pragma once
#include "Common/Transform.h"

class ActorBase
{

public:

	// コンストラクタ
	ActorBase(void);

	// デストラクタ
	virtual ~ActorBase(void);

	// 初期化処理
	virtual void Init(void) = 0;
	
	// 更新処理
	virtual void Update(void) = 0;
	
	// 描画処理
	virtual void Draw(void) = 0;

	// 影の描画
	virtual void DrawShadow(void);

	// モデル基本情報の取得
	const Transform& GetTransform(void) const;

protected:

	// モデル制御の基本情報
	Transform transform_;

};
