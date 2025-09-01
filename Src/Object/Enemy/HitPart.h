#pragma once
#include <DxLib.h>
#include <string>

class HitPart
{
public:

	HitPart(int& model, std::wstring boneName, float rad, float rate);
	~HitPart(void);

	void Update(void);
	void Draw(void);

	const VECTOR GetPos(void) const { return pos_; }
	const float GetRadius(void) const { return radius_; }
	const float GetDameRate(void) const { return damageRate_; }

private:

	int& parModel_;
	std::wstring boneName_;

	//ダメージ関係
	float damageRate_;

	//位置関係
	VECTOR pos_;
	float radius_;

};

