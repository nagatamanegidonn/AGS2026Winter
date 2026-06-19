#pragma once
#include "../../Common/Quaternion.h"
#include "Object.h"
#include "Stage.h"

class ItemObject :
	public Object
{

public:

	// コンストラクタ
	ItemObject(
		Player& player, const Transform& transform, STATE state,int itemId);

	// デストラクタ
	~ItemObject(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

private:

	// アイテムID
	int itemId_;

};

