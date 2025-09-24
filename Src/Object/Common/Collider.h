#pragma once
#include <DxLib.h>
class Collider
{

public :

	// 衝突種別
	enum class TYPE
	{
		STAGE,
		WALL,
		BOSS,
		SPHERE,
		ITEM,
	};

	// コンストラクタ
	Collider(TYPE type, int modelId, VECTOR pos = { 0.0f,0.0f,0.0f }, float rad = 0.0f);

	// デストラクタ
	~Collider(void);

	// 衝突種別
	TYPE type_;

	// モデルのハンドルID
	int modelId_;

	//位置情報
	VECTOR pos_;
	float radius_;
};
