#include "BomObject.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Collider/Capsule.h"

BomObject::BomObject(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key)
	:ShotBase(damage, birthPos, shotVec, key)
{
	// モデル制御の基本情報
	transform_.modelId =
		ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::BOM);
	float scale = 1.0f;
	transform_.scl = { scale, scale, scale };
	transform_.Update();

	// カプセルコライダの作成
	capsule_ = nullptr;
	/*capsule_ = std::make_shared<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 0.0f, 10.0f });
	capsule_->SetLocalPosDown({ 0.0f,0.0f,  -10.0f });*/
	radius_ = 30.0f;
	//capsule_->SetRadius(radius_);

	type_ = TYPE::BOM;
}

BomObject::~BomObject(void)
{
}

void BomObject::Update(void)
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

void BomObject::SetParam(void)
{
	speed_ = 15.0f;
	// 生存時間
	// 生存フラグ、時間の初期化
	timeAlive_ = 0.5f;

	radius_ = 30.0f;
}

void BomObject::UpdateShot(void)
{
	// 生存チェック
	CheckAlive();
	if (state_ != STATE::SHOT)
	{
		// 処理中断
		return;
	}

	// 多少下に飛ばす
	//if (shotVec_.y > -1.0f)shotVec_.y += -0.02f;

	// 移動処理
	/*VECTOR velocity = VScale(shotVec_, speed_);
	transform_.pos = VAdd(transform_.pos, velocity);*/
}
void BomObject::UpdateBlast(void)
{
	radius_ += 100.0f;
	if (radius_ >= 1000.0f)ChangeState(STATE::END);
}