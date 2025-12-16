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

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	int itemId_;
};

