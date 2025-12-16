#include "CharaBase.h"
#include "../Manager/SceneManager.h"
#include "../Utility/AsoUtility.h"
#include "Stage/Planet.h"

CharaBase::CharaBase(void)
	:
	areaId_(-1),
	attrckDamage_(10),
	isHitCheck_(false),
	changeAttrckTime_(0.0f),
	speed_(0.0f),
	stepRotTime_(0.0f),
	goalQuaRot_(),
	jumpPow_({ 0.0f,0.0f,0.0f }),
	moveDir_({ 0.0f,0.0f,0.0f }),
	movePow_({ 0.0f,0.0f,0.0f }),
	movedPos_({ 0.0f,0.0f,0.0f }),
	prePos_({ 0.0f,0.0f,0.0f }),
	attrckRate_(1.0f),
	ActorBase()
{
}

CharaBase::~CharaBase(void)
{
}

void CharaBase::Collision(void)
{
	prePos_ = transform_.pos;

	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionStageCapsule();

	// 衝突(重力)
	CollisionGravity();

	// 移動
	transform_.pos = movedPos_;


	if (transform_.pos.y <= -1000.0f)
	{
		transform_.pos = { 0.0f, -30.0f, 0.0f };
	}

	// 移動誤判定
	CollisionMoveEnd();
}
void CharaBase::CollisionStageCapsule(void)
{
}
void CharaBase::CollisionGravity(void)
{
}
void CharaBase::CollisionMoveEnd(void)
{
	for (const auto c : colliders_)
	{
		if (c.lock()->type_ == Collider::TYPE::WALL)
		{
			// 地面との衝突
			auto hit = MV1CollCheck_Line(
				c.lock()->modelId_, -1, VAdd(prePos_,VScale(AsoUtility::DIR_U, 200.0f))
				, VAdd(transform_.pos, VScale(AsoUtility::DIR_U, 200.0f)));
			
			auto hitD = MV1CollCheck_Line(
				c.lock()->modelId_, -1, prePos_, transform_.pos);

			// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
			if (hit.HitFlag > 0 || hitD.HitFlag > 0)
			{
				movedPos_ = prePos_;

				transform_.pos = movedPos_;
				transform_.Update();
			}
		}
	}
}


#pragma region 衝突関係

void CharaBase::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
}
void CharaBase::ClearCollider(void)
{
	colliders_.clear();
}

#pragma endregion


void CharaBase::SetAreaId(int id)
{
	areaId_ = id;
}


void CharaBase::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 最初は実装しない。地面と突き抜けることを確認する。
	// 内積
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
		jumpPow_ = gravity;
	}

}

void CharaBase::CountTime(float& time)
{
	if (time>0.0f)
	{
		time -= SceneManager::GetInstance().GetDeltaTime();
	}
}


std::unique_ptr<CharaBase::AttrckData> CharaBase::SetAtrckData(
	int nextAtkId, float sHitTim, float HitTim, float sNewTime, bool isChage, int chargeId)
{
	auto atk = std::make_unique<AttrckData>();

	// チャージ関連
	atk->isCharge = isChage;	// チャージ攻撃か
	atk->chargeId = chargeId;	// ナンバー
	// 判定時間
	atk->sHitTime = sHitTim;	// 判定発生時間
	atk->HitTime = HitTim;		// 判定終了時間
	atk->sNewTime = sNewTime;	// n入力受付時間
	atk->NewTime = 0.0f;		// n入力受付終了時間
	// 次の攻撃ID
	atk->nextAttrck = nextAtkId;
	 
	return atk;
}
