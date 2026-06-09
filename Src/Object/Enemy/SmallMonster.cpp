#include <string>
#include <DxLib.h>
#include "../../Application.h"
#include "../../Utility/AsoUtility.h"

#include "../../Manager/SceneManager.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/Camera.h"

#include "../../Net/NetManager.h"

#include "../Common/Collider/Capsule.h"

#include "../Common/AnimationController.h"
#include "../Common/EffectController.h"
#include "../Common/SoundController.h"

#include "../Stage/Planet.h"

#include "Hit/HitDamage.h"
#include "Hit/HitPart.h"
#include "SmallMonster.h"

namespace
{
	// アニメーションリスト
	const std::vector<CharaBase::AnimationInfo> ANIM_LIST =
	{
		// 通常アニメーション
		{static_cast<int>(SmallMonster::ANIM_TYPE::IDLE),L"SmallMonster.mv1",20.0f,0,0.0f, -1.0f},
		{static_cast<int>(SmallMonster::ANIM_TYPE::RUN),L"SmallMonster.mv1",30.0f,1,0.0f,-1.0f},
		// 攻撃アニメーション
		{static_cast<int>(SmallMonster::ANIM_TYPE::ATTRCK_READY),L"SmallMonster.mv1",0.3f, 3, 3.0f, 0.1f},
		{static_cast<int>(SmallMonster::ANIM_TYPE::ATTRCK),L"SmallMonster.mv1",20.0f, 2,0.0f,-1.0f},
		// 被ダメージアニメーション
		{static_cast<int>(SmallMonster::ANIM_TYPE::DAMAGE),L"SmallMonster.mv1",15.0f, 3,0.0f,-1.0f},
		{static_cast<int>(SmallMonster::ANIM_TYPE::DEAD),L"SmallMonster.mv1",30.0f, 4,0.0f,-1.0f},
	};

	constexpr float HALF_RATE = 0.5f;
}

SmallMonster::SmallMonster(int key, int createNo)
	:
	animeAgoType_(-1),
	animeType_(-1),
	stateTime_(0.0f)
{
	key_ = key;
	createNo_ = createNo;
	animationController_ = nullptr;
	state_ = STATE::NONE;

	// 状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&SmallMonster::ChangeStateNone, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&SmallMonster::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&SmallMonster::ChangeStateBattle, this));
	stateChanges_.emplace(STATE::FOLLOW, std::bind(&SmallMonster::ChangeStateFollow, this));
	stateChanges_.emplace(STATE::ATTRCK_READY, std::bind(&SmallMonster::ChangeStateAttrckReady, this));
	stateChanges_.emplace(STATE::ATTRCK, std::bind(&SmallMonster::ChangeStateAttrckStamp, this));
	stateChanges_.emplace(STATE::DAMAGE, std::bind(&SmallMonster::ChangeStateDamage, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&SmallMonster::ChangeStateDead, this));

	// 攻撃情報
	attrckCount_ = 0;
	attackDamage_ = 10;
	// 追跡対象
	follow_ = nullptr;
	followTime_ = 0.0f;
	// 攻撃位置
	attrckPos_ = AsoUtility::VECTOR_ZERO;
	dameRate_ = 1.0f;

	// 当たり判定
	hitParts_.clear();
	AddHitPart(transform_.modelId, L"Bone002", ATTRCK_BITE_RADIUS, 1.0f);//胴体
	
}

SmallMonster::~SmallMonster(void)
{
	soundController_.reset();

	MV1DeleteModel(transform_.modelId);
}

// 初期化処理
void SmallMonster::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::MONSTER));
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, SCALE_SIZE);
	// 初期座標
	transform_.pos = prePos_ = { 0.0f, -30.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f });
	transform_.Update();

	// アニメーションの設定
	InitAnimation();
	InitEffect();
	InitSound();

	// 初期状態
	ChangeState(STATE::PLAY);

	// 位置情報の変数
	movedPos_ = AsoUtility::VECTOR_ZERO;
	moveDir_ = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;

	// 重力兼ジャンプ力
	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 300.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 290.0f, 0.0f });
	capsule_->SetRadius(200.0f);

	auto& nIns = NetManager::GetInstance();
	auto& users = NetManager::GetInstance().GetNetUsers();

	// HPの初期化
	hp_ = hpMax_ = 10;

	isHitCheck_ = false;

	transform_.MakeCollider(Collider::TYPE::WALL);
	hitDamePos_ = AsoUtility::VECTOR_ZERO;
}
// 更新処理
void SmallMonster::Update(void)
{
	auto& nIns = NetManager::GetInstance();
	auto& users = NetManager::GetInstance().GetNetUsers();

	animeAgoType_ = animeType_;

	// ダメージの更新
	int dame = EnemyBase::DamageUpdate();

	// 当たり判定の設定
	for (const auto& part : hitParts_)
	{
		part->Update();
	}

	//当たり判定の設定
	for (const auto& part : hitParts_)
	{
		part->Update();
	}

	// 自分のプレイヤーのときだけ入力を処理する
	if (nIns.GetMode() == NET_MODE::HOST) {
		//ダメージを与える
		hp_ -= dame;
		nIns.SetNetMonsHp(key_, createNo_, hp_);

		//死亡判定
		if (hp_ <= 0.0f && state_ != STATE::DEAD) { ChangeState(STATE::DEAD); }

		movePow_ = AsoUtility::VECTOR_ZERO;

		// 更新ステップ
		bool isDebug = true;
#ifdef _DEBUG

		if (state_ != STATE::DEAD)
		{
			//isDebug = false;
		}

#endif // DEBUG
		if (isDebug) stateUpdate_();

		// 重力による移動量
		CalcGravityPow();

		// 衝突判定
		Collision();

		if (stateTime_ >= 0.0f)stateTime_ -= SceneManager::GetInstance().GetDeltaTime();

		// 重力方向に沿って回転させる
		transform_.quaRot = Quaternion();//grvMng_がないので代わりに
		transform_.quaRot = transform_.quaRot.Mult(playerRotY_);

		// 位置送信もここでOK（ProcessMove内でも呼ばれてるけど念のため）
		nIns.SetMonsData(key_, createNo_, transform_.pos, transform_.quaRot, animeType_, static_cast<int>(state_));
	}
	// 通信プレイヤーの処理
	else
	{
		// HPの同期
		hp_ = nIns.GetNetMonsHp(key_, createNo_);

		const MONSTER_DATA& mons = nIns.GetMonsData(key_, createNo_);

		animeType_ = mons.Anim_;
		state_ = static_cast<SmallMonster::STATE>(mons.state_);

		if (animeType_ == static_cast<int>(ANIM_TYPE::DEAD))
		{
			animationController_->Play(animeType_, false);
		}
		else animationController_->Play(animeType_);

		const auto& pos = mons.postion_;
		transform_.pos = pos;
		const auto& rot = mons.rot_;
		transform_.quaRot = rot;
	}


	// ダメージ描画の更新
	for (auto& hitdamage : hitdamages_)
	{
		hitdamage->Update();
	}

	nIns.SetNetMonsDamage(nIns.GetSelf().key, createNo_, 0);

	// 描画用の位置は、Draw()でNetManagerから取るからOK
	transform_.Update();

	// アニメーション再生
	animationController_->Update();
	if (animationController_->IsEnd() && animeType_ == static_cast<int>(ANIM_TYPE::DEAD))
	{
		ChangeState(STATE::NONE);
	}
}
// 描画処理
void SmallMonster::Draw(void)
{
	if (state_ != STATE::NONE)
	{
		MV1DrawModel(transform_.modelId);

		DrawShadow();
	}

	// ダメージの表記
	for (const auto& hitdamage : hitdamages_)
	{
		hitdamage->Draw();
	}

#ifdef _DEBUG

	DrawDebug();

#endif // DEBUG
}

void SmallMonster::Damage(int _dama, bool _isConst)
{
	auto& nIns = NetManager::GetInstance();

	float dameRate = dameRate_;		// ダメージ倍率
	if (_isConst)dameRate = 1.0f;	// 固定ダメージなら倍率無効

	const int lastDame = static_cast<int>(static_cast<float>(_dama) * dameRate);

	nIns.SetNetMonsDamage(nIns.GetSelf().key, createNo_,lastDame);
}

const bool SmallMonster::CollisionCapsule(std::weak_ptr<Capsule> _capsule)
{
	// 当たり判定フラグ
	bool ret = false;
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;

	for (auto& part : hitParts_)
	{

		// 衝突した複数のポリゴンと衝突回避するまで、
		// プレイヤーのdamage
		// 当たったかどうかで処理を分岐
		if (_capsule.lock()->IsHitSphere(part->GetPos(), part->GetRadius()))
		{
			// 当たった場合は衝突の情報を描画する
			ret = true;
			dameRate_ = part->GetDameRate();

			// 必要であれば保存しておく（例：メンバ変数 hitPos_ に）
			hitDamePos_ = part->GetPos();

			return ret;
		}

	}

	return ret;
}
const bool SmallMonster::CollisionAttrck(const int& modelId)
{

	auto& nIns = NetManager::GetInstance();
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK))
	{
		attackType_ = static_cast<int>(ANIM_TYPE::ATTRCK);
		attrckPos_ = VScale(VAdd(AsoUtility::MV1GetFreamPos(transform_.modelId, L"Bone002")
			, AsoUtility::MV1GetFreamPos(transform_.modelId, L"Bone002")), HALF_RATE);
		attrckRadius = ATTRCK_STAMP_RADIUS;
	}
	else
	{
		return false;
	}

	if (atkData_[attackType_]->sHitTime < animationController_->GetStepTime()
		&& atkData_[attackType_]->HitTime > animationController_->GetStepTime())
	{
		isHitCheck_ = true;
	}
	else
	{
		isHitCheck_ = false;
		return false;
	}

	// ０番目のフレームのコリジョン情報を更新する
	MV1RefreshCollInfo(modelId, -1);

	//当たり判定フラグ
	bool ret = false;

	// カプセルとの衝突判定
	auto hits = MV1CollCheck_Sphere(
		modelId, -1,
		attrckPos_, attrckRadius);

	// 衝突した複数のポリゴンと衝突回避するまで、
	// プレイヤーのdamage
	 // 当たったかどうかで処理を分岐
	if (hits.HitNum >= 1){
		// 当たった場合は衝突の情報を描画する
		ret = true;
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);

	return ret;
}

void SmallMonster::SetFollow(const Transform* follow)
{
	follow_ = follow;
	ChangeState(STATE::BATTLE);
}



#pragma region 初期化処理

void SmallMonster::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Enemy/Monster/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーションの登録
	for (const auto& anim : ANIM_LIST)
	{
		animationController_
			->Add(anim.type, path + anim.name, anim.speed, anim.loopNum, anim.startFrame, anim.endFrame);
	}

	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTRCK), true, 1.0f);

	animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::RUN);
	animeAgoType_ = animeType_;

	//攻撃情報の設定
	atkData_.emplace(static_cast<int>(ANIM_TYPE::ATTRCK), std::move(SetActionData(-1, 10.0f, 14.0f)));

}
void SmallMonster::InitEffect(void)
{
}
void SmallMonster::InitSound(void)
{
}

#pragma endregion

#pragma region 状態遷移

void SmallMonster::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	stateTime_ = 3.0f;

	// 各状態遷移の初期処理
	stateChanges_[state_]();
}
void SmallMonster::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&SmallMonster::UpdateNone, this);
}
void SmallMonster::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&SmallMonster::UpdatePlay, this);
}
void SmallMonster::ChangeStateBattle(void)
{
	rotateTimer_ = rotateInterval_; // リセット
	stateUpdate_ = std::bind(&SmallMonster::UpdateBattle, this);
}
void SmallMonster::ChangeStateFollow(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::RUN);

	stateUpdate_ = std::bind(&SmallMonster::UpdateFollow, this);
}
void SmallMonster::ChangeStateAttrckReady(void)
{
	stateUpdate_ = std::bind(&SmallMonster::UpdateAttrckReady, this);
}
void SmallMonster::ChangeStateAttrckStamp(void)
{
	stateUpdate_ = std::bind(&SmallMonster::UpdateAttrckStamp, this);
}
void SmallMonster::ChangeStateDamage(void)
{
	stateUpdate_ = std::bind(&SmallMonster::UpdateDamage, this);
}
void SmallMonster::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&SmallMonster::UpdateDead, this);
}

#pragma endregion

void SmallMonster::UpdateNone(void)
{
}
void SmallMonster::UpdatePlay(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::RUN);

	VECTOR axis = AsoUtility::VECTOR_ZERO;
	axis.y = -HALF_RATE * (createNo_ + 1);

	//回転
	if (!AsoUtility::EqualsVZero(axis)) {
		playerRotY_ = playerRotY_.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axis.y), AsoUtility::AXIS_Y
			));
	}

	// 前方向を取得
	VECTOR forward = transform_.GetForward();
	// 移動
	movePow_ =
		VScale(forward, SPEED_MOVE);
}
void SmallMonster::UpdateBattle(void)
{
	// タイマー更新
	rotateTimer_ -= SceneManager::GetInstance().GetDeltaTime(); // フレームの経過時間を使う（環境によって異なります）
	
	// 一定時間ごとに回転処理
	if (rotateTimer_ <= 1.0f)
	{
		// ターゲットに向けて回転
		TargetRotate(follow_->pos, 0.3f);
		if (rotateTimer_ <= 0.0f)
		{
			rotateTimer_ = rotateInterval_; // リセット
		}
		// この時の回転は歩く
		animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
		animeType_ = static_cast<int>(ANIM_TYPE::RUN);

	}
	else
	{
		animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
		animeType_ = static_cast<int>(ANIM_TYPE::IDLE);
	}

	// プレイヤーとの衝突判定
	const VECTOR diff = VSub(transform_.pos, follow_->pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;


	// 視線の先に至らに変更予定
	if (stateTime_ < 0.0f || (IsTargetInFOV(follow_->pos, FOV_RADIUS) && stateTime_ < 1.0f)) 
	{		
		//　攻撃半径×攻撃半径
		if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))
		{
			ChangeState(STATE::ATTRCK_READY);
		}
		// プレイヤーが索敵範囲内	// 回り続けると回転だけなので突進を絡める
		else if (disPow < MOVE_RADIUS * MOVE_RADIUS) {
			ChangeState(STATE::FOLLOW);// 接近
		}
	}
}
void SmallMonster::UpdateFollow(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::RUN);

	if (IsTargetInFOV(follow_->pos, FOV_RADIUS))
	{
		// ターゲットに向けて回転
		TargetRotate(follow_->pos);
	}

	// 移動
	movePow_ =
		VScale(transform_.GetForward(), SPEED_FOLLOW);

	// プレイヤーとの衝突判定
	VECTOR diff = VSub(transform_.pos, follow_->pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	// ダメージ半径×攻撃半径
	if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))
	{
		ChangeState(STATE::ATTRCK_READY);
	}
	else if (stateTime_ < 0.0f)
	{
		ChangeState(STATE::BATTLE);
	}
}
void SmallMonster::UpdateAttrckReady(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTRCK), false);
	animeType_ = static_cast<int>(ANIM_TYPE::ATTRCK);

	if (animationController_->IsEnd())
	{
		auto& nIns = NetManager::GetInstance();

		nIns.SetAction(PLAYER_ACTION::BOSS_ATTRCK_A);
		ChangeState(STATE::ATTRCK);
	}
}
void SmallMonster::UpdateAttrckStamp(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTRCK), false);
	animeType_ = static_cast<int>(ANIM_TYPE::ATTRCK);

	if (animationController_->IsEnd())
	{
		ChangeState(STATE::BATTLE);
	}
}
void SmallMonster::UpdateDamage(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::DAMAGE), false);
	animeType_ = static_cast<int>(ANIM_TYPE::DAMAGE);
}
void SmallMonster::UpdateDead(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::DEAD), false);
	animeType_ = static_cast<int>(ANIM_TYPE::DEAD);
}

void SmallMonster::CollisionStageCapsule(void)
{
	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);
	// カプセルとの衝突判定(主にステージ)
	for (const auto c : colliders_)
	{
		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		if (c.lock()->type_ == Collider::TYPE::STAGE)
		{
			// 衝突した複数のポリゴンと衝突回避するまで、
			// プレイヤーの位置を移動させる
			for (int i = 0; i < hits.HitNum; i++)
			{
				auto hit = hits.Dim[i];

				// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
				// この時、移動させる方向は、移動前座標に向いた方向であったり、
				// 衝突したポリゴンの法線方向だったりする
				for (int tryCnt = 0; tryCnt < 10; tryCnt++)
				{
					// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
					// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
					int pHit = HitCheck_Capsule_Triangle(
						cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
						hit.Position[0], hit.Position[1], hit.Position[2]);

					if (pHit)
					{
						// 法線の方向にちょっとだけ移動させる
						movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
						// カプセルも一緒に移動させる
						trans.pos = movedPos_;
						trans.Update();
						continue;
					}
					break;
				}
			}
		}
		else if (c.lock()->type_ == Collider::TYPE::WALL)
		{
			// 衝突した複数のポリゴンと衝突回避するまで、
			// プレイヤーの位置を移動させる
			for (int i = 0; i < hits.HitNum; i++)
			{
				auto hit = hits.Dim[i];

				// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
				// この時、移動させる方向は、移動前座標に向いた方向であったり、
				// 衝突したポリゴンの法線方向だったりする
				for (int tryCnt = 0; tryCnt < 10; tryCnt++)
				{
					// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
					// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
					int pHit = HitCheck_Capsule_Triangle(
						cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
						hit.Position[0], hit.Position[1], hit.Position[2]);

					if (pHit)
					{
						//法線方向高さなし
						VECTOR nor = hit.Normal;
						nor.y = 0.0f;

						// 法線の方向にちょっとだけ移動させる
						movedPos_ = VAdd(movedPos_, VScale(nor, 1.0f));
						// カプセルも一緒に移動させる
						trans.pos = movedPos_;
						trans.Update();
						continue;
					}
					break;
				}
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void SmallMonster::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{

		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{

			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
		}

	}
}

void SmallMonster::AttrckUpdate(void)
{
	animationController_->Play(attackType_, false);

	//　当たり判定が発生するか
	if (atkData_[attackType_]->sHitTime < animationController_->GetStepTime()
		&& atkData_[attackType_]->HitTime > animationController_->GetStepTime())
	{
		isHitCheck_ = true;
	}
	else
	{
		isHitCheck_ = false;
	}
}

void SmallMonster::DrawDebug(void)
{
	//　当たり判定
	for (const auto& part : hitParts_)
	{
		part->Draw();
	}

	capsule_->Draw();
}
