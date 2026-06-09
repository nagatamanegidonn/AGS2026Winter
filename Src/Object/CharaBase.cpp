#include "CharaBase.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "Stage/Planet.h"

CharaBase::CharaBase(void)
	:
	areaId_(-1),
	attackDamage_(10),
	isHitCheck_(false),
	changeAttackTime_(0.0f),
	speed_(0.0f),
	stepRotTime_(0.0f),
	goalQuaRot_(),
	jumpPow_({ 0.0f,0.0f,0.0f }),
	imgShadow_(-1),
	moveDir_({ 0.0f,0.0f,0.0f }),
	movePow_({ 0.0f,0.0f,0.0f }),
	movedPos_({ 0.0f,0.0f,0.0f }),
	prePos_({ 0.0f,0.0f,0.0f }),
	attackRate_(1.0f),
	ActorBase()
{
	// 丸影画像
	imgShadow_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::PLAYER_SHADOW).handleId_;
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

void CharaBase::DrawShadow(void)
{
	float PLAYER_SHADOW_HEIGHT = 300.0f;
	float PLAYER_SHADOW_SIZE = 30.0f;

	int i;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3] = { VERTEX3D(), VERTEX3D(), VERTEX3D() };
	VECTOR SlideVec;
	int ModelHandle;

	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// 影を落とすモデルの数だけ繰り返し
	for (const auto c : colliders_)
	{
		// チェックするモデルは、jが0の時はステージモデル、1以上の場合はコリジョンモデル
		ModelHandle = c.lock()->modelId_;

		// プレイヤーの直下に存在する地面のポリゴンを取得
		HitResDim = MV1CollCheck_Capsule(
			ModelHandle, -1,
			transform_.pos, VAdd(transform_.pos, { 0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f }), PLAYER_SHADOW_SIZE);

		// 頂点データで変化が無い部分をセット
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		// 球の直下に存在するポリゴンの数だけ繰り返し
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			// ポリゴンの座標は地面ポリゴンの座標
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			// ちょっと持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			// ポリゴンの不透明度を設定する
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = static_cast<int>(roundf(128.0f * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT)));

			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = static_cast<int>(roundf(128.0f * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT)));

			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = static_cast<int>(roundf(128.0f * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT)));

			// ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

			// 影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);
	}

	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);

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
