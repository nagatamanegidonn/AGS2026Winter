#include "CharaBase.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/Utility.h"
#include "Stage/Planet.h"

namespace
{
	// 初期座標・落下救済ライン
	const VECTOR RESPAWN_POS = { 0.0f, -30.0f, 0.0f };
	constexpr float FALL_OUT_LINE = -1000.0f; // このY座標以下に落ちたらリスポーン
	// コリジョン・レイキャスト設定
	constexpr float COLL_LINE_OFFSET = 200.0f; // 地面判定レイのブレ防止用上方向オフセット

	// 丸影描画（ポリスープ投影）設定
	constexpr float PLAYER_SHADOW_HEIGHT = 300.0f; // 影が届く最大高度
	constexpr float PLAYER_SHADOW_SIZE = 30.0f;  // 影の半径
	constexpr float SHADOW_SLIDE_FACTOR = 0.5f;   // 地面ポリゴンとのZファイティング（チラつき）防止オフセット
	constexpr float SHADOW_MAX_ALPHA = 128.0f; // 最低高度（密着時）の不透明度最大値

	// カラー・ブレンド設定
	constexpr unsigned char COLOR_RGB_MAX = 255;
	constexpr unsigned char COLOR_RGB_MIN = 0;
}

CharaBase::CharaBase(void)
	:
	areaId_(-1),
	attackDamage_(0),
	isHitCheck_(false),
	changeAttackTime_(0.0f),
	speed_(0.0f),
	stepRotTime_(0.0f),
	goalQuaRot_(),
	jumpPow_(Utility::VECTOR_ZERO),
	moveDir_(Utility::VECTOR_ZERO),
	movePow_(Utility::VECTOR_ZERO),
	movedPos_(Utility::VECTOR_ZERO),
	prePos_(Utility::VECTOR_ZERO),
	attackRate_(1.0f),
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

	if (transform_.pos.y <= FALL_OUT_LINE)
	{
		transform_.pos = RESPAWN_POS;
	}

	// 移動判定
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
	for (const auto& c : colliders_)
	{
		if (c.lock()->type_ == Collider::TYPE::WALL)
		{
			// 地面との衝突
			auto hit = MV1CollCheck_Line(
				c.lock()->modelId_, -1, VAdd(prePos_,VScale(Utility::DIR_U, COLL_LINE_OFFSET))
				, VAdd(transform_.pos, VScale(Utility::DIR_U, COLL_LINE_OFFSET)));
			
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
	VECTOR dirGravity = Utility::DIR_D;

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

std::unique_ptr<CharaBase::ActionData> CharaBase::SetActionData(
	int nextAtkId, float sHitTim, float HitTim, float sNewTime, bool isChage, int chargeId)
{
	auto atk = std::make_unique<ActionData>();

	// チャージ関連
	atk->isCharge = isChage;	// チャージ攻撃か
	atk->chargeId = chargeId;	// ナンバー
	// 判定時間
	atk->sHitTime = sHitTim;	// 判定発生時間
	atk->HitTime = HitTim;		// 判定終了時間
	atk->sNewTime = sNewTime;	// n入力受付時間
	atk->NewTime = 0.0f;		// n入力受付終了時間
	// 次の攻撃ID
	atk->nextAttack = nextAtkId;
	 
	return atk;
}

void CharaBase::SetActionData(int id, const ActionData& data)
{
	auto atk = std::make_unique<ActionData>();

	// チャージ関連
	atk->isCharge = data.isCharge;	// チャージ攻撃か
	atk->chargeId = data.chargeId;	// ナンバー
	// 判定時間
	atk->sHitTime = data.sHitTime;	// 判定発生時間
	atk->HitTime = data.HitTime;	// 判定終了時間
	atk->sNewTime = data.sNewTime;	// n入力受付時間
	atk->NewTime = 0.0f;			// n入力受付終了時間
	// 次の攻撃ID
	atk->nextAttack = data.nextAttack;

	atkData_.emplace(id, std::move(atk));
}
