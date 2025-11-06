#include "ArrowShot.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Collider/Capsule.h"

ArrowShot::ArrowShot(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key)
	:ShotBase(damage, birthPos, shotVec, key)
{
	// モデル制御の基本情報
	transform_.modelId =
		ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::ARROW);
	float scale = 2.0f;
	transform_.scl = { scale, scale, scale };
	transform_.Update();

	// カプセルコライダの作成
	capsule_ = std::make_shared<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 0.0f, 110.0f });
	capsule_->SetLocalPosDown({ 0.0f,0.0f,  -30.0f });
	capsule_->SetRadius(10.0f);


}
ArrowShot::~ArrowShot(void)
{
}


void ArrowShot::Update(void)
{
	switch (state_)
	{
	case ShotBase::STATE::NONE:
		break;
	case ShotBase::STATE::SHOT:
		UpdateShot();
		break;
	case ShotBase::STATE::BLAST:
		UpdateBlast();
		break;
	case ShotBase::STATE::END:
		break;
	}
	// モデル制御の基本情報更新
	transform_.Update();
}

void ArrowShot::SetParam(void)
{
	speed_ = 20.0f;
	// 生存時間
	// 生存フラグ、時間の初期化
	timeAlive_ = 5.0f;
}
