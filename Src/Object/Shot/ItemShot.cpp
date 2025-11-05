#include "ItemShot.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Collider/Capsule.h"

ItemShot::ItemShot(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key)
	:ShotBase(damage, birthPos, shotVec, key)
{
	// モデル制御の基本情報
	transform_.modelId =
		ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::SMALL_BAG);
	float scale = 1.0f;
	transform_.scl = { scale, scale, scale };
	transform_.Update();

	// カプセルコライダの作成
	capsule_ = std::make_shared<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 0.0f, 10.0f });
	capsule_->SetLocalPosDown({ 0.0f,0.0f,  -10.0f });
	capsule_->SetRadius(30.0f);

}

ItemShot::~ItemShot(void)
{
}

void ItemShot::Update(void)
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