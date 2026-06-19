#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/Utility.h"
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

#pragma region 定数

namespace
{
	// パス・リソース関係
	const std::wstring DIR_PATH_PLAYER2 = L"Player2/";
	const std::wstring EFFECT_FILE_POWERUP = L"PowerUp/PowerUp.efkefc";
	const std::wstring EFFECT_FILE_SLASH = L"Slash/Slash.efkefc";
	const std::wstring SOUND_FILE_GS_SE = L"Player/GreatSowrd.mp3";
	// 同期用ボーン名
	const std::wstring BONE_SPINE2 = L"mixamorig:Spine2";
	const std::wstring BONE_RIGHT_HAND = L"mixamorig:RightHandMiddle1";
	// アニメーションリスト
	const std::vector<CharaBase::AnimationInfo> ANIM_LIST =
	{
		{ static_cast<int>(Player::ANIM_TYPE::IDLE), L"Idle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::RUN), L"Run.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::FAST_RUN), L"FastRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ROLL), L"Sprinting Forward Roll.mv1", 45.0f, 0, 0.0f, 32.0f },
		// 抜刀、納刀
		{ static_cast<int>(Player::ANIM_TYPE::DRAW), L"Draw Great Sword 1.mv1", 30.0f, 0, 0.0f, -1.0f},
		{ static_cast<int>(Player::ANIM_TYPE::BATTLE_DRAW), L"Draw Great Sword 2.mv1", 30.0f, 0, 0.0f, -1.0f},
		{ static_cast<int>(Player::ANIM_TYPE::CLOSE), L"Draw Great Sword 1.mv1", 30.0f, -1, 13.0f, 0.1f},
		{ static_cast<int>(Player::ANIM_TYPE::BATTLE_CLOSE), L"Draw Great Sword 2.mv1", 30.0f, -1, 24.0f, 0.1f},
		// バトル時アニメーション
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_IDLE), L"BattleIdle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_RUN), L"BattleRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		// ダメージ
		{ static_cast<int>(Player::ANIM_TYPE::DAMAGE), L"Great Sword Impact.mv1", 30.0f, 0, 0.0f, 24.0f },
		{ static_cast<int>(Player::ANIM_TYPE::FLYING), L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f },
		{ static_cast<int>(Player::ANIM_TYPE::DOWN), L"Down.mv1", 20.0f, -1, 0.0f, -1.0f },
		// 攻撃
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK1S), L"Great Sword Slash (1).mv1", 20.0f, -1, 0.0f, 13.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK1STOP), L"Great Sword Slash (1).mv1", 20.0f, -1, 13.0f, 13.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK1E), L"Great Sword Slash (1).mv1", 20.0f, -1, 13.0f ,-1.0f},
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK2), L"Great Sword Slash (2).mv1", 30.0f, -1, 10.0f ,-1.0f},
		{ static_cast<int>(Player::ANIM_TYPE::ATTACK3), L"Great Sword Casting.mv1", 40.0f, 0, 0.0f, -1.0f },
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
	const CharaBase::ActionData ATTRCK_ATTRCK1S_DATA
		= { true, static_cast<int>(Player::ANIM_TYPE::ATTACK1STOP), -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTACK1E) };
	const CharaBase::ActionData ATTRCK_ATTRCK1E_DATA = { false, -1, 16.0f, 24.0f, 22.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTACK2) };
	const CharaBase::ActionData ATTRCK_ATTRCK2_DATA = { false, -1, 21.0f, 40.0f, 38.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTACK3) };
	const CharaBase::ActionData ATTRCK_ATTRCK3_DATA = { false, -1, 60.0f, 78.0f, 80.0f,0.0f,-1 };
	// 共通データ
	const CharaBase::ActionData FLYING_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::DOWN) };
	const CharaBase::ActionData DOWN_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::IDLE) };
	const CharaBase::ActionData IDLE_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,-1 };	
	// 武器カプセル関係
	constexpr VECTOR WEAPON_CAPSULE_TOP = { 0.0f, 200.0f, 0.0f };
	constexpr VECTOR WEAPON_CAPSULE_DOWN = { 0.0f, 10.0f, 0.0f };
	constexpr float CAP_RADIUS = 10.0f;
	constexpr float WEAPON_SCALE = 2.0f;
	// サウンド発生時間
	constexpr float ATTRCK1_SE_STIME = 16.0f;
	constexpr float ATTRCK2_SE_STIME = 21.0f;
	constexpr float ATTRCK3_SE_STIME = 60.0f;
	constexpr float SOUND_VOLUME = 0.6f;
	constexpr float HALF_RATE = 0.5f;
	// アニメーションブレンド関係
	constexpr float BLEND_SPEED_30 = 3.0f;
	constexpr float BLEND_SPEED_50 = 5.0f;
	constexpr float BLEND_SPEED_100 = 10.0f;
}

#pragma endregion

GreatSword::GreatSword(int key, GameScene* scene, PLAYER_TYPE type)
	:
	Player(key, scene, type)
{
}

GreatSword::~GreatSword(void)
{
}

bool GreatSword::IsSyncAttack()
{
	return (animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK1E)
		|| animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK2)
		|| animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK3)
		);
}

void GreatSword::InitParam(void)
{
	// メインウェポン
	transWeapon_.scl = VScale(Utility::VECTOR_ONE, WEAPON_SCALE);
	// 初期座標
	transWeapon_.pos = prePos_ = Utility::VECTOR_ZERO;
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal =
		Quaternion::Euler({ Utility::Deg2RadF(WEPON_LOCAL_ROT.x), Utility::Deg2RadF(WEPON_LOCAL_ROT.y), Utility::Deg2RadF(WEPON_LOCAL_ROT.z) });
	transWeapon_.Update();

	// サブウェポン
	// なし

	// カプセルの設定
	capsuleWeapon_ = std::make_shared<Capsule>(transWeapon_);
	capsuleWeapon_->SetLocalPosTop(WEAPON_CAPSULE_TOP);
	capsuleWeapon_->SetLocalPosDown(WEAPON_CAPSULE_DOWN);
	capsuleWeapon_->SetRadius(CAP_RADIUS);

	// 抜刀ダッシュの有無
	isBattleDash_ = false;

	// 攻撃情報の設定
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK1S), ATTRCK_ATTRCK1S_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK1E), ATTRCK_ATTRCK1E_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK2), ATTRCK_ATTRCK2_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTACK3), ATTRCK_ATTRCK3_DATA);

	SetActionData(static_cast<int>(ANIM_TYPE::FLYING), FLYING_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::DOWN), DOWN_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::IDLE), IDLE_DATA);
}

void GreatSword::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + DIR_PATH_PLAYER2;
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーションの登録
	for (const auto& anim : ANIM_LIST)
	{
		animationController_
			->Add(anim.type, path + anim.name, anim.speed, anim.loopNum, anim.startFrame, anim.endFrame);
	}

	// 待機のブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::IDLE), true, BLEND_SPEED_50);
	// 走りのブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::RUN), true, BLEND_SPEED_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::FAST_RUN), true, BLEND_SPEED_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ROLL), true);
	// 抜刀納刀のブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::DRAW), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BATTLE_CLOSE), true);
	// 戦闘待機のブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_IDLE), true, BLEND_SPEED_100);
	// 戦闘走りのブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_RUN), true, BLEND_SPEED_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_FAST_RUN), true, BLEND_SPEED_100);
	// ダメージのブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::FLYING), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::DOWN), true, BLEND_SPEED_30);
	// 攻撃のブレンド設定
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTACK1S), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTACK2), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTACK3), true);

	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
	animeType_ = static_cast<int>(ANIM_TYPE::IDLE);
	animeAgoType_ = animeType_;
}

void GreatSword::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();
	// チャージアニメーション
	effectController_->Add(POWER_UP_EFFECT, path + EFFECT_FILE_POWERUP);

	effectController_->Add(POWER_SLASH_EFFECT, path + EFFECT_FILE_SLASH);
	effectController_->Play(POWER_SLASH_EFFECT);
}

void GreatSword::InitAttackSound(void)
{
	std::wstring path = Application::PATH_SOUND;

	soundController_->Add(static_cast<int>(SE::ATTACK1), path + SOUND_FILE_GS_SE, SOUND_VOLUME);
	soundController_->Add(static_cast<int>(SE::ATTACK2), path + SOUND_FILE_GS_SE, SOUND_VOLUME);
	soundController_->Add(static_cast<int>(SE::ATTACK3), path + SOUND_FILE_GS_SE, SOUND_VOLUME);
}

void GreatSword::PlayAttackSound(void)
{
	if (animeType_ == (int)ANIM_TYPE::ATTACK1E
		&& animationController_->GetStepTime() > ATTRCK1_SE_STIME
		&& animationController_->GetStepTime() < ATTRCK1_SE_STIME + HALF_RATE)
	{
		soundController_->Play((int)SE::ATTACK1, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTACK2
		&& animationController_->GetStepTime() > ATTRCK2_SE_STIME
		&& animationController_->GetStepTime() < ATTRCK2_SE_STIME + HALF_RATE)
	{
		soundController_->Play((int)SE::ATTACK2, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTACK3
		&& animationController_->GetStepTime() > ATTRCK3_SE_STIME
		&& animationController_->GetStepTime() < ATTRCK3_SE_STIME + HALF_RATE)
	{
		soundController_->Play((int)SE::ATTACK3, Sound::TIMES::ONCE);
	}
}

void GreatSword::SyncWeaponPlay()
{
#pragma region 武器の同期＿非戦闘時

	// メインウェポン（腰）
	SyncWeaponToFream(BONE_SPINE2.c_str(), GSOWRD_SPINE_ROT, GSOWRD_SPINE_POS,
		transform_, transWeapon_);

#pragma endregion
}

void GreatSword::SyncWeaponBattle()
{
#pragma region 武器の同期（戦闘時）

	// メインウェポン（右手）
	SyncWeaponToFream(BONE_RIGHT_HAND.c_str(), GSOWRD_HAND_ROT, GSOWRD_HAND_POS,
		transform_, transWeapon_);

#pragma endregion
}

