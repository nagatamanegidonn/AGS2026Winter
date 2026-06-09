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

#include "Sword.h"

#pragma region 定数

namespace
{
	// アニメーションリスト
	const std::vector<CharaBase::AnimationInfo> ANIM_LIST =
	{
		// 通常アニメーション
		{ static_cast<int>(Player::ANIM_TYPE::IDLE), L"Axe/Idle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::RUN), L"Run.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::FAST_RUN), L"FastRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ROLL), L"Sprinting Forward Roll.mv1", 45.0f, 0, 0.0f, 32.0f },
		// 抜刀、納刀
		{ static_cast<int>(Player::ANIM_TYPE::DRAW), L"Axe/Draw Close.mv1", 30.0f, 0, 0.0f, 22.0f },
		{ static_cast<int>(Player::ANIM_TYPE::BATTLE_DRAW), L"Axe/Draw Close.mv1", 30.0f, 0, 22.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::CLOSE), L"Axe/Draw Close.mv1", 30.0f, 0, 22.0f, 0.1f },
		{ static_cast<int>(Player::ANIM_TYPE::BATTLE_CLOSE), L"Axe/Draw Close.mv1", 30.0f, 0, 37.0f, 22.0f },
		// バトル時アニメーション
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_IDLE), L"Axe/Battle Idle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_RUN), L"Axe/Standing Run Forward.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_FAST_RUN), L"Axe/Sword And Shield Run.mv1", 30.0f, -1, 0.0f, -1.0f },
		// ダメージ
		{ static_cast<int>(Player::ANIM_TYPE::DAMAGE), L"Axe/Damage.mv1", 30.0f, 0, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::FLYING), L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f },
		{ static_cast<int>(Player::ANIM_TYPE::DOWN), L"Down.mv1", 20.0f, -1, 0.0f, -1.0f },
		// 攻撃		// 下記にスペルミスがあるから修正
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK1S), L"Axe/Attrck1.mv1", 40.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK1STOP), L"Axe/Attrck1.mv1", 0.0f, 0, 0.0f, -1.0f },	// 使用無し
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK1E), L"Axe/Attrck1.mv1", 0.0f, 0, 0.0f, -1.0f },	// 使用無し
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK2), L"Axe/Attrck2.mv1", 40.0f, -1, 12.0f, -1.0f  },
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK3), L"Axe/Attrck3.mv1", 40.0f, 0, 0.0f, -1.0f },
		// 死亡
		{ static_cast<int>(Player::ANIM_TYPE::DEAD), L"Dying.mv1", 30.0f, 0, 0.0f, -1.0f },
		// 共通アニメーション
		{ static_cast<int>(Player::ANIM_TYPE::GET), L"Normal/Picking Up.mv1", 55.0f, 0, 0.0f, 210.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ITEM_THROW), L"Normal/Goalie Throw.mv1", 30.0f, 0, 30.0f, 50.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ITEM_THROW_E), L"Normal/Goalie Throw.mv1", 30.0f, 0, 50.0f, 60.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ITEM_SET), L"Normal/Tender Placement.mv1", 35.0f, 0, 40.0f, 170.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ITEM_SET_E), L"Normal/Tender Placement.mv1", 35.0f, 0, 170.0f, 250.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ITEM_DRINK), L"Normal/Drinking.mv1", 50.0f, 0, 40.0f, 160.0f },
	};

	// アクションデータリスト
	// 攻撃情報の設定
	const CharaBase::ActionData ATTRCK_ATTRCK1S_DATA = { false, -1,  20.0f, 30.0f, 32.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTACK2) };
	const CharaBase::ActionData ATTRCK_ATTRCK2_DATA = { false, -1, 25.0f, 30.0f, 32.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTACK3) };
	const CharaBase::ActionData ATTRCK_ATTRCK3_DATA = { false, -1, 25.0f, 40.0f, 60.0f,0.0f,-1 };
	// 共通データ
	const CharaBase::ActionData FLYING_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::DOWN) };
	const CharaBase::ActionData DOWN_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::IDLE) };
	const CharaBase::ActionData IDLE_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,-1 };

	// 武器カプセル関係
	constexpr VECTOR WEAPON_CAPSULE_TOP = { 0.0f, 125.0f, 0.0f };
	constexpr VECTOR WEAPON_CAPSULE_DOWN = { 0.0f, 0.0f, 0.0f };

	// サウンド発生時間
	constexpr float ATTRCK1_SE_STIME = 20.0f;
	constexpr float ATTRCK2_SE_STIME = 25.0f;
	constexpr float ATTRCK3_SE_STIME = 25.0f;
	constexpr float HALF_RATE = 0.5f;

	constexpr float BLEND_RATE_30 = 3.0f;
	constexpr float BLEND_RATE_50 = 5.0f;
	constexpr float BLEND_RATE_100 = 10.0f;
}

#pragma endregion

Sword::Sword(int key, GameScene* scene, PLAYER_TYPE type)
	:
	Player(key, scene, type)
{
}

Sword::~Sword(void)
{
}

bool Sword::IsSyncAttack()
{
	return (animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK1S)
		|| animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK2)
		|| animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK3)
		);
}

void Sword::InitParam(void)
{
	// メインウェポン
	transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 1.0f);
	// 初期座標
	transWeapon_.pos = prePos_ = { 0.0f, -30.0f, 0.0f };
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal =
		Quaternion::Euler({ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f),  AsoUtility::Deg2RadF(0.0f) });
	transWeapon_.Update();

	// サブウェポン
	// なし

	// カプセルの設定
	capsuleWeapon_ = std::make_shared<Capsule>(transWeapon_);
	capsuleWeapon_->SetLocalPosTop(WEAPON_CAPSULE_TOP);
	capsuleWeapon_->SetLocalPosDown(WEAPON_CAPSULE_DOWN);
	capsuleWeapon_->SetRadius(10.0f); 

	// 抜刀ダッシュの有無
	isBattleDash_ = true;

	// 攻撃情報の設定
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK1S), ATTRCK_ATTRCK1S_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK2), ATTRCK_ATTRCK2_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK3), ATTRCK_ATTRCK3_DATA);

	SetActionData(static_cast<int>(ANIM_TYPE::FLYING), FLYING_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::DOWN), DOWN_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::IDLE), IDLE_DATA);
}

void Sword::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add(static_cast<int>(ANIM_TYPE::IDLE), path + L"Axe/Idle.mv1", 20.0f);
	
	// アニメーションの登録
	for (const auto& anim : ANIM_LIST)
	{
		animationController_
			->Add(anim.type, path + anim.name, anim.speed, anim.loopNum, anim.startFrame, anim.endFrame);
	}

	// 従来からのブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::IDLE), true, BLEND_RATE_50);
	// 冬からの新規ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::RUN), true, BLEND_RATE_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::FAST_RUN), true, BLEND_RATE_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ROLL), true);
	// 抜刀、納刀ブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::DRAW), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BATTLE_CLOSE), true);
	// バトル時アニメーションブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_IDLE), true, BLEND_RATE_100);
	// 武器持ち移動
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_RUN), true, BLEND_RATE_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_FAST_RUN), true, BLEND_RATE_100);
	// ダメージ（共通）
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::FLYING), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::DOWN), true, BLEND_RATE_30);
	// 攻撃
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTACK1S), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTACK2), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTACK3), true);


	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
	animeType_ = static_cast<int>(ANIM_TYPE::IDLE);
}

void Sword::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();

	effectController_->Add(0, path + L"PowerUp/PowerUp.efkefc");
	effectController_->Add(1, path + L"Slash/Slash.efkefc");
	effectController_->Play(1);
}

void Sword::InitAttackSound(void)
{
	std::wstring path = Application::PATH_SOUND;

	soundController_->Add(static_cast<int>(SE::ATTACK1), path + L"Player/SwordSwing.mp3", 0.6f);
	soundController_->Add(static_cast<int>(SE::ATTACK2), path + L"Player/SwordSwing.mp3", 0.6f);
	soundController_->Add(static_cast<int>(SE::ATTACK3), path + L"Player/SowrdSlash.mp3", 0.6f);
}
void Sword::PlayAttackSound(void)
{
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTACK1S)
		&& animationController_->GetStepTime() > ATTRCK1_SE_STIME
		&& animationController_->GetStepTime() < ATTRCK1_SE_STIME + HALF_RATE)
	{
		soundController_->Play(static_cast<int>(SE::ATTACK1), Sound::TIMES::ONCE);
	}
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ATTACK2)
		&& animationController_->GetStepTime() > ATTRCK2_SE_STIME
		&& animationController_->GetStepTime() < ATTRCK2_SE_STIME + HALF_RATE)
	{
		soundController_->Play(static_cast<int>(SE::ATTACK2), Sound::TIMES::ONCE);
	}
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ATTACK3)
		&& animationController_->GetStepTime() > ATTRCK3_SE_STIME
		&& animationController_->GetStepTime() < ATTRCK3_SE_STIME + HALF_RATE)
	{
		soundController_->Play(static_cast<int>(SE::ATTACK3), Sound::TIMES::ONCE);
	}
}

void Sword::SyncWeaponPlay()
{
#pragma region 武器の同期＿非戦闘時

	// メインウェポン（背中）
	SyncWeaponToFream(L"mixamorig:Spine", SOWRD_SPINE_ROT, SOWRD_SPINE_POS,
		transform_, transWeapon_);

#pragma endregion
}
void Sword::SyncWeaponBattle()
{
#pragma region 武器の同期（戦闘時）

	// メインウェポン（右手）
	SyncWeaponToFream(L"mixamorig:RightHandMiddle1", SOWRD_HAND_ROT, SOWRD_HAND_POS,
		transform_, transWeapon_);

#pragma endregion
}

