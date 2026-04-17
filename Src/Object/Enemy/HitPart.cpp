#include "HitPart.h"
#include "../../Utility/AsoUtility.h"


HitPart::HitPart(int& model, std::wstring boneName, float rad, float rate) 
	: 
	parModel_(model),
	boneName_(boneName),
	radius_(rad),
	damageRate_(rate)
{
	pos_ = AsoUtility::MV1GetFreamPos(parModel_, boneName_);
}

HitPart::~HitPart(void)
{
}

void HitPart::Update(void)
{
	pos_ = AsoUtility::MV1GetFreamPos(parModel_, boneName_);

}

void HitPart::Draw(void) const
{
	DrawSphere3D(pos_, radius_, 20, 0xffffff, 0xffffff, false);
}
