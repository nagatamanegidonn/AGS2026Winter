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

#include "HitDamage.h"
#include "HitPart.h"
#include "Boss.h"

Boss::Boss(int key)
{
	key_ = key;
	createNo_ = 0;

	animationController_ = nullptr;
	state_ = STATE::NONE;

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

	//攻撃情報
	attrckCount_ = 0;
	attrckTypeState_ = ATTRCK_TYPE::NONE;
	attrckDamage_ = 0;
	//追跡対象
	follow_ = nullptr;
	followTime_ = 0.0f;
	//攻撃位置
	attrckPos_ = AsoUtility::VECTOR_ZERO;
	dameRate_ = 1.0f;

	//当たり判定
	hitParts_.clear();
	AddHitPart(transform_.modelId, L"Chest_M",ATTRCK_BITE_RADIUS, 1.0f);//胴体
	AddHitPart(transform_.modelId, L"Root_M",90.0f,1.5f);//お尻
	AddHitPart(transform_.modelId, L"Spine2_M", 120.0f, 1.2f);//腰
	AddHitPart(transform_.modelId, L"Tongue1_M",120.0f, 1.0f);//胸
	//左前足
	AddHitPart(transform_.modelId, L"Shoulder_L", 75.0f, 0.5f);
	AddHitPart(transform_.modelId, L"ElbowPart1_L", 75.0f, 0.5f);
	//左前足
	AddHitPart(transform_.modelId, L"Shoulder_R", 75.0f, 0.5f);
	AddHitPart(transform_.modelId, L"ElbowPart1_R", 75.0f, 0.5f);
	//左後足
	AddHitPart(transform_.modelId, L"Hip_L", 75.0f, 1.0f);
	AddHitPart(transform_.modelId, L"Knee_L", 75.0f, 1.0f);
	//左後足
	AddHitPart(transform_.modelId, L"Hip_R", 75.0f, 1.0f);
	AddHitPart(transform_.modelId, L"Knee_R", 75.0f, 1.0f);

	lerpTime_ = MAX_LERP_TIME;
	lerpPos_ = AsoUtility::VECTOR_ZERO;
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
	transform_.pos = prePos_ = { 0.0f, -30.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
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

	// カプセルコライダ
	/*capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 30.0f, 0.0f });
	capsule_->SetRadius(20.0f);*/

	auto& nIns = NetManager::GetInstance();
	auto& users = NetManager::GetInstance().GetNetUsers();

	// ＨＰの初期化
	hp_ = hpMax_ = MAX_HP * users.size();

	isHitCheck_ = false;

	transform_.MakeCollider(Collider::TYPE::WALL);
	hitDamePos_ = AsoUtility::VECTOR_ZERO;
}

void Boss::Update(void)
{
	auto& nIns = NetManager::GetInstance();

	auto& users = NetManager::GetInstance().GetNetUsers();

	animeAgoType_ = animeType_;

	//ダメージ処理
	int dame = 0;
	for (auto& user : users)
	{
		const int userDame = nIns.GetNetBossDamage(user.first);

		dame += nIns.GetNetBossDamage(user.first);
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

	//アニメーションに応じて攻撃力設定
	if (animeType_ == (int)ANIM_TYPE::ATTRCK_STAMP)
	{
		attrckDamage_ = ATTRCK_STAMP;
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK_L_CLOW
		|| animeType_ == (int)ANIM_TYPE::ATTRCK_R_CLOW)
	{
		attrckDamage_ = ATTRCK_CLOW;
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK_DASH)
	{
		attrckDamage_ = ATTRCK_DASH;
		effectController_->LoopPlay(0);
	}
	else
	{
		effectController_->Stop(0);
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
		nIns.SetNetBossHp(key_, hp_);

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
		if(isDebug) stateUpdate_();

		// 重力による移動量
		CalcGravityPow();

		// 衝突判定
		Collision();

		if (stateTime_ >= 0.0f)stateTime_ -= SceneManager::GetInstance().GetDeltaTime();


		// 重力方向に沿って回転させる
		transform_.quaRot = Quaternion();//grvMng_がないので代わりに
		transform_.quaRot = transform_.quaRot.Mult(playerRotY_);


		// 位置送信もここでOK（ProcessMove内でも呼ばれてるけど念のため）
		nIns.SetBoss(key_, transform_.pos, transform_.quaRot, animeType_, (int)state_);
	}
	//通信プレイヤーの処理
	else
	{
		//HPの同期
		hp_ = nIns.GetNetBossHp(key_);

		const MONSTER_DATA& boss = nIns.GetBoss(key_);

		animeType_ = boss.Anim_;
		state_ = static_cast<Boss::STATE>(boss.state_);

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

	//ダッシュエフェクト
	effectController_->LoopUpdate(0, transform_.pos, transform_.rot, 50.0f);
	effectController_->Update(1);



	//音の再生
	const auto& selfPos = nIns.GetPostion(nIns.GetSelf().key);
	//音量設定
	float volume = AsoUtility::CalcVolumeByDistance(selfPos, transform_.pos, (MOVE_RADIUS + MOVE_RADIUS));

	// 無音なら停止
	soundController_->ChengeVolume(0, volume); // ボリュームだけ更新
	soundController_->ChengeVolume(1, volume); // ボリュームだけ更新
	soundController_->ChengeVolume(2, volume); // ボリュームだけ更新


	if (animeType_ == (int)ANIM_TYPE::ATTRCK_STAMP
		&& animationController_->GetStepTime() > 23.0f
		&& animationController_->GetStepTime() < 23.5f)
	{
		soundController_->Play(2, Sound::TIMES::ONCE);
	}
	else if (
		(animeType_ == (int)ANIM_TYPE::ATTRCK_L_CLOW
		&& animeAgoType_ != (int)ANIM_TYPE::ATTRCK_L_CLOW)||
		(animeType_ == (int)ANIM_TYPE::ATTRCK_R_CLOW
			&& animeAgoType_ != (int)ANIM_TYPE::ATTRCK_R_CLOW)
		)
	{
		soundController_->Play(1, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK_DASH
		&& animeAgoType_ != (int)ANIM_TYPE::ATTRCK_DASH)
	{
		soundController_->Play(0, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::DEAD
		&& animeAgoType_ != (int)ANIM_TYPE::DEAD)
	{
		SceneManager::GetInstance().CaptureMainScreen();
	}


	//ダメージ描画の更新
	for (auto& hitdamage : hitdamages_)
	{
		hitdamage->Update();
	}

	nIns.SetNetBossDamage(nIns.GetSelf().key, 0);

	// 描画用の位置は、Draw()でNetManagerから取るからOK
	transform_.Update();

	// アニメーション再生
	animationController_->Update();
	if (animationController_->IsEnd() && animeType_ == (int)ANIM_TYPE::DEAD)
	{
		// ゲームの勝敗判定
		SceneManager::GAME_RESULT result = SceneManager::GAME_RESULT::GAME_CLEAR;
		SceneManager::GetInstance().SetGameResult(result);
	}

}
void Boss::Draw(void)
{

	// カメラクリップ外になったら描画しない
	/*if (!CheckCameraViewClip(transform_.pos))
	{*/
	// モデルの描画
	MV1DrawModel(transform_.modelId);

	//ダメージの表記
	for (auto& hitdamage : hitdamages_)
	{
		hitdamage->Draw();
	}


#ifdef _DEBUG

	DrawFormatString(100, 400, 0x000000, L"Boss_Anim(%d)", animeType_);
	DrawFormatString(100, 416, 0x000000, L"Boss_State(%d)", state_);
	DrawFormatString(100, 432, 0x000000, L"Boss_Area(%d)", GetAreaId());

	// デバッグ用
	DrawDebug();
	DrawFOV(FOV_RADIUS, MOVE_RADIUS, 15, GetColor(255, 255, 0)); // 視野角90度、半径200、15本の線
#endif
}



void Boss::Damage(int dama)
{
	//無敵中はない

	effectController_->Play(1, hitDamePos_, { 0.0f,0.0f,0.0f }, 7.5f);

	auto& nIns = NetManager::GetInstance();

	const int lastDame = dama * dameRate_;

	nIns.SetNetBossDamage(nIns.GetSelf().key, lastDame);

}

////武器との当たり判定
//const bool Boss::CollisionCapsule(int& modelId)
//{
//	// ０番目のフレームのコリジョン情報を更新する
//	MV1RefreshCollInfo(modelId, -1);
//
//	//当たり判定フラグ
//	bool ret = false;
//	VECTOR hitPos = AsoUtility::VECTOR_ZERO;
//
//	for (auto& part : hitParts_)
//	{
//		// カプセルとの衝突判定
//		auto hits = MV1CollCheck_Sphere(
//			modelId, -1,
//			part->GetPos(), part->GetRadius());
//		
//		// 衝突した複数のポリゴンと衝突回避するまで、
//		// プレイヤーのdamage
//		 // 当たったかどうかで処理を分岐
//		if (hits.HitNum >= 1)
//		{
//
//			// 当たった場合は衝突の情報を描画する
//			ret = true;
//			dameRate_ = part->GetDameRate();
//
//			// 最初に衝突したポリゴンのインデックス
//			const auto& poly = hits.Dim[0];
//
//			// 頂点の平均を取ってポリゴンの中心を求める
//			hitPos = {
//				(poly.Position[0].x + poly.Position[1].x + poly.Position[2].x) / 3.0f,
//				(poly.Position[0].y + poly.Position[1].y + poly.Position[2].y) / 3.0f,
//				(poly.Position[0].z + poly.Position[1].z + poly.Position[2].z) / 3.0f
//			};
//
//			// 必要であれば保存しておく（例：メンバ変数 hitPos_ に）
//			hitDamePos_ = hitPos;
//
//			// 検出した地面ポリゴン情報の後始末
//			MV1CollResultPolyDimTerminate(hits);
//			return ret;
//		}
//
//		// 検出した地面ポリゴン情報の後始末
//		MV1CollResultPolyDimTerminate(hits);
//	}
//
//	return ret;
//}
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
	/*if (animeType_==(int)ANIM_TYPE::ATTRCK_BITE)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK_BITE;
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, "TongueEnd_M");//牙攻撃用
		attrckRadius = ATTRCK_BITE_RADIUS;
	}*/
	if (animeType_==(int)ANIM_TYPE::ATTRCK_STAMP)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK_STAMP;
		attrckPos_ = VScale(VAdd(AsoUtility::MV1GetFreamPos(transform_.modelId, L"Fingers1_L")
			, AsoUtility::MV1GetFreamPos(transform_.modelId, L"Fingers1_R")), 0.5f);
		attrckRadius = ATTRCK_STAMP_RADIUS;
	}
	else if (animeType_==(int)ANIM_TYPE::ATTRCK_L_CLOW)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK_L_CLOW;
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, L"Fingers1_L");
		attrckRadius = ATTRCK_BITE_RADIUS;
	}
	else if (animeType_==(int)ANIM_TYPE::ATTRCK_R_CLOW)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK_R_CLOW;
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, L"Fingers1_R");
		attrckRadius = ATTRCK_BITE_RADIUS;
	}
	else if (animeType_==(int)ANIM_TYPE::ATTRCK_DASH)
	{
		attrckType_ = (int)ANIM_TYPE::ATTRCK_R_CLOW;
		attrckPos_ = AsoUtility::MV1GetFreamPos(transform_.modelId, L"Chest_M");
		attrckRadius = ATTRCK_DASH_RADIUS;
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


void Boss::SetLerpPos(VECTOR pos)
{
	lerpPos_ = pos;
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

	std::wstring path = Application::PATH_MODEL + L"Enemy/Boss/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + L"Boss.mv1", 20.0f, 0);
	animationController_->Add((int)ANIM_TYPE::RUN, path + L"Boss.mv1", 30.0f, 6);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + L"Boss.mv1", 30.0f, 2);

	animationController_->Add((int)ANIM_TYPE::READY_ATTRCK, path + L"Boss.mv1", 1.2f, 12, 0.0f, 1.5f);
	//animationController_->Add((int)ANIM_TYPE::ATTRCK_BITE, path + L"Boss.mv1", 30.0f, 9);
	animationController_->Add((int)ANIM_TYPE::ATTRCK_STAMP, path + L"Boss.mv1", 25.0f, 10);
	animationController_->Add((int)ANIM_TYPE::ATTRCK_L_CLOW, path + L"Boss.mv1", 20.0f, 8);
	animationController_->Add((int)ANIM_TYPE::ATTRCK_R_CLOW, path + L"Boss.mv1", 20.0f, 5);
	animationController_->Add((int)ANIM_TYPE::ATTRCK_DASH, path + L"Boss.mv1", 40.0f, 2);
	
	animationController_->Add((int)ANIM_TYPE::STUNNED, path + L"Boss.mv1", 30.0f, 14);
	animationController_->Add((int)ANIM_TYPE::DEAD, path + L"Boss.mv1", 30.0f, 13);

	//atkData_.emplace((int)ANIM_TYPE::ATTRCK_BITE, std::move(SetAtrckData(-1, 4.0f, 15.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK_STAMP, std::move(SetAtrckData(-1, 17.0f, 24.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK_L_CLOW, std::move(SetAtrckData(-1, 9.0f, 17.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK_R_CLOW, std::move(SetAtrckData(-1, 9.0f, 17.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK_DASH, std::move(SetAtrckData(-1, 0.0f, 22.5f)));

	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_IDLE, true, 5.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK_STAMP, true, 1.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::STUNNED, true, 3.0f);

	//animationController_->Play((int)ANIM_TYPE::IDLE);
	animationController_->Play((int)ANIM_TYPE::RUN);
	animeType_ = (int)ANIM_TYPE::RUN;
	animeAgoType_ = animeType_;
}
void Boss::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();


	effectController_->Add(0, path + L"Dash/Dash.efkefc");
	effectController_->Add(1, path + L"Damage/Damage.efkefc");
}
void Boss::InitSound(void)
{
	std::wstring path = Application::PATH_SOUND;
	soundController_ = std::make_unique<SoundController>();

	soundController_->Add(0, path + L"Boss/Dash.mp3", 1.0f);
	soundController_->Add(1, path + L"Boss/Clow.mp3", 1.0f);
	soundController_->Add(2, path + L"Boss/Stamp.mp3", 2.0f);

}

#pragma region StateによるUpdateの切り替え

void Boss::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	stateTime_ = 3.0f;

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
	rotateTimer_ = rotateInterval_; // リセット
	stateUpdate_ = std::bind(&Boss::UpdateBattle, this);
}
void Boss::ChangeStateFollow(void)
{
	animationController_->Play((int)ANIM_TYPE::FAST_RUN);
	animeType_ = (int)ANIM_TYPE::FAST_RUN;

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
	stateTime_ = 4.0f;
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

//stateがNONEの時のUpdate
void Boss::UpdateNone(void)
{
}
//stateがPLAYの時のUpdate
void Boss::UpdatePlay(void)
{
	animationController_->Play((int)ANIM_TYPE::RUN);
	animeType_ = (int)ANIM_TYPE::RUN;

	VECTOR axis = AsoUtility::VECTOR_ZERO;
	axis.y = 1.0f;

	//回転
	if (!AsoUtility::EqualsVZero(axis)){
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
void Boss::UpdateLerpMove(void)
{
	animationController_->Play((int)ANIM_TYPE::FAST_RUN);
	animeType_ = (int)ANIM_TYPE::FAST_RUN;

	movePow_ =
		VScale(VNorm(VSub(lerpPos_, transform_.pos)), SPEED_RUN);
	movePow_.y = 0.0f;

	//ターゲットに向けて回転
	TargetRotate(lerpPos_, 0.3f);

	// playerとの衝突判定
	const VECTOR diff = VSub(transform_.pos, lerpPos_);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	//視線の先に至らに変更予定
	if (disPow < ATTRCK_RADIUS * ATTRCK_BITE_RADIUS)//攻撃半径×攻撃半径
	{
		isLerp_ = false;
	}
	else
	{
		isLerp_ = true;
	}
}
//stateがBATTLEの時のUpdate
void Boss::UpdateBattle(void)
{

	// タイマー更新
	rotateTimer_ -= SceneManager::GetInstance().GetDeltaTime(); // フレームの経過時間を使う（環境によって異なります）
	if (lerpTime_ >= 0.0f)lerpTime_ -= SceneManager::GetInstance().GetDeltaTime();

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


	//攻撃方法
	attrckTypeState_ = ATTRCK_TYPE::NONE;

	// playerとの衝突判定
	float disPow = AsoUtility::GetDisPow(transform_.pos, follow_->pos);

	//視線の先に至らに変更予定
	if (stateTime_ < 0.0f || (IsTargetInFOV(follow_->pos, FOV_RADIUS) && stateTime_ < 1.0f)) {

		if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))//攻撃半径×攻撃半径
		{
			if ((int)disPow % 2 == 0)
			{
				//攻撃方法
				attrckTypeState_ = ATTRCK_TYPE::BITE;
			}
			else
			{
				//攻撃方法
				attrckTypeState_ = ATTRCK_TYPE::CLOW_R;
				attrckCount_ = 3;
			}

			ChangeState(STATE::ATTRCK_READY);
		}
		//プレイヤーが索敵範囲内	//回り続けると回転だけなので突進を絡める
		else if (disPow > DASH_RADIUS * DASH_RADIUS) {
			ChangeState(STATE::ATTRCK_DASH);//接近
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
void Boss::UpdateFollow(void)
{
	animationController_->Play((int)ANIM_TYPE::FAST_RUN);
	animeType_ = (int)ANIM_TYPE::FAST_RUN;

	if (IsTargetInFOV(follow_->pos,FOV_RADIUS))
	{
		//ターゲットに向けて回転
		TargetRotate(follow_->pos);
	}

	// 移動
	movePow_ =
		VScale(transform_.GetForward(), SPEED_FOLLOW);

	// playerとの衝突判定
	float disPow = AsoUtility::GetDisPow(transform_.pos, follow_->pos);

	if (disPow < ATTRCK_RADIUS * ATTRCK_RADIUS && IsTargetInFOV(follow_->pos, FOV_RADIUS))//ダメージ半径×攻撃半径
	{
		if ((int)disPow % 2 == 0)
		{
			//攻撃方法
			attrckTypeState_ = ATTRCK_TYPE::BITE;
		}
		else
		{
			//攻撃方法
			attrckTypeState_ = ATTRCK_TYPE::CLOW_R;
			attrckCount_ = 3;
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
		animationController_->Play((int)ANIM_TYPE::READY_ATTRCK, false);
		animeType_ = (int)ANIM_TYPE::READY_ATTRCK;

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
		animationController_->Play((int)ANIM_TYPE::READY_ATTRCK, false);
		animeType_ = (int)ANIM_TYPE::READY_ATTRCK;

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
	animationController_->Play((int)ANIM_TYPE::ATTRCK_STAMP, false);
	animeType_ = (int)ANIM_TYPE::ATTRCK_STAMP;

	if (animationController_->IsEnd())
	{
		ChangeState(STATE::BATTLE);
	}


	auto& nIns = NetManager::GetInstance();
	nIns.SetAction(PLAYER_ACTION::BOSS_ATTRCK_A);

	if (nIns.IsAction(key_, PLAYER_ACTION::BOSS_ATTRCK_A))
	{

	}

}
void Boss::UpdateAttrckLeftClaw(void)
{

	animationController_->Play((int)ANIM_TYPE::ATTRCK_L_CLOW, false);
	animeType_ = (int)ANIM_TYPE::ATTRCK_L_CLOW;

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
	animationController_->Play((int)ANIM_TYPE::ATTRCK_R_CLOW, false);
	animeType_ = (int)ANIM_TYPE::ATTRCK_R_CLOW;

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
	animationController_->Play((int)ANIM_TYPE::ATTRCK_DASH);
	animeType_ = (int)ANIM_TYPE::ATTRCK_DASH;

	

	// 移動
	movePow_ =
		VScale(transform_.GetForward(), SPEED_RUN);

	// playerとの衝突判定
	VECTOR diff = VSub(transform_.pos, follow_->pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	/*if (IsTargetInFOV(270.0f))
	{

	}*/

	//離れていて視線にいると
	if (IsTargetInFOV(follow_->pos,FOV_RADIUS) && disPow > ATTRCK_RADIUS * ATTRCK_RADIUS)
	{
		//ターゲットに向けて回転
		TargetRotate(follow_->pos);
	}

	if (stateTime_ < 0.0f)
	{
		ChangeState(STATE::BATTLE);
	}
}
void Boss::UpdateStunned(void)
{
	animationController_->Play((int)ANIM_TYPE::STUNNED);
	animeType_ = (int)ANIM_TYPE::STUNNED;


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
	animationController_->Play((int)ANIM_TYPE::DEAD, false);
	animeType_ = (int)ANIM_TYPE::DEAD;


	if (animationController_->IsEnd())
	{
		
	}
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

// デバッグ用描画
void Boss::DrawDebug(void)
{
	// カプセルコライダ
	//capsule_->Draw();

	//索敵範囲
	DrawSphere3D(transform_.pos, MOVE_RADIUS, 20, 0x0000ff, 0x0000ff, false);
	DrawSphere3D(transform_.pos, ATTRCK_RADIUS, 20, 0xff0000, 0xff0000, false);
	DrawSphere3D(transform_.pos, DASH_RADIUS, 20, 0xff0000, 0x00ff00, false);
	//攻撃範囲
	DrawSphere3D(attrckPos_, attrckRadius, 20, 0xff0000, 0xff0000, false);

	//当たり判定
	for (const auto& part:hitParts_)
	{
		part->Draw();
	}

	DrawFormatString(100, 400, 0x000000, L"Boss_HP(%d)", hp_);
	DrawFormatString(100, 416, 0x000000, L"Boss_Attrck(%d)", attrckDamage_);
}


void Boss::AttrckUpdate(void)
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
