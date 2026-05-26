#include <DxLib.h>
#include "../Transform.h"
#include "Capsule.h"

Capsule::Capsule(const Transform& parent) 
	: 
	transformParent_(parent),
	localPosTop_({ 0.0f, 0.0f, 0.0f }),
	localPosDown_({ 0.0f, 0.0f, 0.0f }),
	radius_(0.0f)

{
}

Capsule::Capsule(const Capsule& base, const Transform& parent) 
	: 
	transformParent_(parent)
{
	radius_ = base.GetRadius();
	localPosTop_ = base.GetLocalPosTop();
	localPosDown_ = base.GetLocalPosDown();
}

Capsule::~Capsule(void)
{
}

void Capsule::Draw(void)
{

	// 上の球体
	VECTOR pos1 = GetPosTop();
	DrawSphere3D(pos1, radius_, 5, COLOR, COLOR, false);

	// 下の球体
	VECTOR pos2 = GetPosDown();
	DrawSphere3D(pos2, radius_, 5, COLOR, COLOR, false);

	VECTOR dir;
	VECTOR s;
	VECTOR e;

	// 球体を繋ぐ線(X+)
	dir = transformParent_.GetRight();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	// 球体を繋ぐ線(X-)
	dir = transformParent_.GetLeft();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	// 球体を繋ぐ線(Z+)
	dir = transformParent_.GetForward();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	// 球体を繋ぐ線(Z-)
	dir = transformParent_.GetBack();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, COLOR);

	// カプセルの中心
	DrawSphere3D(GetCenter(), 5.0f, 10, COLOR, COLOR, true);

}

VECTOR Capsule::GetLocalPosTop(void) const
{
	return localPosTop_;
}
VECTOR Capsule::GetLocalPosDown(void) const
{
	return localPosDown_;
}

void Capsule::SetLocalPosTop(const VECTOR& pos)
{
	localPosTop_ = pos;
}
void Capsule::SetLocalPosDown(const VECTOR& pos)
{
	localPosDown_ = pos;
}

VECTOR Capsule::GetPosTop(void) const
{
	return GetRotPos(localPosTop_);
}
VECTOR Capsule::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}

VECTOR Capsule::GetRotPos(const VECTOR& localPos) const
{
	VECTOR localRotPos = transformParent_.quaRot.PosAxis(localPos);
	return VAdd(transformParent_.pos, localRotPos);
}

float Capsule::GetRadius(void) const
{
	return radius_;
}

void Capsule::SetRadius(float radius)
{
	radius_ = radius;
}

float Capsule::GetHeight(void) const
{
	return localPosTop_.y;
}

VECTOR Capsule::GetCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();

	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}

bool Capsule::IsHitSphere(const VECTOR _centerPos, float _radius)
{
	//球体とカプセルの当たり判定
	bool ret = false;

	// カプセル球体の中心を繋ぐベクトル
	VECTOR cap1to2 = VSub(GetPosDown(), GetPosTop());

	// ベクトルを正規化
	VECTOR cap1to2ENor = VNorm(cap1to2);

	// カプセル繋ぎの単位ベクトルと、
	// そのベクトル元から球体へのベクトルの内積を取る
	float dot = VDot(cap1to2ENor, VSub(_centerPos, GetPosTop()));

	// 内積で求めた射影距離を使って、カプセル繋ぎ上の座標を取る
	VECTOR capRidePos = VAdd(GetPosTop(), VScale(cap1to2ENor, dot));

	// カプセル繋ぎのベクトルの長さを取る
	float len = sqrt((cap1to2.x * cap1to2.x) + (cap1to2.y * cap1to2.y) + (cap1to2.z * cap1to2.z));

	// 球体がカプセル繋ぎ上にいるか判別するため、比率を取る
	float rate = dot / len;

	VECTOR centerPos = { 0.0f,0.0f,0.0f };

	// 球体の位置が３エリアに分割されたカプセル形状のどこにいるか判別
	if (rate > 0.0f && rate <= 1.0f)
	{
		// ①球体がカプセル繋ぎ上にいる
		centerPos = VAdd(GetPosTop(), VScale(cap1to2ENor, dot));
	}
	else if (rate > 1.0f)
	{
		// ②球体がカプセルの終点側にいる
		centerPos = GetPosDown();
	}
	else if (rate < 0.0f)
	{
		// ③球体がカプセルの始点側にいる
		centerPos = GetPosTop();
	}
	else
	{
		// ここにきてはいけない
		return false;
	}

	// お互いの半径の合計
	float radius = _radius + GetRadius();

	// 座標の差からお互いの距離を取る
	VECTOR diff = VSub(centerPos, _centerPos);

	// 三平方の定理で比較(SqrMagnitudeと同じ)
	float dis = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);
	if (dis < (_radius * _radius))
	{
		ret = true;
	}

	return ret;
}
