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

#pragma region 定数

namespace
{
	const VECTOR START_POS = VECTOR{ 0.0f, -30.0f, 0.0f };
	const VECTOR START_LOCAL_ROT
		= VECTOR{ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f),  AsoUtility::Deg2RadF(0.0f) };

	// アニメーションリスト
	const std::vector<CharaBase::AnimationInfo> ANIM_LIST =
	{
		// 通常アニメーション
		{ static_cast<int>(Player::ANIM_TYPE::IDLE), L"Axe/Idle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::RUN), L"Run.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::FAST_RUN), L"FastRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ROLL), L"Sprinting Forward Roll.mv1", 45.0f, 0, 0.0f, 32.0f },
		// 抜刀、納刀
		{ static_cast<int>(Player::ANIM_TYPE::DRAW), L"Arrow/Draw.mv1", 20.0f, 0, 0.0f, 8.0f },
		{ static_cast<int>(Player::ANIM_TYPE::BATTLE_DRAW), L"Arrow/Draw.mv1", 20.0f, 0, 8.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::CLOSE), L"Arrow/Draw.mv1", 20.0f, 0, 8.0f, 0.1f },
		{ static_cast<int>(Player::ANIM_TYPE::BATTLE_CLOSE), L"Arrow/Draw.mv1", 30.0f, 0, 27.0f, 8.0f },
		// バトル時アニメーション
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_IDLE), L"Arrow/BattleIdle.mv1", 20.0f, -1, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::BTLLE_RUN), L"Arrow/BattleRun.mv1", 30.0f, -1, 0.0f, -1.0f },
		// ダメージ
		{ static_cast<int>(Player::ANIM_TYPE::DAMAGE), L"Axe/Damage.mv1", 30.0f, 0, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::FLYING), L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f },
		{ static_cast<int>(Player::ANIM_TYPE::DOWN), L"Down.mv1", 20.0f, -1, 0.0f, -1.0f },
		// 攻撃
		{ static_cast<int>(Player::ANIM_TYPE::ATTRCK1S), L"Arrow/AttrckS.mv1", 20, 0, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ATTRCK1STOP), L"Arrow/AttrckC.mv1", 20, 0, 0.0f, -1.0f },
		{ static_cast<int>(Player::ANIM_TYPE::ATTRCK1E), L"Arrow/AttrckE.mv1", 20, 0, 0.0f, -1.0f },
		// 2段攻撃
		{ static_cast<int>(Player::ANIM_TYPE::ATTRCK2), L"Attrck2.mv1", 30.0f, 0, 0.0f, -1.0f },
		// 3段攻撃
		{ static_cast<int>(Player::ANIM_TYPE::ATTRCK3), L"Attrck3.mv1", 30.0f, 0, 0.0f, -1.0f },
		// 死亡
		{ static_cast<int>(Player::ANIM_TYPE::DEAD), L"Dying.mv1",30.0f, 0, 0.0f, -1.0f },
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
		= { true, static_cast<int>(Player::ANIM_TYPE::ATTRCK1STOP), -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTRCK1E) };
	const CharaBase::ActionData ATTRCK_ATTRCK1E_DATA = { false, -1, -1.0f, -1.0f,4.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::ATTRCK1S), };
	// 共通データ
	const CharaBase::ActionData FLYING_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::DOWN) };
	const CharaBase::ActionData DOWN_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,static_cast<int>(Player::ANIM_TYPE::IDLE) };
	const CharaBase::ActionData IDLE_DATA = { false, -1, -1.0f, -1.0f,-1.0f,0.0f,-1 };

	constexpr float BLEND_RATE_30 = 3.0f;
	constexpr float BLEND_RATE_50 = 5.0f;
	constexpr float BLEND_RATE_100 = 10.0f;
}

#pragma endregion

Arrow::Arrow(int key, GameScene* scene, PLAYER_TYPE type)
	:
	Player(key, scene, type)
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
	transWeapon_.pos = prePos_ = START_POS;
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal =
		Quaternion::Euler(START_LOCAL_ROT);
	transWeapon_.Update();

	// サブウェポン
	const std::wstring PATH_MDL = Application::PATH_MODEL;
	transSubWeapon_.modelId = MV1LoadModel((PATH_MDL + L"Weapon/Bow/Arrow.mv1").c_str());
	transSubWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 2.0f);
	// 初期座標
	transSubWeapon_.pos = prePos_ = START_POS;
	transSubWeapon_.quaRot = Quaternion();
	transSubWeapon_.quaRotLocal = Quaternion();
	transSubWeapon_.Update();

	// カプセルの設定
	capsuleWeapon_ = std::make_shared<Capsule>(transWeapon_);
	capsuleWeapon_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsuleWeapon_->SetLocalPosDown({ 0.0f, -30.0f, 0.0f });
	capsuleWeapon_->SetRadius(10.0f);

	// 抜刀ダッシュの有無
	isBattleDash_ = false;

	// 攻撃情報の設定
	SetActionData(static_cast<int>(ANIM_TYPE::ATTRCK1S), ATTRCK_ATTRCK1S_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::ATTRCK1E), ATTRCK_ATTRCK1E_DATA);

	SetActionData(static_cast<int>(ANIM_TYPE::FLYING), FLYING_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::DOWN), DOWN_DATA);
	SetActionData(static_cast<int>(ANIM_TYPE::IDLE), IDLE_DATA);
}

void Arrow::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーションの登録
	for (const auto& anim : ANIM_LIST)
	{
		animationController_
			->Add(anim.type, path + anim.name, anim.speed, anim.loopNum, anim.startFrame, anim.endFrame);
	}

	// 待機ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::IDLE), true, BLEND_RATE_50);
	// 冬からの新規ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::RUN), true, BLEND_RATE_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::FAST_RUN), true, BLEND_RATE_100);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ROLL), true);
	// 抜刀、納刀ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::DRAW), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BATTLE_CLOSE), true);
	// 戦闘時待機ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_IDLE), true, BLEND_RATE_100);
	// 武器持ち移動ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::BTLLE_RUN), true, BLEND_RATE_100);
	// ダメージブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::FLYING), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::DOWN), true, BLEND_RATE_30);
	// 攻撃ブレンド
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTRCK1S), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTRCK2), true);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ATTRCK3), true);
	
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
	animeType_ = static_cast<int>(ANIM_TYPE::IDLE);
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

	soundController_->Add(static_cast<int>(SE::ATTRCK1), path + L"Player/Bow1.mp3", 0.6f);
	soundController_->Add(static_cast<int>(SE::ATTRCK2), path + L"Player/BowShot.mp3", 0.6f);
}
void Arrow::PlayAttrckSound(void)
{
	// 弾の発射
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK1E)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ATTRCK1E)
		)
	{
		// フレームの取得
		int frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:RightHand");
		if (frmNo == -1) {
			// エラー処理またはログ出力
			return;
		}
		soundController_->Play(static_cast<int>(SE::ATTRCK2), Sound::TIMES::ONCE);

		// 手の位置とグローバルマトリクスを取得
		const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);
		gameScene_->CreateShot(ShotBase::TYPE::ARROW,attrckDamage_
			, posHand, transform_.GetForward(), key_);
	}

	// 弦を引っ張る音
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK1STOP)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ATTRCK1S)
		)
	{
		soundController_->Play(static_cast<int>(SE::ATTRCK1), Sound::TIMES::ONCE);
	}
}

void Arrow::DrawWeapon()
{
	auto& nIns = NetManager::GetInstance();
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK1S)
		||animeType_ == static_cast<int>(ANIM_TYPE::ATTRCK1STOP)
		)
	{
		MV1DrawModel(transSubWeapon_.modelId);
	}

	MV1DrawModel(transWeapon_.modelId);
}

void Arrow::SyncWeaponPlay()
{
	// 武器の同期＿非戦闘時

	// メインウェポン（背中）
	SyncWeaponToFream(L"mixamorig:Spine", BOW_SPINE_ROT, BOW_SPINE_POS,
		transform_, transWeapon_);
}
void Arrow::SyncWeaponBattle()
{
	// 武器の同期（戦闘時）

	// メインウェポン（左手）
	SyncWeaponToFream(L"mixamorig:LeftHandMiddle1", BOW_LHAND_ROT, BOW_LHAND_POS,
		transform_, transWeapon_);

	// サブウェポン（右手）
	SyncWeaponToFream(L"mixamorig:RightHand", ARROW_RHAND_ROT, ARROW_RHAND_POS,
		transform_, transSubWeapon_);
}

