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

#include "Arrow.h"

namespace
{
	// アニメーションリスト
	const std::vector<CharaBase::AnimationInfo> animList =
	{
		{ (int)Player::ANIM_TYPE::IDLE, L"Axe/Idle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::RUN, L"Run.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::FAST_RUN, L"FastRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::ROLL, L"Sprinting Forward Roll.mv1", 45.0f, 0, 0.0f, 32.0f },
		// 抜刀、納刀
		{ (int)Player::ANIM_TYPE::DRAW, L"Arrow/Draw.mv1", 20.0f, 0, 0.0f, 8.0f },
		{ (int)Player::ANIM_TYPE::BATTLE_DRAW, L"Arrow/Draw.mv1", 20.0f, 0, 8.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::CLOSE, L"Arrow/Draw.mv1", 20.0f, 0, 8.0f, 0.1f },
		{ (int)Player::ANIM_TYPE::BATTLE_CLOSE, L"Arrow/Draw.mv1", 30.0f, 0, 27.0f, 8.0f },
		// バトル時アニメーション
		{ (int)Player::ANIM_TYPE::BTLLE_IDLE, L"Arrow/BattleIdle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::BTLLE_RUN, L"Arrow/BattleRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		// ダメージ
		{ (int)Player::ANIM_TYPE::DAMAGE, L"Axe/Damage.mv1", 30.0f, 0, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::FLYING, L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f },
		{ (int)Player::ANIM_TYPE::DOWN, L"Down.mv1", 20.0f, -1, 0.0f, -1.0f },
		// 攻撃
		{ (int)Player::ANIM_TYPE::ATTRCK1S, L"Arrow/AttrckS.mv1", 20, 0, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::ATTRCK1STOP, L"Arrow/AttrckC.mv1", 20, 0, 0.0f, -1.0f },
		{ (int)Player::ANIM_TYPE::ATTRCK1E, L"Arrow/AttrckE.mv1", 20, 0, 0.0f, -1.0f },
		// 2段攻撃
		{ (int)Player::ANIM_TYPE::ATTRCK2, L"Attrck2.mv1", 30.0f, 0, 0.0f, -1.0f },
		// 3段攻撃
		{ (int)Player::ANIM_TYPE::ATTRCK3, L"Attrck3.mv1", 30.0f, 0, 0.0f, -1.0f },
		// 死亡
		{ (int)Player::ANIM_TYPE::DEAD, L"Dying.mv1",30.0f, 0, 0.0f, -1.0f },	
	};
}

Arrow::Arrow(int key) :Player(key)
{
}

Arrow::~Arrow(void)
{
	MV1DeleteModel(transSubWeapon_.modelId);
}

bool Arrow::IsSyncAttrck()
{
	return false;
}

void Arrow::InitParam(void)
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
	const std::wstring PATH_MDL = Application::PATH_MODEL;
	transSubWeapon_.modelId = MV1LoadModel((PATH_MDL + L"Weapon/Bow/Arrow.mv1").c_str());
	transSubWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 2.0f);
	// 初期座標
	transSubWeapon_.pos = prePos_ = { 0.0f, -30.0f, 0.0f };
	transSubWeapon_.quaRot = Quaternion();
	transSubWeapon_.quaRotLocal = Quaternion();
	transSubWeapon_.Update();

	isBattleDash_ = false;

	atkData_.emplace((int)ANIM_TYPE::ATTRCK1S, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK1E, -1.0f, -1.0f
		, -1.0f, true, (int)ANIM_TYPE::ATTRCK1STOP)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK1E, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK1S, -1.0f, -1.0f, 4.0f)));
	
	atkData_.emplace((int)ANIM_TYPE::FLYING, std::move(SetAtrckData((int)ANIM_TYPE::DOWN)));
	atkData_.emplace((int)ANIM_TYPE::DOWN, std::move(SetAtrckData((int)ANIM_TYPE::IDLE)));
	atkData_.emplace((int)ANIM_TYPE::IDLE, std::move(SetAtrckData(-1)));
}

void Arrow::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	//// 待機
	//animationController_->Add((int)ANIM_TYPE::IDLE, path + L"Axe/Idle.mv1", 20.0f);
	//// 移動系（共通）
	//animationController_->Add((int)ANIM_TYPE::RUN, path + L"Run.mv1", 30.0f);
	//animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + L"FastRun.mv1", 30.0f);
	//animationController_->Add((int)ANIM_TYPE::ROLL, path + L"Sprinting Forward Roll.mv1", 45.0f, -1, 0.0f, 32.0f);
	//// 抜刀
	//animationController_->Add((int)ANIM_TYPE::DRAW, path + L"Arrow/Draw.mv1", 20.0f, 0, 0.0f, 8.0f);
	//animationController_->Add((int)ANIM_TYPE::BATTLE_DRAW, path + L"Arrow/Draw.mv1", 20.0f, 0, 8.0f);
	//// 納刀
	//animationController_->Add((int)ANIM_TYPE::CLOSE, path + L"Arrow/Draw.mv1", 20.0f, 0, 8.0f, 0.1f);
	//animationController_->Add((int)ANIM_TYPE::BATTLE_CLOSE, path + L"Arrow/Draw.mv1", 30.0f, 0, 27.0f, 8.0f);
	//// 戦闘時待機
	//animationController_->Add((int)ANIM_TYPE::BTLLE_IDLE, path + L"Arrow/BattleIdle.mv1", 20.0f);
	//// 武器持ち移動
	//animationController_->Add((int)ANIM_TYPE::BTLLE_RUN, path + L"Arrow/BattleRun.mv1", 30.0f);
	//// ダメージ
	//animationController_->Add((int)ANIM_TYPE::DAMAGE, path + L"Axe/Damage.mv1", 30.0f);
	//// 吹っ飛び（共通）
	//animationController_->Add((int)ANIM_TYPE::FLYING, path + L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f);
	//animationController_->Add((int)ANIM_TYPE::DOWN, path + L"Down.mv1", 20.0f);
	//// 攻撃
	//animationController_->Add((int)ANIM_TYPE::ATTRCK1S, path + L"Arrow/AttrckS.mv1", 20.0f);
	//animationController_->Add((int)ANIM_TYPE::ATTRCK1STOP, path + L"Arrow/AttrckC.mv1", 20.0f);
	//animationController_->Add((int)ANIM_TYPE::ATTRCK1E, path + L"Arrow/AttrckE.mv1", 20.0f);
	//// 通常攻撃２
	//animationController_->Add((int)ANIM_TYPE::ATTRCK2, path + L"Axe/Attrck2.mv1", 40.0f);
	//// 通常攻撃３
	//animationController_->Add((int)ANIM_TYPE::ATTRCK3, path + L"Axe/Attrck3.mv1", 40.0f);
	//// 死亡（共通）
	//animationController_->Add((int)ANIM_TYPE::DEAD, path + L"Dying.mv1", 30.0f);

	for (const auto& anim : animList)
	{
		animationController_
			->Add(anim.type, path + anim.name, anim.speed, anim.loopNum, anim.startFrame, anim.endFrame);
	}

	// 待機ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::IDLE, true, 5.0f);
	// 冬からの新規ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::RUN, true, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::FAST_RUN, true, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ROLL, true);	
	// 抜刀、納刀ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::DRAW, true);
	animationController_->SetIsBlend((int)ANIM_TYPE::BATTLE_CLOSE, true);
	// 戦闘時待機ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_IDLE, true, 10.0f);
	// 武器持ち移動ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_RUN, true, 10.0f);
	// ダメージブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::FLYING, true);
	animationController_->SetIsBlend((int)ANIM_TYPE::DOWN, true, 3.0f);
	// 攻撃ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK1S, true);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK2, true);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK3, true);
	
	animationController_->Play((int)ANIM_TYPE::IDLE);
	animeType_ = (int)ANIM_TYPE::IDLE;
}

void Arrow::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();

	effectController_->Add(0, path + L"PowerUp/PowerUp.efkefc");
	effectController_->Add(1, path + L"Slash/Slash.efkefc");
	effectController_->Play(1);

}

void Arrow::InitAttrckSound(void)
{
	std::wstring path = Application::PATH_SOUND;

	soundController_->Add((int)SE::ATTRCK1, path + L"Player/Bow1.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK2, path + L"Player/BowShot.mp3", 0.6f);
}
void Arrow::PlayAttrckSound(void)
{
	// 弾の発射
	if (animeType_ == (int)ANIM_TYPE::ATTRCK1E
		&& animeAgoType_ != (int)ANIM_TYPE::ATTRCK1E)
	{
		// フレームの取得
		int frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:RightHand");
		if (frmNo == -1) {
			// エラー処理またはログ出力
			return;
		}
		soundController_->Play((int)SE::ATTRCK2, Sound::TIMES::ONCE);

		// 手の位置とグローバルマトリクスを取得
		const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);
		gameScene_->CreateShot(ShotBase::TYPE::ARROW,attrckDamage_
			, posHand, transform_.GetForward(), key_);
	}

	// 弦を引っ張る音
	if (animeType_ == (int)ANIM_TYPE::ATTRCK1STOP
		&& animeAgoType_ != (int)ANIM_TYPE::ATTRCK1S)
	{
		soundController_->Play((int)SE::ATTRCK1, Sound::TIMES::ONCE);
	}
	
}

void Arrow::DrawWeapon()
{
	auto& nIns = NetManager::GetInstance();
	if (animeType_ == (int)ANIM_TYPE::ATTRCK1S
		||animeType_ == (int)ANIM_TYPE::ATTRCK1STOP	
		)
	{
		MV1DrawModel(transSubWeapon_.modelId);
	}

	MV1DrawModel(transWeapon_.modelId);
}

void Arrow::SyncWeaponPlay()
{
#pragma region 武器の同期＿非戦闘時

	// メインウェポン（背中）
	SyncWeaponToFream(L"mixamorig:Spine", BOW_SPINE_ROT, BOW_SPINE_POS,
		transform_, transWeapon_);

#pragma endregion
}
void Arrow::SyncWeaponBattle()
{
#pragma region 武器の同期（戦闘時）

	// メインウェポン（左手）
	SyncWeaponToFream(L"mixamorig:LeftHandMiddle1", BOW_LHAND_ROT, BOW_LHAND_POS,
		transform_, transWeapon_);

	// サブウェポン（右手）
	SyncWeaponToFream(L"mixamorig:RightHand", ARROW_RHAND_ROT, ARROW_RHAND_POS,
		transform_, transSubWeapon_);

#pragma endregion
}

