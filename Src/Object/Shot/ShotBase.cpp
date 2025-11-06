
#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"

#include "../Common/Collider/Capsule.h"

#include "ShotBase.h"

ShotBase::ShotBase(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key)
{
	Create(damage,birthPos, shotVec, key);
}
ShotBase::~ShotBase(void)
{
	MV1DeleteModel(transform_.modelId);
}


void ShotBase::Create(int damage, const VECTOR& birthPos, const VECTOR& dir, int key)
{
	damage_ = damage;
	key_ = key;

	// モデル制御の基本情報
	// パラメータ設定
	SetParam();
	// 再利用可能なようにする
	// 指定方向に弾を飛ばす
	shotVec_ = dir;
	// 弾の発生位置
	transform_.pos = birthPos;

	// 弾モデルの向き(角度)を指定方向に合わせる
	transform_.quaRot = Quaternion::LookRotation(shotVec_);
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f });

	// モデル制御の基本情報更新
	transform_.Update();

	
	// 状態遷移
	ChangeState(STATE::SHOT);
}

void ShotBase::Init(void)
{
}

void ShotBase::Update(void)
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

void ShotBase::Draw(void)
{
	// カメラクリップ外になったら描画しない
	if (!CheckCameraViewClip(transform_.pos))
	{
		switch (state_)
		{
		case ShotBase::STATE::NONE:
			break;
		case ShotBase::STATE::SHOT:
			MV1DrawModel(transform_.modelId);
			
			break;
		case ShotBase::STATE::BLAST:
			MV1DrawModel(transform_.modelId);
			break;
		case ShotBase::STATE::END:
			break;
		}
	}
#ifdef _DEBUG
	capsule_->Draw();
#endif // _DEBUG

}

int ShotBase::GetDamage(void) const
{
	return (int)((float)damage_ * (timeAlive_ / 2));
}

std::weak_ptr<Capsule> ShotBase::GetCapsule(void)
{
	return capsule_;
}

void ShotBase::Destroy(void)
{
	ChangeState(STATE::END);
}


void ShotBase::SetParam(void)
{
	speed_ = 20.0f;
	// 生存時間
	// 生存フラグ、時間の初期化
	timeAlive_ = 5.0f;
}
void ShotBase::CheckAlive(void)
{
	// 生存時間を減らす
	timeAlive_ -= SceneManager::GetInstance().GetDeltaTime();
	if (timeAlive_ < 0.0f)
	{
		ChangeState(STATE::BLAST);
	}
}

void ShotBase::ChangeState(STATE state)
{
	if (state_ == state)
	{
		// 同じ状態なら処理しない
		return;
	}
	// 状態更新
	state_ = state;
	
}

void ShotBase::UpdateShot(void)
{
	// 生存チェック
	CheckAlive();
	if (state_ != STATE::SHOT)
	{
		// 処理中断
		return;
	}
	// 移動処理
	VECTOR velocity = VScale(shotVec_, speed_);
	transform_.pos = VAdd(transform_.pos, velocity);
}
void ShotBase::UpdateBlast(void)
{
	ChangeState(STATE::END);
}