#include "BomObject.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Collider/Capsule.h"
#include "../../Application.h"
#include "../Common/EffectController.h" // ★この行を追加

BomObject::BomObject(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key)
	:ShotBase(damage, birthPos, shotVec, key)
{
	// モデル制御の基本情報
	transform_.modelId =
		ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::BOM);
	float scale = 1.0f;
	transform_.scl = { scale, scale, scale };
	transform_.pos = VAdd(birthPos, VScale({0.0f,1.0f,0.0f}, 30.0f));
	transform_.Update();

	// カプセルコライダの作成
	capsule_ = nullptr;
	radius_ = 60.0f;

	type_ = TYPE::BOM;

	SetParam();

	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();

	effectController_->Add(0, path + L"Fire/Fire.efkefc");
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

	effectController_->Update(0);

	// モデル制御の基本情報更新
	transform_.Update();
}

void BomObject::SetParam(void)
{
	speed_ = 15.0f;
	// 生存時間
	// 生存フラグ、時間の初期化
	timeAlive_ = 10.5f;

	radius_ = 60.0f;

	damage_ = 100;
}

void BomObject::UpdateShot(void)
{
	// 生存チェック
	CheckAlive();
	// 大きさ変化
	float t = 10.5f - timeAlive_;
	float val = fabs(sin(t * t)) * 0.3f;
	float scale = val + 1.0f;
	transform_.scl = { scale, scale, scale };

	if (timeAlive_ <= 0.0f)
	{
		radius_ = 30.0f;
		effectController_->Play(0, transform_.pos, { 0.0f,0.0f,0.0f }, 100.0f);

	}
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