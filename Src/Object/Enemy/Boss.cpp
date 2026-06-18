#include <string>
#include <DxLib.h>
#include "../../Application.h"
#include "../../Utility/AsoUtility.h"

#include "../../Manager/SceneManager.h"
#include "../../Manager/GameManager.h"
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
#include "Boss.h"

namespace
{
	// ボーン名
	const std::wstring BONE_CHEST = L"Chest_M";
	const std::wstring BONE_FINGERS_L = L"Fingers1_L";
	const std::wstring BONE_FINGERS_R = L"Fingers1_R";
	// パス・リソース関係
	const std::wstring DIR_PATH_BOSS = L"Enemy/Boss/";
	const std::wstring EFFECT_FILE_DASH = L"Dash/Dash.efkefc";
	const std::wstring EFFECT_FILE_DAME = L"Damage/Damage.efkefc";
	const std::wstring SOUND_FILE_DASH = L"Boss/Dash.mp3";
	const std::wstring SOUND_FILE_CLOW = L"Boss/Clow.mp3";
	const std::wstring SOUND_FILE_STAMP = L"Boss/Stamp.mp3";
	// アニメーションリスト
	const std::vector<CharaBase::AnimationInfo> ANIM_LIST =
	{
		// 通常アニメーション
		{static_cast<int>(Boss::ANIM_TYPE::IDLE),L"Boss.mv1",20.0f,0,0.0f, -1.0f},
		{static_cast<int>(Boss::ANIM_TYPE::RUN),L"Boss.mv1",30.0f,6,0.0f,-1.0f},
		{static_cast<int>(Boss::ANIM_TYPE::FAST_RUN),L"Boss.mv1",30.0f, 2,0.0f,-1.0f},
		// 攻撃アニメーション
		{static_cast<int>(Boss::ANIM_TYPE::READY_ATTRCK),L"Boss.mv1",1.2f, 12, 0.0f, 1.5f},
		{static_cast<int>(Boss::ANIM_TYPE::ATTRCK_STAMP),L"Boss.mv1",25.0f, 10,0.0f,-1.0f},
		{static_cast<int>(Boss::ANIM_TYPE::ATTRCK_L_CLOW),L"Boss.mv1",20.0f, 8,0.0f,-1.0f},
		{static_cast<int>(Boss::ANIM_TYPE::ATTRCK_R_CLOW),L"Boss.mv1",20.0f, 5,0.0f,-1.0f},
		{static_cast<int>(Boss::ANIM_TYPE::ATTRCK_DASH),L"Boss.mv1",40.0f, 2,0.0f,-1.0f},
		// 被ダメージアニメーション
		{static_cast<int>(Boss::ANIM_TYPE::STUNNED),L"Boss.mv1",30.0f, 14,0.0f,-1.0f},
		{static_cast<int>(Boss::ANIM_TYPE::DEAD),L"Boss.mv1", 30.0f, 13,0.0f,-1.0f},
	};
	// ヒットパーツリスト
	const std::vector<HitPart::HitPartData> PART_LIST =
	{
		{L"Chest_M", 150.0f, 1.0f},		// 胴体
		{L"Root_M", 90.0f, 1.5f},		// 尻
		{L"Spine2_M", 120.0f, 1.2f},	// 腰
		{L"Tongue1_M", 120.0f, 1.0f},	// 胸
		// 左前足
		{L"Shoulder_L", 75.0f, 0.5f},
		{L"ElbowPart1_L", 75.0f, 0.5f},
		// 右前足
		{L"Shoulder_R", 75.0f, 0.5f},
		{L"ElbowPart1_R", 75.0f, 0.5f},
		// 左後足
		{L"Hip_L", 75.0f, 1.0f},
		{L"Knee_L", 75.0f, 1.0f},
		// 右後足
		{L"Hip_R", 75.0f, 1.0f},
		{L"Knee_R", 75.0f, 1.0f},
	};
	// クリア後の待機時間
	constexpr float CLEAR_WAIT_TIME = 10.0f;
	// アタックデータリスト
	const CharaBase::ActionData ATTRCK_STAMP_DATA = { false, -1, 17.0f, 24.0f,-1.0f,0.0f,-1, };
	const CharaBase::ActionData ATTRCK_L_CLOW_DATA = { false, -1, 9.0f, 17.0f,-1.0f,0.0f,-1, };
	const CharaBase::ActionData ATTRCK_R_CLOW_DATA = { false, -1, 9.0f, 17.0f,-1.0f,0.0f,-1, };
	const CharaBase::ActionData ATTRCK_DASH_DATA = { false, -1, 9.0f, 17.0f,-1.0f,0.0f,-1, };
	constexpr int ATTACK_COUNT = 3;
	// カプセルの初期値
	constexpr VECTOR CAP_LOACL_TOP = { 0.0f, 310.0f, 0.0f };
	constexpr VECTOR CAP_LOACL_DOWN = { 0.0f, 300.0f, 0.0f };
	constexpr float CAP_RADIUS = 200.0f;
	constexpr float ROT_RATE = 3.0f;
	// 初期座標
	constexpr VECTOR START_POS = { 0.0f, -30.0f, 0.0f };
	// 状態維持時間
	constexpr float DEFAULT_STATE_TIME = 3.0f;
	constexpr float STUNNED_STATE_DURATION = 4.0f;
	// 衝突、判定関係
	constexpr float JUMP_DOT_BORDER = 0.9f;
	constexpr float COL_CHECK_UP_POW = 10.0f * 2.0f;
	constexpr float COL_CHECK_DOWN_POW = 10.0f;
	constexpr float PUSH_BACK_LENGTH = 2.0f;
	constexpr float STAMP_POSTION_RATE = 0.5f;
	// アニメーションブレンド時間
	constexpr float BLEND_TIME_BATTLE_IDLE = 5.0f;
	constexpr float BLEND_TIME_STAMP = 1.0f;
	constexpr float BLEND_TIME_STUNNED = 3.0f;
	// スタンプ攻撃のSE発音タイミング（アニメーションのステップ時間）
	constexpr float SE_STAMP_PLAY_START = 23.0f;
	constexpr float SE_STAMP_PLAY_END = 23.5f;
	// 視野角デバッグ用パラメータ
	constexpr int DEBUG_FOV_DIVISIONS = 15;
}

Boss::Boss(int key, int createNo)
	:
	EnemyBase(),
	state_(STATE::NONE),
	animeType_(0),
	animeAgoType_(0),
	stateTime_(0.0f)
{
	key_ = key;
	createNo_ = createNo;

	animationController_ = nullptr;

	// 状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Boss::ChangeStateNone, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Boss::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::LERP_MOVE, std::bind(&Boss::ChangeStateLerpMove, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&Boss::ChangeStateBattle, this));
	stateChanges_.emplace(STATE::FOLLOW, std::bind(&Boss::ChangeStateFollow, this));
	stateChanges_.emplace(STATE::ATTRCK_READY, std::bind(&Boss::ChangeStateAttrckReady, this));
	stateChanges_.emplace(STATE::ATTRCK_STAMP, std::bind(&Boss::ChangeStateAttrckStamp, this));
	stateChanges_.emplace(STATE::ATTRCK_L_CLOW, std::bind(&Boss::ChangeStateAttrckLeftClaw, this));
	stateChanges_.emplace(STATE::ATTRCK_R_CLOW, std::bind(&Boss::ChangeStateAttrckRightClaw, this));
	stateChanges_.emplace(STATE::ATTRCK_DASH, std::bind(&Boss::ChangeStateAttrckDash, this));
	stateChanges_.emplace(STATE::STUNNED, std::bind(&Boss::ChangeStateStunned, this));
	stateChanges_.emplace(STATE::DAMAGE, std::bind(&Boss::ChangeStateDamage, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Boss::ChangeStateDead, this));

	// 攻撃情報
	attrckCount_ = 0;
	attrckTypeState_ = ATTRCK_TYPE::NONE;
	attackDamage_ = 0;

	// 追跡対象
	follow_ = nullptr;
	followTime_ = 0.0f;

	// 攻撃位置
	attrckPos_ = AsoUtility::VECTOR_ZERO;
	attrckRadius = 0.0f;
	dameRate_ = 1.0f;

	// 当たり判定
	hitParts_.clear();
	for (auto& part : PART_LIST)
	{
		AddHitPart(transform_.modelId, part.boneName, part.rad, part.rate);;
	}

	// LERP移動関係
	lerpTime_ = MAX_LERP_TIME;
	waypoint_ = AsoUtility::VECTOR_ZERO;
	isLerp_ = false;
	lerpId_ = 0;
}

Boss::~Boss(void)
{
	soundController_.reset();

	MV1DeleteModel(transform_.modelId);
}

void Boss::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::BOSS));
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, SCALE_SIZE);
	// 初期座標
	transform_.pos = prePos_ = START_POS;
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({
		AsoUtility::Deg2RadF(BOSS_LOCAL_ROT.x),
		AsoUtility::Deg2RadF(BOSS_LOCAL_ROT.y),
		AsoUtility::Deg2RadF(BOSS_LOCAL_ROT.z) });
	transform_.Update();

	// アニメーションの設定
	InitAnimation();

	// エフェクトの設定
	InitEffect();

	// サウンドの設定
	InitSound();

	// 初期状態
	ChangeState(STATE::PLAY);

	// 位置情報の変数
	movedPos_ = AsoUtility::VECTOR_ZERO;
	moveDir_ = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;

	//重力兼ジャンプ力
	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop(CAP_LOACL_TOP);
	capsule_->SetLocalPosDown(CAP_LOACL_DOWN);
	capsule_->SetRadius(CAP_RADIUS);

	// HP初期化
	auto& users = NetManager::GetInstance().GetNetUsers();
	hp_ = hpMax_ = static_cast<int>(static_cast<float>(MAX_HP) * static_cast<float>(users.size()));

	isHitCheck_ = false;

	transform_.MakeCollider(Collider::TYPE::WALL);
	hitDamePos_ = AsoUtility::VECTOR_ZERO;
}

void Boss::Update(void)
{
	auto& nIns = NetManager::GetInstance();
	auto& users = nIns.GetNetUsers();

	animeAgoType_ = animeType_;

	// ダメージの更新
	int dame = EnemyBase::DamageUpdate(BONE_CHEST);

	// 攻撃情報の更新
	AttackDataUpdate();

	// 当たり判定の設定
	for (const auto& part : hitParts_)
	{
		part->Update();
	}

	// 自分のプレイヤーのときだけ入力を処理する
	if (nIns.GetMode() == NET_MODE::HOST)
	{
		// ダメージを与える
		hp_ -= dame;
		nIns.SetNetMonsHp(key_, createNo_, hp_);

		// 死亡判定
		if (hp_ <= 0.0f && state_ != STATE::DEAD) { ChangeState(STATE::DEAD); }

		movePow_ = AsoUtility::VECTOR_ZERO;

		// 更新ステップ
		stateUpdate_();

		// 重力による移動量
		CalcGravityPow();

		// 衝突判定
		Collision();

		// 状態の制限時間カウント
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
		state_ = static_cast<Boss::STATE>(mons.state_);

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

	// ダッシュエフェクト
	effectController_->LoopUpdate(DASH_EFFECT, transform_.pos, transform_.rot, DASH_EFFECT_SIZE);
	effectController_->Update(DAMAGE_EFFECT);

	// 音の再生
	const auto& selfPos = nIns.GetPostion(nIns.GetSelf().key);
	// 音量設定
	float volume = AsoUtility::CalcVolumeByDistance(selfPos, transform_.pos, (MOVE_RADIUS + MOVE_RADIUS));

	// 無音なら停止
	soundController_->ChengeVolume(DASH_SOUND, volume);			// ボリュームだけ更新
	soundController_->ChengeVolume(CLOW_ATTACK_SOUND, volume);	// ボリュームだけ更新
	soundController_->ChengeVolume(STAMP_ATTACK_SOUND, volume); // ボリュームだけ更新

	// 音の再生
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_STAMP)
		&& animationController_->GetStepTime() > SE_STAMP_PLAY_START
		&& animationController_->GetStepTime() < SE_STAMP_PLAY_END)
	{
		soundController_->Play(STAMP_ATTACK_SOUND, Sound::TIMES::ONCE);
	}
	else if (
		(animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW))||
		(animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW)
			&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW))
		)
	{
		soundController_->Play(CLOW_ATTACK_SOUND, Sound::TIMES::ONCE);
	}
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_DASH)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ATTRCK_DASH)
		)
	{
		soundController_->Play(DASH_SOUND, Sound::TIMES::ONCE);
	}
	else if (animeType_ == static_cast<int>(ANIM_TYPE::DEAD)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::DEAD))
	{
		SceneManager::GetInstance().CaptureMainScreen();
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

	// クエストがボス討伐ならカウントする//bossの死亡アニメーション開始時に1回だけ
	if (animeAgoType_ != static_cast<int>(ANIM_TYPE::DEAD)
		&& animeType_ == static_cast<int>(ANIM_TYPE::DEAD)
		)
	{
		// クエストがボス討伐ならカウントアップ
		GameManager::GetInstance().SetClearCount(
		GameManager::GetInstance().GetClearCount() + 1);

		if (GameManager::GetInstance().IsClear())
		{
			GameManager::GetInstance().SetClearTime(CLEAR_WAIT_TIME);
		}
	}
}

void Boss::Draw(void)
{
	// モデルの描画
	MV1DrawModel(transform_.modelId);

	//ダメージの表記
	for (auto& hitdamage : hitdamages_)
	{
		hitdamage->Draw();
	}

#ifdef _DEBUG

	// デバッグ用
	DrawDebug();

#endif
}

void Boss::Damage(int _dama,bool _isConst)
{
	// ダメージエフェクト再生
	effectController_->Play(DAMAGE_EFFECT, hitDamePos_, AsoUtility::VECTOR_ZERO, DAMAGE_EFFECT_SIZE);

	auto& nIns = NetManager::GetInstance();

	float dameRate = dameRate_;
	if (_isConst)dameRate = 1.0f;// 固定ダメージなら倍率無効

	const int lastDame = static_cast<int>(static_cast<float>(_dama) * dameRate);
	// ダメージの設定
	nIns.SetNetMonsDamage(nIns.GetSelf().key, createNo_, lastDame);
}

const bool Boss::CollisionCapsule(std::weak_ptr<Capsule> _capsule)
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

const bool Boss::CollisionAttrck(const int& modelId)
{
	auto& nIns = NetManager::GetInstance();
	if (animeType_==static_cast<int>(ANIM_TYPE::ATTRCK_STAMP))
	{
		attackType_ = static_cast<int>(ANIM_TYPE::ATTRCK_STAMP);
		attrckPos_ = VScale(VAdd(
			AsoUtility::MV1GetFreamPos(transform_.modelId, BONE_FINGERS_L),
			AsoUtility::MV1GetFreamPos(transform_.modelId, BONE_FINGERS_R)),
			STAMP_POSTION_RATE);
		attrckRadius = ATTRCK_STAMP_RADIUS;
	}
	else if (animeType_==static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW))
	{
		attackType_ = static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW);
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, BONE_FINGERS_L);
		attrckRadius = ATTRCK_BITE_RADIUS;
	}
	else if (animeType_==static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW))
	{
		attackType_ = static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW);
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, BONE_FINGERS_R);
		attrckRadius = ATTRCK_BITE_RADIUS;
	}
	else if (animeType_==static_cast<int>(ANIM_TYPE::ATTRCK_DASH))
	{
		attackType_ = static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW);
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, BONE_CHEST);
		attrckRadius = ATTRCK_DASH_RADIUS;
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
	if (hits.HitNum >= 1)
	{
		// 当たった場合は衝突の情報を描画する
		ret = true;
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);

	return ret;
}

void Boss::SetLerpPos(VECTOR pos)
{
	waypoint_ = pos;
	isLerp_ = true;
}

void Boss::StartLerp(void)
{
	lerpId_ = 0;
	ChangeState(STATE::LERP_MOVE);
}

void Boss::SetFollow(const Transform* follow)
{
	follow_ = follow;
	if (state_ != STATE::STUNNED)ChangeState(STATE::BATTLE);
}

void Boss::SetBattleCancel(void)
{
	lerpTime_ = MAX_LERP_TIME;
	if (state_ != STATE::STUNNED)ChangeState(STATE::PLAY);
}

void Boss::StartStunned(void)
{
	if (state_ != STATE::STUNNED)ChangeState(STATE::STUNNED);
}

bool Boss::IsBattle(void) const
{
	if (
		state_ != STATE::LERP_MOVE &&
		state_ != STATE::PLAY &&
		state_ != STATE::DEAD &&
		state_ != STATE::HOWLING
		)
	{
		return true;
	}
	return false;
}

void Boss::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + DIR_PATH_BOSS;
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーションの登録
	for (const auto& anim : ANIM_LIST)
	{
		animationController_
			->Add(anim.type, path + anim.name, anim.speed, anim.loopNum, anim.startFrame, anim.endFrame);
	}

	// 攻撃データの設定
	SetActionData(static_cast<int>(ANIM_TYPE::ATTRCK_STAMP), ATTRCK_STAMP_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW), ATTRCK_L_CLOW_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW), ATTRCK_R_CLOW_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTRCK_DASH), ATTRCK_DASH_DATA);

	// ブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_IDLE), true, BLEND_TIME_BATTLE_IDLE);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTRCK_STAMP), true, BLEND_TIME_STAMP);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::STUNNED), true, BLEND_TIME_STUNNED);

	animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::RUN);
	animeAgoType_ = animeType_;
}

void Boss::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();

	effectController_->Add(DASH_EFFECT, path + EFFECT_FILE_DASH);
	effectController_->Add(DAMAGE_EFFECT, path + EFFECT_FILE_DAME);
}
void Boss::InitSound(void)
{
	std::wstring path = Application::PATH_SOUND;
	soundController_ = std::make_unique<SoundController>();

	soundController_->Add(DASH_SOUND, path + SOUND_FILE_DASH, 1.0f);
	soundController_->Add(CLOW_ATTACK_SOUND, path + SOUND_FILE_CLOW, 1.0f);
	soundController_->Add(STAMP_ATTACK_SOUND, path + SOUND_FILE_STAMP, 2.0f);
}

#pragma region StateによるUpdateの切り替え

void Boss::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	stateTime_ = DEFAULT_STATE_TIME;

	// 各状態遷移の初期処理
	stateChanges_[state_]();
}

void Boss::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateNone, this);
}

void Boss::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Boss::UpdatePlay, this);
}

void Boss::ChangeStateLerpMove(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateLerpMove, this);
}

void Boss::ChangeStateBattle(void)
{
	rotateTimer_ = ROT_INTERVAL; // リセット
	stateUpdate_ = std::bind(&Boss::UpdateBattle, this);
}

void Boss::ChangeStateFollow(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::FAST_RUN);

	stateUpdate_ = std::bind(&Boss::UpdateFollow, this);
}

void Boss::ChangeStateAttrckReady(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateAttrckReady, this);
}

void Boss::ChangeStateAttrckStamp(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateAttrckStamp, this);
}

void Boss::ChangeStateAttrckLeftClaw(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateAttrckLeftClaw, this);
}

void Boss::ChangeStateAttrckRightClaw(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateAttrckRightClaw, this);
}

void Boss::ChangeStateAttrckDash(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateAttrckDash, this);
}

void Boss::ChangeStateStunned(void)
{
	stateTime_ = STUNNED_STATE_DURATION;
	stateUpdate_ = std::bind(&Boss::UpdateStunned, this);
}

void Boss::ChangeStateDamage(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateDamage, this);
}

void Boss::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Boss::UpdateDead, this);
}

#pragma endregion

#pragma region StateごとのUpdate

// stateがNONEの時のUpdate
void Boss::UpdateNone(void)
{
}

// stateがPLAYの時のUpdate
void Boss::UpdatePlay(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::RUN);

	VECTOR axis = AsoUtility::VECTOR_ZERO;
	axis.y = 1.0f;

	// 回転
	if (!AsoUtility::EqualsVZero(axis))
	{
		playerRotY_ = playerRotY_.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axis.y), AsoUtility::AXIS_Y));
	}

	// 前方向を取得
	VECTOR forward = transform_.GetForward();
	// 移動
	movePow_ = VScale(forward, SPEED_MOVE);
}

void Boss::UpdateLerpMove(void)
{
	// アニメーション再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::FAST_RUN);

	// 移動
	movePow_ = VScale(VNorm(VSub(waypoint_, transform_.pos)), SPEED_RUN);
	movePow_.y = 0.0f;

	// ターゲットに向けて回転
	TargetRotate(waypoint_);

	// プレイヤーとの衝突判定
	const VECTOR diff = VSub(transform_.pos, waypoint_);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	// 視線の先に至らに変更予定
	if (disPow < ATTRCK_RADIUS * ATTRCK_BITE_RADIUS)//攻撃半径×攻撃半径
	{
		isLerp_ = false;
	}
	else
	{
		isLerp_ = true;
	}
}

// stateがBATTLEの時のUpdate
void Boss::UpdateBattle(void)
{
	// タイマー更新
	rotateTimer_ -= SceneManager::GetInstance().GetDeltaTime(); // フレームの経過時間を使う（環境によって異なります）
	if (lerpTime_ >= 0.0f)lerpTime_ -= SceneManager::GetInstance().GetDeltaTime();

	// 一定時間ごとに回転処理
	if (rotateTimer_ <= 1.0f)
	{
		// ターゲットに向けて回転
		TargetRotate(follow_->pos, TARGET_ROT_RATE);
		if (rotateTimer_ <= 0.0f)
		{
			rotateTimer_ = ROT_INTERVAL; // リセット
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

	// 攻撃方法
	attrckTypeState_ = ATTRCK_TYPE::NONE;

	// プレイヤーとの衝突判定
	float disPow = AsoUtility::GetDisPow(transform_.pos, follow_->pos);

	// 視線の先に至らに変更予定
	if (stateTime_ < 0.0f || (IsTargetInFOV(follow_->pos, FOV_RADIUS) && stateTime_ < 1.0f)) 
	{
		if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))
		{
			if (static_cast<int>(disPow) % 2 == 0)
			{
				// 攻撃方法
				attrckTypeState_ = ATTRCK_TYPE::BITE;
			}
			else
			{
				// 攻撃方法
				attrckTypeState_ = ATTRCK_TYPE::CLOW_R;
				attrckCount_ = ATTACK_COUNT;
			}
			// 攻撃準備へ
			ChangeState(STATE::ATTRCK_READY);
		}
		// プレイヤーが索敵範囲内	// 回り続けると回転だけなので突進を絡める
		else if (disPow > DASH_RADIUS * DASH_RADIUS)
		{
			ChangeState(STATE::ATTRCK_DASH);	//接近
		}
		// プレイヤーが索敵範囲内	// 回り続けると回転だけなので突進を絡める
		else if (disPow < MOVE_RADIUS * MOVE_RADIUS)
		{
			ChangeState(STATE::FOLLOW);			//接近
		}
		else 
		{
			//ChangeState(STATE::PLAY);//バトル中止
		}
	}
}

void Boss::UpdateFollow(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN));
	animeType_ = static_cast<int>(ANIM_TYPE::FAST_RUN);

	if (IsTargetInFOV(follow_->pos,FOV_RADIUS))
	{
		//ターゲットに向けて回転
		TargetRotate(follow_->pos);
	}

	// 移動
	movePow_ = VScale(transform_.GetForward(), SPEED_FOLLOW);	

	// プレイヤーとの衝突判定
	float disPow = AsoUtility::GetDisPow(transform_.pos, follow_->pos);

	if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))
	{
		if (static_cast<int>(disPow) % 2 == 0)
		{
			//攻撃方法
			attrckTypeState_ = ATTRCK_TYPE::BITE;
		}
		else
		{
			//攻撃方法
			attrckTypeState_ = ATTRCK_TYPE::CLOW_R;
			attrckCount_ = ATTACK_COUNT;
		}

		ChangeState(STATE::ATTRCK_READY);
	}
	else if (stateTime_ < 0.0f)
	{
		ChangeState(STATE::BATTLE);
	}
}

void Boss::UpdateAttrckReady(void)
{
	switch (attrckTypeState_)
	{
	case ATTRCK_TYPE::BITE:
		animationController_->Play(static_cast<int>(ANIM_TYPE::READY_ATTRCK), false);
		animeType_ = static_cast<int>(ANIM_TYPE::READY_ATTRCK);

		if (animationController_->IsEnd())
		{
			auto& nIns = NetManager::GetInstance();

			nIns.SetAction(PLAYER_ACTION::BOSS_ATTRCK_A);
			ChangeState(STATE::ATTRCK_STAMP);
		}
		break;
	case ATTRCK_TYPE::CLOW_L:
		ChangeState(STATE::BATTLE);
		break;
	case ATTRCK_TYPE::CLOW_R:
		animationController_->Play(static_cast<int>(ANIM_TYPE::READY_ATTRCK), false);
		animeType_ = static_cast<int>(ANIM_TYPE::READY_ATTRCK);

		if (animationController_->IsEnd())
		{
			auto& nIns = NetManager::GetInstance();

			ChangeState(STATE::ATTRCK_R_CLOW);
		}
		break;
	default:
		ChangeState(STATE::BATTLE);
		break;
	}
}

void Boss::UpdateAttrckStamp(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTRCK_STAMP), false);
	animeType_ = static_cast<int>(ANIM_TYPE::ATTRCK_STAMP);

	if (animationController_->IsEnd())
	{
		ChangeState(STATE::BATTLE);
	}
}

void Boss::UpdateAttrckLeftClaw(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW), false);
	animeType_ = static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW);

	if (animationController_->IsEnd())
	{
		if (attrckCount_ > 0)
		{
			attrckCount_--;
			ChangeState(STATE::ATTRCK_R_CLOW);
			return;
		}
		else
		{
			ChangeState(STATE::BATTLE);
			return;
		}
	}
}

void Boss::UpdateAttrckRightClaw(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW), false);
	animeType_ = static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW);

	if (animationController_->IsEnd())
	{
		if (attrckCount_ > 0)
		{
			attrckCount_--;
			ChangeState(STATE::ATTRCK_L_CLOW);
			return;
		}
		else
		{
			ChangeState(STATE::BATTLE);
			return;
		}	
	}
}

void Boss::UpdateAttrckDash(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTRCK_DASH));
	animeType_ = static_cast<int>(ANIM_TYPE::ATTRCK_DASH);

	// 移動
	movePow_ = VScale(transform_.GetForward(), SPEED_RUN);

	// プレイヤーとの衝突判定
	VECTOR diff = VSub(transform_.pos, follow_->pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	// 離れていて視線にいると
	if (IsTargetInFOV(follow_->pos, FOV_RADIUS) && disPow > ATTRCK_RADIUS * ATTRCK_RADIUS)
	{
		// ターゲットに向けて回転
		TargetRotate(follow_->pos);
	}

	if (stateTime_ < 0.0f)
	{
		ChangeState(STATE::BATTLE);
	}
}

void Boss::UpdateStunned(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::STUNNED));
	animeType_ = static_cast<int>(ANIM_TYPE::STUNNED);


	if (stateTime_ < 0.0f)
	{
		if (follow_ != nullptr)ChangeState(STATE::BATTLE);
		else ChangeState(STATE::PLAY);
	}
}

void Boss::UpdateDamage(void)
{	
}

void Boss::UpdateDead(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::DEAD), false);
	animeType_ = static_cast<int>(ANIM_TYPE::DEAD);
}

#pragma endregion


void Boss::CollisionStageCapsule(void)
{
	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);
	// カプセルとの衝突判定(主にステージ)
	for (const auto& c : colliders_)
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

void Boss::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, COL_CHECK_UP_POW));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, COL_CHECK_DOWN_POW));
	for (const auto& c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > JUMP_DOT_BORDER)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, PUSH_BACK_LENGTH));

			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
		}
	}
}

// デバッグ用描画
void Boss::DrawDebug(void)
{
	// 索敵範囲
	DrawSphere3D(transform_.pos, MOVE_RADIUS, 20, 0x0000ff, 0x0000ff, false);
	DrawSphere3D(transform_.pos, ATTRCK_RADIUS, 20, 0xff0000, 0xff0000, false);
	DrawSphere3D(transform_.pos, DASH_RADIUS, 20, 0xff0000, 0x00ff00, false);
	// 攻撃範囲
	DrawSphere3D(attrckPos_, attrckRadius, 20, 0xff0000, 0xff0000, false);

	// 当たり判定
	for (const auto& part:hitParts_)
	{
		part->Draw();
	}

	DrawFormatString(100, 400, 0x000000, L"Boss_HP(%d)", hp_);
	DrawFormatString(100, 416, 0x000000, L"Boss_Attrck(%d)", attackDamage_);

	// 視野描画
	DrawFOV(FOV_RADIUS, MOVE_RADIUS, DEBUG_FOV_DIVISIONS, GetColor(255, 255, 0)); // 視野角90度、半径200、15本の線
}

void Boss::AttrckUpdate(void)
{
	animationController_->Play(attackType_, false);

	// 当たり判定が発生するか
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

void Boss::AttackDataUpdate(void)
{
	//アニメーションに応じて攻撃力設定
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_STAMP))
	{
		attackDamage_ = ATTRCK_STAMP;
	}
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_L_CLOW)
		|| animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_R_CLOW)
		)
	{
		attackDamage_ = ATTRCK_CLOW;
	}
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK_DASH))
	{
		attackDamage_ = ATTRCK_DASH;
		effectController_->LoopPlay(DASH_EFFECT);
	}
	else
	{
		effectController_->Stop(DASH_EFFECT);
	}
}
