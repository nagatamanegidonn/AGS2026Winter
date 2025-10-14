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

#include "../Statge/Planet.h"

#include "HitDamage.h"
#include "HitPart.h"
#include "SmallMonster.h"

SmallMonster::SmallMonster(int key, int createNo)
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

	//攻撃情報
	attrckCount_ = 0;
	//attrckTypeState_ = ATTRCK_TYPE::NONE;
	attrckDamage_ = 10;
	//追跡対象
	follow_ = nullptr;
	followTime_ = 0.0f;
	//攻撃位置
	attrckPos_ = AsoUtility::VECTOR_ZERO;
	dameRate_ = 1.0f;

	//当たり判定
	hitParts_.clear();
	AddHitPart(transform_.modelId, L"Bone002", ATTRCK_BITE_RADIUS, 1.0f);//胴体
	
}

SmallMonster::~SmallMonster(void)
{
	soundController_.reset();

	MV1DeleteModel(transform_.modelId);
}

//初期化処理
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

	//位置情報の変数
	movedPos_ = AsoUtility::VECTOR_ZERO;
	moveDir_ = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;

	//重力兼ジャンプ力
	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 310.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 300.0f, 0.0f });
	capsule_->SetRadius(200.0f);

	auto& nIns = NetManager::GetInstance();
	auto& users = NetManager::GetInstance().GetNetUsers();

	// ＨＰの初期化
	hp_ = hpMax_ = MAX_HP * users.size();

	isHitCheck_ = false;

	transform_.MakeCollider(Collider::TYPE::WALL);
	hitDamePos_ = AsoUtility::VECTOR_ZERO;
}
//更新処理
void SmallMonster::Update(void)
{
	auto& nIns = NetManager::GetInstance();

	auto& users = NetManager::GetInstance().GetNetUsers();

	animeAgoType_ = animeType_;

	//ダメージ処理
	int dame = 0;
	for (auto& user : users)
	{
		const int userDame = nIns.GetNetBossDamage(user.first);

		dame += nIns.GetNetMonsDamage(user.first, createNo_);
		//damage表記//ダメージを受けていたなら
		if (userDame > 0)
		{
			bool isEnd = false;
			for (auto& hitdamage : hitdamages_)
			{//表示終了しているものがあるなら
				if (hitdamage->GetState() == HitDamage::STATE::END)
				{
					hitdamage->Init(userDame);
					isEnd = true;
					break;
				}
			}
			if (!isEnd)
			{
				auto  part = std::make_unique<HitDamage>(transform_.modelId, "Chest_M", userDame);
				hitdamages_.emplace_back(std::move(part));
			}
		}
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
		stateUpdate_();

		// 重力による移動量
		CalcGravityPow();

		// 衝突判定//仮で消す
		// ジャンプ量を加算
		// 重力方向の反対
		VECTOR dirUpGravity = AsoUtility::DIR_U;
		// 重力の強さ
		float gravityPow = Planet::DEFAULT_GRAVITY_POW;
		movePow_ = VAdd(movePow_, VScale(dirUpGravity, gravityPow));

		Collision();

		if (stateTime_ >= 0.0f)stateTime_ -= SceneManager::GetInstance().GetDeltaTime();


		// 重力方向に沿って回転させる
		transform_.quaRot = Quaternion();//grvMng_がないので代わりに
		transform_.quaRot = transform_.quaRot.Mult(playerRotY_);


		// 位置送信もここでOK（ProcessMove内でも呼ばれてるけど念のため）
		nIns.SetMonsData(key_, createNo_, transform_.pos, transform_.quaRot, animeType_, (int)state_);
	}
	//通信プレイヤーの処理
	else
	{
		//HPの同期
		hp_ = nIns.GetNetMonsHp(key_,createNo_);

		const MONSTER_DATA& boss = nIns.GetMonsData(key_, createNo_);

		animeType_ = boss.Anim_;
		state_ = static_cast<SmallMonster::STATE>(boss.state_);

		if (animeType_ == (int)ANIM_TYPE::DEAD)
		{
			animationController_->Play(animeType_, false);
		}
		else animationController_->Play(animeType_);


		const auto& pos = boss.postion_;
		transform_.pos = pos;
		const auto& rot = boss.rot_;
		transform_.quaRot = rot;

	}


	//ダメージ描画の更新
	for (auto& hitdamage : hitdamages_)
	{
		hitdamage->Update();
	}

	nIns.SetNetMonsDamage(nIns.GetSelf().key, createNo_, 0);

	// 描画用の位置は、Draw()でNetManagerから取るからOK
	transform_.Update();

	// アニメーション再生
	animationController_->Update();
	if (animationController_->IsEnd() && animeType_ == (int)ANIM_TYPE::DEAD)
	{
		
	}
}
//描画処理
void SmallMonster::Draw(void)
{

	MV1DrawModel(transform_.modelId);

	//ダメージの表記
	for (auto& hitdamage : hitdamages_)
	{
		hitdamage->Draw();
	}

#ifdef _DEBUG

	DrawDebug();

#endif // DEBUG
}

void SmallMonster::Damage(int dama)
{
	//無敵中はない

	auto& nIns = NetManager::GetInstance();

	const int lastDame = dama * dameRate_;

	nIns.SetNetBossDamage(nIns.GetSelf().key, lastDame);
}


const bool SmallMonster::CollisionCapsule(std::weak_ptr<Capsule> _capsule)
{
	//当たり判定フラグ
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
	/*if (animeType_==(int)ANIM_TYPE::ATTRCK_BITE)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK_BITE;
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, "TongueEnd_M");//牙攻撃用
		attrckRadius = ATTRCK_BITE_RADIUS;
	}*/
	if (animeType_ == (int)ANIM_TYPE::ATTRCK)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK;
		attrckPos_ = VScale(VAdd(AsoUtility::MV1GetFreamPos(transform_.modelId, L"Bone002")
			, AsoUtility::MV1GetFreamPos(transform_.modelId, L"Bone002")), 0.5f);
		attrckRadius = ATTRCK_STAMP_RADIUS;
	}
	else
	{
		return false;
	}

	if (atkData_[attrckType_]->sHitTime < animationController_->GetStepTime()
		&& atkData_[attrckType_]->HitTime > animationController_->GetStepTime())
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
	if (hits.HitNum >= 1)
	{
		// 当たった場合は衝突の情報を描画する
		ret = true;
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);

	return ret;
}



#pragma region 初期化処理

void SmallMonster::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Monster/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + L"Monster.mv1", 20.0f, 0);
	animationController_->Add((int)ANIM_TYPE::RUN, path + L"Monster.mv1", 30.0f, 1);

	animationController_->Add((int)ANIM_TYPE::ATTRCK_READY, path + L"Monster.mv1", 1.2f, 3, 3.0f, 0.1f);
	animationController_->Add((int)ANIM_TYPE::ATTRCK, path + L"Boss.mv1", 25.0f, 2);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + L"Boss.mv1", 40.0f, 3);
	animationController_->Add((int)ANIM_TYPE::DEAD, path + L"Boss.mv1", 30.0f, 4);

	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK, true, 1.0f);

	animationController_->Play((int)ANIM_TYPE::RUN);
	animeType_ = (int)ANIM_TYPE::RUN;
	animeAgoType_ = animeType_;
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
	animationController_->Play((int)ANIM_TYPE::RUN);
	animeType_ = (int)ANIM_TYPE::RUN;

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
	animationController_->Play((int)ANIM_TYPE::RUN);
	animeType_ = (int)ANIM_TYPE::RUN;

	VECTOR axis = AsoUtility::VECTOR_ZERO;
	axis.y = -0.5f * (createNo_ + 1);

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
		//ターゲットに向けて回転
		TargetRotate(follow_->pos, 0.3f);
		if (rotateTimer_ <= 0.0f)
		{
			rotateTimer_ = rotateInterval_; // リセット
		}
		//この時の回転は歩く
		animationController_->Play((int)ANIM_TYPE::RUN);
		animeType_ = (int)ANIM_TYPE::RUN;

	}
	else
	{
		animationController_->Play((int)ANIM_TYPE::IDLE, true);
		animeType_ = (int)ANIM_TYPE::IDLE;
	}

	// playerとの衝突判定
	const VECTOR diff = VSub(transform_.pos, follow_->pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;


	//視線の先に至らに変更予定
	if (stateTime_ < 0.0f || (IsTargetInFOV(follow_->pos, FOV_RADIUS) && stateTime_ < 1.0f)) {

		if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))//攻撃半径×攻撃半径
		{

			ChangeState(STATE::ATTRCK_READY);
		}
		//プレイヤーが索敵範囲内	//回り続けると回転だけなので突進を絡める
		else if (disPow > DASH_RADIUS * DASH_RADIUS) {
			//ChangeState(STATE::ATTRCK_DASH);//接近
		}
		//プレイヤーが索敵範囲内	//回り続けると回転だけなので突進を絡める
		else if (disPow < MOVE_RADIUS * MOVE_RADIUS) {
			ChangeState(STATE::FOLLOW);//接近
		}
		else {
			//ChangeState(STATE::PLAY);//バトル中止
		}
	}
}
void SmallMonster::UpdateFollow(void)
{
	animationController_->Play((int)ANIM_TYPE::RUN);
	animeType_ = (int)ANIM_TYPE::RUN;

	if (IsTargetInFOV(follow_->pos, FOV_RADIUS))
	{
		//ターゲットに向けて回転
		TargetRotate(follow_->pos);
	}

	// 移動
	movePow_ =
		VScale(transform_.GetForward(), SPEED_FOLLOW);

	// playerとの衝突判定
	VECTOR diff = VSub(transform_.pos, follow_->pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;


	if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))//ダメージ半径×攻撃半径
	{
		//攻撃方法の決定
		//if ((int)disPow % 2 == 0)
		//{
		//	//攻撃方法
		//	attrckTypeState_ = ATTRCK_TYPE::BITE;
		//}
		//else
		//{
		//	//攻撃方法
		//	attrckTypeState_ = ATTRCK_TYPE::CLOW_R;
		//	attrckCount_ = 3;
		//}

		ChangeState(STATE::ATTRCK_READY);
	}
	else if (stateTime_ < 0.0f)
	{
		ChangeState(STATE::BATTLE);
	}
}
void SmallMonster::UpdateAttrckReady(void)
{
	animationController_->Play((int)ANIM_TYPE::ATTRCK, false);
	animeType_ = (int)ANIM_TYPE::ATTRCK;

	if (animationController_->IsEnd())
	{
		auto& nIns = NetManager::GetInstance();

		nIns.SetAction(PLAYER_ACTION::BOSS_ATTRCK_A);
		ChangeState(STATE::ATTRCK);
	}
}
void SmallMonster::UpdateAttrckStamp(void)
{
	animationController_->Play((int)ANIM_TYPE::ATTRCK, false);
	animeType_ = (int)ANIM_TYPE::ATTRCK;

	if (animationController_->IsEnd())
	{
		ChangeState(STATE::BATTLE);
	}
}
void SmallMonster::UpdateDamage(void)
{
	animationController_->Play((int)ANIM_TYPE::DAMAGE, false);
	animeType_ = (int)ANIM_TYPE::DAMAGE;


	if (animationController_->IsEnd())
	{

	}
}
void SmallMonster::UpdateDead(void)
{
	animationController_->Play((int)ANIM_TYPE::DEAD, false);
	animeType_ = (int)ANIM_TYPE::DEAD;


	if (animationController_->IsEnd())
	{

	}
}

void SmallMonster::AttrckUpdate(void)
{
	animationController_->Play(attrckType_, false);

	//当たり判定が発生するか
	if (atkData_[attrckType_]->sHitTime < animationController_->GetStepTime()
		&& atkData_[attrckType_]->HitTime > animationController_->GetStepTime())
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
	//当たり判定
	for (const auto& part : hitParts_)
	{
		part->Draw();
	}
	capsule_->Draw();
}
