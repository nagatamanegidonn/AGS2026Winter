#pragma once
#include <DxLib.h>
#include <string>

class HitPart
{

public:

	// コンストラクタ
	HitPart(int& model, std::wstring boneName, float rad, float rate);
	
	// デストラクタ
	~HitPart(void);

	// 更新処理
	void Update(void);
	
	// 描画処理
	void Draw(void) const;

	// 位置の取得
	const VECTOR GetPos(void) const { return pos_; }

	// 半径の取得
	const float GetRadius(void) const { return radius_; }

	// ダメージ倍率の取得
	const float GetDameRate(void) const { return damageRate_; }

private:

	// モデルの参照
	int& parModel_;
	
	// 参照ボーンネーム
	std::wstring boneName_;

	// ダメージ関係
	float damageRate_;

	// 位置関係
	VECTOR pos_;

	// 半径
	float radius_;

};

