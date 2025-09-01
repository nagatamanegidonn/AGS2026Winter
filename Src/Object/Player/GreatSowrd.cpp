#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Common/Vector2.h"

#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"

#include "../Manager/Camera.h"

#include "../Net/NetManager.h"
#include "../Scene/GameScene.h"

#include "../Common/Capsule.h"
#include "../Common/Collider.h"

#include "../Common/AnimationController.h"
#include "../Common/InputController.h"
#include "../Common/EffectController.h"
#include "../Common/SoundController.h"

#include "GreatSowrd.h"

GreatSowrd::GreatSowrd(int key) :Player(key)
{
}

GreatSowrd::~GreatSowrd(void)
{
}

void GreatSowrd::InitPram(void)
{
	//メインウェポン
	transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 1.0f);
	// 初期座標
	transWeapon_.pos = prePos_ = { 0.0f, -30.0f, 0.0f };
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal =
		Quaternion::Euler({ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f),  AsoUtility::Deg2RadF(0.0f) });
	transWeapon_.Update();

	//サブウェポン
	//なし

	isBattleDash_ = true;


	atkData_.emplace((int)ANIM_TYPE::ATTRCK1S, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK2, 20.0f, 30.0f, 32.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK2, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK3, 25.0f, 30.0f, 32.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK3, std::move(SetAtrckData(-1, 25.0f, 40.0f, 60.0f)));

	atkData_.emplace((int)ANIM_TYPE::FLYING, std::move(SetAtrckData((int)ANIM_TYPE::DOWN)));
	atkData_.emplace((int)ANIM_TYPE::DOWN, std::move(SetAtrckData((int)ANIM_TYPE::BLEND_IDLE)));
	atkData_.emplace((int)ANIM_TYPE::BLEND_IDLE, std::move(SetAtrckData(-1)));
}

void GreatSowrd::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::IDLE, path + L"Axe/Idle.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::BLEND_IDLE, path + L"Axe/Idle.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BLEND_IDLE, true, 3.0f);
	
	//移動系（共通）
	animationController_->Add((int)ANIM_TYPE::RUN, path + L"Run.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + L"FastRun.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::ROLL, path + L"Sprinting Forward Roll.mv1", 45.0f, -1, 0.0f, 32.0f);
	//抜刀
	animationController_->Add((int)ANIM_TYPE::DRAW, path + L"Axe/Draw Close.mv1", 30.0f, 0, 0.0f, 22.0f);
	animationController_->Add((int)ANIM_TYPE::BATTLE_DRAW, path + L"Axe/Draw Close.mv1", 30.0f, 0, 22.0f);
	//納刀
	animationController_->Add((int)ANIM_TYPE::CLOSE, path + L"Axe/Draw Close.mv1", 30.0f, 0, 22.0f, 0.1f);
	animationController_->Add((int)ANIM_TYPE::BATTLE_CLOSE, path + L"Axe/Draw Close.mv1", 30.0f, 0, 37.0f, 22.0f);

	animationController_->SetIsBlend((int)ANIM_TYPE::DRAW, true);
	animationController_->SetIsBlend((int)ANIM_TYPE::BATTLE_CLOSE, true);

	//攻撃
	animationController_->Add((int)ANIM_TYPE::BTLLE_IDLE, path + L"Axe/Battle Idle.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_IDLE, true, 10.0f);

	//武器持ち移動
	animationController_->Add((int)ANIM_TYPE::BTLLE_RUN, path + L"Axe/Standing Run Forward.mv1", 30.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_RUN, true, 10.0f);

	animationController_->Add((int)ANIM_TYPE::BTLLE_FAST_RUN, path + L"Axe/Sword And Shield Run.mv1", 30.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_FAST_RUN, true, 10.0f);

	//ダメージ
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + L"Axe/Damage.mv1", 30.0f);
	//吹っ飛び（共通）
	animationController_->Add((int)ANIM_TYPE::FLYING, path + L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f);
	animationController_->SetIsBlend((int)ANIM_TYPE::FLYING, true);

	animationController_->Add((int)ANIM_TYPE::DOWN, path + L"Down.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::DOWN, true, 3.0f);


	animationController_->Add((int)ANIM_TYPE::ATTRCK1S, path + L"Axe/Attrck1.mv1", 40.0f, -1);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK1S, true);

	//animationController_->Add((int)ANIM_TYPE::ATTRCK1STOP, path + L"Great Sword Slash (1).mv1", 20.0f, -1, 13.0f, 13.0f);
	//animationController_->Add((int)ANIM_TYPE::ATTRCK1E, path + L"Great Sword Slash (1).mv1", 20.0f, -1, 13.0f);

	animationController_->Add((int)ANIM_TYPE::ATTRCK2, path + L"Axe/Attrck2.mv1", 40.0f, -1, 12.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK2, true);

	animationController_->Add((int)ANIM_TYPE::ATTRCK3, path + L"Axe/Attrck3.mv1", 40.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK3, true);
	//死亡（共通）
	animationController_->Add((int)ANIM_TYPE::DEAD, path + L"Dying.mv1", 30.0f);
	
	animationController_->Play((int)ANIM_TYPE::IDLE);
	animeType_ = (int)ANIM_TYPE::IDLE;
}

void GreatSowrd::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();

	effectController_->Add(0, path + L"PowerUp/PowerUp.efkefc");
}

void GreatSowrd::InitAttrckSound(void)
{
	std::wstring path = Application::PATH_SOUND;

	soundController_->Add((int)SE::ATTRCK1, path + L"Player/SwordSwing.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK2, path + L"Player/SwordSwing.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK3, path + L"Player/SowrdSlash.mp3", 0.6f);
}
void GreatSowrd::PlayAttrckSound(void)
{
	if (animeType_ == (int)ANIM_TYPE::ATTRCK1S
		&& animationController_->GetStepTime() > 20.0f
		&& animationController_->GetStepTime() < 20.5f)
	{
		soundController_->Play((int)SE::ATTRCK1, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK2
		&& animationController_->GetStepTime() > 25.0f
		&& animationController_->GetStepTime() < 25.5f)
	{
		soundController_->Play((int)SE::ATTRCK2, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK3
		&& animationController_->GetStepTime() > 25.0f
		&& animationController_->GetStepTime() < 25.5f)
	{
		soundController_->Play((int)SE::ATTRCK3, Sound::TIMES::ONCE);
	}
}

void GreatSowrd::SyncWeaponPlay()
{
#pragma region 武器の同期＿非戦闘時

	//auto frmNo = MV1SearchFrame(transform_.modelId, "mixamorig1:Spine2");//マネキン
	//auto frmNo = MV1SearchFrame(transform_.modelId, "mixamorig:Spine2");//ナイト
	auto frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:Spine");//ナイト腰
	//プレイヤーの手の位置
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	// プレイヤーの手のグローバルマトリクス
	//MATRIX mat = MV1GetFrameLocalMatrix(transform_.modelId, frmNo);
	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	// 手のワールド回転
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	//参照元の大きさを考慮
	auto scl = transform_.scl;
	scl.x = 1.0f / scl.x;
	scl.y = 1.0f / scl.y;
	scl.z = 1.0f / scl.z;

	//回転だけを取り出す（大きさも出るので考慮）
	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	//回転
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_R, handRot))//X回転
				//, AsoUtility::Deg2RadF(demoRot_.x))
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_U, handRot))//Y回転
				//, AsoUtility::Deg2RadF(demoRot_.y))
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_F, handRot))//Z回転
				//, AsoUtility::Deg2RadF(demoRot_.z))
			, AsoUtility::Deg2RadF(-120.0f))
	);

	// 最終回転（手 + 握り補正）
	transWeapon_.matRot = mixMat;

	// 最終位置
	transWeapon_.pos = posHand;
	//transformItem_.pos = VAdd(transformItem_.pos, VScale(transformItem_.quaRot.GetRight(), 50.0f));//武器が斜めではないなら消す
	//transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetDown(), demoRot_.x));
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetForward(), 15.0f));
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetRight(), -30.0f));

	//// 最終回転（手 + 握り補正）
	//transWeapon_.quaRot = handWorldRot.Mult(transform_.quaRot);

	//// モデルの更新
	transWeapon_.Update(true);

#pragma endregion
}
void GreatSowrd::SyncWeaponBattle()
{
#pragma region 武器の同期（戦闘時）

	//auto frmNo = MV1SearchFrame(transform_.modelId, "mixamorig1:RightHandMiddle1");//マネキン
	auto frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:RightHandMiddle1");//ナイト背中
	//プレイヤーの手の位置
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	// プレイヤーの手のグローバルマトリクス
	//MATRIX mat = MV1GetFrameLocalMatrix(transform_.modelId, frmNo);
	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	// 手のワールド回転
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	//参照元の大きさを考慮
	auto scl = transform_.scl;
	scl.x = 1.0f / scl.x;
	scl.y = 1.0f / scl.y;
	scl.z = 1.0f / scl.z;

	//回転だけを取り出す（大きさも出るので考慮）
	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	//回転
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_R, handRot))//X回転
				//, AsoUtility::Deg2RadF(demoRot_.x))
			, AsoUtility::Deg2RadF(15.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_U, handRot))//Y回転
				//, AsoUtility::Deg2RadF(demoRot_.y))
			, AsoUtility::Deg2RadF(50.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_F, handRot))//Z回転
				//, AsoUtility::Deg2RadF(demoRot_.z))
			, AsoUtility::Deg2RadF(-80.0f))
	);

	// 最終回転（手 + 握り補正）
	//transformItem_.quaRot = Quaternion::GetRotation(mixMat);
	transWeapon_.matRot = mixMat;
	// 最終位置
	transWeapon_.pos = VAdd(posHand, VScale(handWorldRot.GetBack(), 3.0f));

	// モデルの更新
	//transformItem_.Update();
	transWeapon_.Update(true);

#pragma endregion
}

