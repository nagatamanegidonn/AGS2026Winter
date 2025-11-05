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

#include "../Common/Collider/Capsule.h"
#include "../Common/Collider/Collider.h"

#include "../Common/AnimationController.h"
#include "../Common/InputController.h"
#include "../Common/EffectController.h"
#include "../Common/SoundController.h"

#include "GreatSword.h"

GreatSword::GreatSword(int key) :Player(key)
{
}

GreatSword::~GreatSword(void)
{
}

void GreatSword::InitParam(void)
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
	atkData_.emplace((int)ANIM_TYPE::DOWN, std::move(SetAtrckData((int)ANIM_TYPE::IDLE)));
	atkData_.emplace((int)ANIM_TYPE::IDLE, std::move(SetAtrckData(-1)));
	/*atkData_.emplace((int)ANIM_TYPE::DOWN, std::move(SetAtrckData((int)ANIM_TYPE::BLEND_IDLE)));
	atkData_.emplace((int)ANIM_TYPE::BLEND_IDLE, std::move(SetAtrckData(-1)));*/
}

void GreatSword::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::IDLE, path + L"Axe/Idle.mv1", 20.0f);
	//animationController_->Add((int)ANIM_TYPE::BLEND_IDLE, path + L"Axe/Idle.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::IDLE, true, 5.0f);
	
	//移動系（共通）
	animationController_->Add((int)ANIM_TYPE::RUN, path + L"Run.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + L"FastRun.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::ROLL, path + L"Sprinting Forward Roll.mv1", 45.0f, -1, 0.0f, 32.0f);

	//冬からの新規ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::RUN, true, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::FAST_RUN, true, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ROLL, true);


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

void GreatSword::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();

	effectController_->Add(0, path + L"PowerUp/PowerUp.efkefc");
	effectController_->Add(1, path + L"Slash/Slash.efkefc");
	effectController_->Play(1);

}

void GreatSword::InitAttrckSound(void)
{
	std::wstring path = Application::PATH_SOUND;

	soundController_->Add((int)SE::ATTRCK1, path + L"Player/SwordSwing.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK2, path + L"Player/SwordSwing.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK3, path + L"Player/SowrdSlash.mp3", 0.6f);
}
void GreatSword::PlayAttrckSound(void)
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

void GreatSword::SyncWeaponPlay()
{
#pragma region 武器の同期＿非戦闘時

	// メインウェポン（背中）
	SyncWeaponToFream(L"mixamorig:Spine", SOWRD_SPINE_ROT, SOWRD_SPINE_POS,
		transform_, transWeapon_);

#pragma endregion
}
void GreatSword::SyncWeaponBattle()
{
#pragma region 武器の同期（戦闘時）

	// メインウェポン（右手）
	SyncWeaponToFream(L"mixamorig:RightHandMiddle1", SOWRD_HAND_ROT, SOWRD_HAND_POS,
		transform_, transWeapon_);

#pragma endregion
}

