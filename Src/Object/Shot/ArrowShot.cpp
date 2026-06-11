#include "ArrowShot.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Collider/Capsule.h"

namespace {
	// カプセルの初期値
	constexpr VECTOR CAP_LOACL_TOP = { 0.0f, 0.0f, 110.0f };
	constexpr VECTOR CAP_LOACL_DOWN = { 0.0f, 0.0f, -30.0f };
	constexpr float CAP_RADIUS = 10.0f;
}

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
	capsule_->SetLocalPosTop(CAP_LOACL_TOP);
	capsule_->SetLocalPosDown(CAP_LOACL_DOWN);
	radius_ = CAP_RADIUS;
	capsule_->SetRadius(radius_);

	type_ = TYPE::ARROW;
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

int ArrowShot::GetDamage(void) const
{
	return static_cast<int>(((float)damage_) * (timeAlive_ / 2));
}

void ArrowShot::SetParam(void)
{
	// 移動スピード
	speed_ = 20.0f;
	// 生存時間
	// 生存フラグ、時間の初期化
	timeAlive_ = 5.0f;
}
