#include "BomObject.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Collider/Capsule.h"
#include "../../Application.h"
#include "../Common/EffectController.h" 

namespace {
	// カプセルの初期値
	constexpr VECTOR CAP_LOACL_TOP = { 0.0f, 0.0f, 0.0f };
	constexpr VECTOR CAP_LOACL_DOWN = { 0.0f, 0.0f, 0.0f };
	constexpr float CAP_RADIUS = 60.0f;
}

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
	radius_ = CAP_RADIUS;

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
	// 移動スピード
	speed_ = 15.0f;
	// 生存時間
	// 生存フラグ、時間の初期化
	timeAlive_ = 10.5f;
	// 判定半径
	radius_ = 60.0f;
	// ダメージ
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
}

void BomObject::UpdateBlast(void)
{
	radius_ += 100.0f;
	if (radius_ >= 1000.0f)ChangeState(STATE::END);
}