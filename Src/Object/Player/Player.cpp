#include <string>
#include "../Application.h"
#include "../Utility/Utility.h"
#include "../Common/Vector2.h"

#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"

#include "../Manager/Camera.h"

#include "../Net/NetManager.h"
#include "../Scene/GameScene.h"

#include "../Common/Collider/Capsule.h"
#include "../Common/Collider/Collider.h"

#include "../Common/AnimationController.h"
#include "../Common/InputController.h"
#include "../Common/EffectController.h"
#include "../Common/SoundController.h"

#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "../Stage/Planet.h"

#include "../Item/ItemBase.h"
#include "../Item/ItemPoach.h"

#include "Player.h"

namespace
{
	// ステータスの描画位置、サイズ
	constexpr int PRAM_SIZE_X = 100;
	constexpr int PRAM_SIZE_Y = 30;
	constexpr int PRAM_POS_X = 60;
	constexpr int PRAM_POS_Y = 150;
	constexpr int PRAM_distance_Y = PRAM_SIZE_Y + 4;
	constexpr float FLING_COUNT_RATE = 2.0f;
	constexpr float FLING_MOVE_RATE = 0.3f;
	// HPバーのサイズ
	constexpr int HP_SIZE_X = 512;
	constexpr int HP_SIZE_Y = 16;
	// 武器ID
	constexpr int SWORD_ID = 0;
	constexpr int GREAT_SWORD_ID = 1;
	constexpr int BOW_ID = 2;
	// 武器ダメージ
	constexpr int SWORD_DAMAGE = 10;
	constexpr int GREAT_SWORD_DAMAGE = 20;
	constexpr int BOW_DAMAGE = 5;
	// 初期座標
	constexpr VECTOR START_POS = { 1200.0f,0.0f,-5000.0f };
	// ゲージ関連
	constexpr int GAGE_X_BASE_OFFSET = 60;	 // ゲージの基準X座標
	constexpr int GAGE_X_ALIGN_OFFSET = 40;	 // ゲージの追加Xオフセット (アイコン幅など)
	constexpr int GAGE_Y_BASE_OFFSET = 60;	 // ゲージの基準Y座標 (画面上部からの距離)
	constexpr int GAGE_Y_HALF_GAP = 2;		 // HPとSTAの間の隙間を計算するためのオフセット
	// HP、スタミナ表示位置
	constexpr int GAGE_POS_X = GAGE_X_BASE_OFFSET + GAGE_X_ALIGN_OFFSET + HP_SIZE_X / 2;
	constexpr int GAGE_POS_Y = GAGE_Y_BASE_OFFSET - GAGE_Y_HALF_GAP - HP_SIZE_Y / 2;
	constexpr int STA_POS_Y = GAGE_Y_BASE_OFFSET + GAGE_Y_HALF_GAP + HP_SIZE_Y / 2;
	// 画像関連
	const std::wstring PATH_IMAGE_FREAM = Application::PATH_IMAGE + L"Fream.png";
	const std::wstring PATH_IMAGE_PRAM_HP = Application::PATH_IMAGE + L"hp.png";
	const std::wstring PATH_IMAGE_HP_FREAM = Application::PATH_IMAGE + L"UI/hpFream.png";
	const std::wstring PATH_IMAGE_HP_MASK = Application::PATH_IMAGE + L"UI/hpMask.png";
	const std::wstring PATH_IMAGE_STAMINA_FREAM = Application::PATH_IMAGE + L"UI/staFream.png";
	const std::wstring PATH_IMAGE_STAMINA_MASK = Application::PATH_IMAGE + L"UI/staMask.png";
	const std::wstring PATH_IMAGE_SOWRD_ICON = Application::PATH_IMAGE + L"Job/Sowrd.png";
	const std::wstring PATH_IMAGE_GREATSOWRD_ICON = Application::PATH_IMAGE + L"Job/GreatSowrd.png";
	const std::wstring PATH_IMAGE_ARROW_ICON = Application::PATH_IMAGE + L"Job/Arrow.png";
	// カプセルの初期値
	constexpr VECTOR CAP_LOACL_TOP = { 0.0f, 110.0f, 0.0f };
	constexpr VECTOR CAP_LOACL_DOWN = { 0.0f, 30.0f, 0.0f };
	constexpr float CAP_RADIUS = 20.0f;
	// アイテム関連
	constexpr int START_BOM_HOLD = 2;
	constexpr int START_FLASH_HOLD = 3;
	const std::wstring ITEM_TYPE_FLASH = L"攻撃";
	const std::wstring ITEM_TYPE_HEEL = L"回復";
	const std::wstring ITEM_TYPE_BOM = L"設置";
	// アニメーション関連
	constexpr float BLEND_SPEED = 5.0f;
	// 衝突、判定関係
	constexpr float JUMP_DOT_BORDER = 0.9f;
	constexpr float COL_CHECK_UP_POW = 10.0f * 2.0f;
	constexpr float COL_CHECK_DOWN_POW = 10.0f;
	constexpr float PUSH_BACK_LENGTH = 2.0f;
	// アニメーションリスト
	std::wstring path = Application::PATH_SOUND;
	const std::vector<CharaBase::SoundData> SOUND_LIST =
	{
		{static_cast<int>(Player::SE::CHAGE), path + L"Player/Chage.mp3", 1.0f},
		{static_cast<int>(Player::SE::DRAW), path + L"Weapon/Draw.mp3", 0.6f},
		{static_cast<int>(Player::SE::CLOSE), path + L"Weapon/Close.mp3", 0.6f},
		{static_cast<int>(Player::SE::WALK), path + L"Player/Walk.mp3", 0.6f},
		{static_cast<int>(Player::SE::RUN), path + L"Player/Run.mp3", 0.6f},
		{static_cast<int>(Player::SE::ROLL), path + L"Player/Roll.mp3", 0.6f},
		{static_cast<int>(Player::SE::DAMAGE), path + L"Player/Damage.mp3", 0.6f},
		{static_cast<int>(Player::SE::HI_DAMAGE), path + L"Player/HiDamage.mp3", 0.6f},
		{static_cast<int>(Player::SE::DOWN), path + L"Player/Down.mp3.mp3", 0.6f},
	};
	// ボーン関連
	const std::wstring RIGHT_SPINE_BONE = L"mixamorig:Spine2";
	const std::wstring RIGHT_HAND_BONE = L"mixamorig:RightHandMiddle1";
	const std::wstring RIGHT_ARROW_HAND_BONE = L"mixamorig:RightHand";
	// ヒットストップ関連
	constexpr float HITSTOP_TIME = 0.2f;
	const CharaBase::ShaderData SHADER_STATUS = { L"PlayerStatus.cso", 2,{{ 0.3f, 0.7f, 0.5f, 0.05f },{ 1.0f, 1.0f, 1.0f, 1.0f }} };
	const CharaBase::ShaderData SHADER_HP = { L"PlayerHp.cso", 2,{{ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }} };
	const CharaBase::ShaderData SHADER_STAMINA = { L"PlayerHp.cso", 2,{{ 1.0f, 0.9f,0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }} };
	// 回転関係
	constexpr float ROT_DIFF = 0.1f;
}

MATRIX GetFrameGlobalMatrix(int modelHandle, int frameIndex)
{
	// ローカル行列を取得
	MATRIX localMat = MV1GetFrameLocalMatrix(modelHandle, frameIndex);
	int parentIndex = MV1GetFrameParent(modelHandle, frameIndex);
	if (parentIndex == -1)
	{
		return localMat; // ルートなのでそのまま返す
	}
	// 親フレームのグローバル行列を取得
	MATRIX parentMat = GetFrameGlobalMatrix(modelHandle, parentIndex);
	return MMult(localMat, parentMat); // DxLibの行列乗算
}

Player::Player(int key, GameScene* scene, PLAYER_TYPE type)
	:
	CharaBase(),
	type_(type),
	key_(key),
	itemId_(-1),
	isHit_(false),
	walkTime_(0.0f),
	isBattle_(false),
	isDrawWeapon_(false),
	isCloseWeapon_(false),
	hp_(0),
	hpAgo_(0),
	hpMax_(0),
	damage_(0),
	stamina_(0.0f),
	staminaMax_(0.0f),
	staminaDir_(0.0f),
	stateChanges_(),
	isBattleDash_(false),
	uiImgs_(),
	statusMaterial_(nullptr),
	statusRenderer_(nullptr),
	hpMaterial_(nullptr),
	hpRenderer_(nullptr),
	staMaterial_(nullptr),
	staRenderer_(nullptr),
	transWeapon_(),
	animationController_(nullptr),
	inputController_(nullptr),
	effectController_(nullptr),
	soundController_(nullptr),
	state_(STATE::NONE),
	animeType_(0),
	animeAgoType_(0),
	gameScene_(scene),
	chageCount_(0.0f),
	downTime_(0.0f),
	flyigDir_({0.0f, 0.0f, 0.0f}),
	flyigTime_(0.0f), 
	gravHitPosDown_({0.0f, 0.0f, 0.0f}),
	gravHitPosUp_({0.0f, 0.0f, 0.0f}),
	invisibleTime_(0.0f), 
	isBreak_(false)
{
	soundController_.reset();
	soundController_ = nullptr;

	// 状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&Player::ChangeStateBattle, this));
	stateChanges_.emplace(STATE::WEAPON, std::bind(&Player::ChangeStateWeapon, this));
	stateChanges_.emplace(STATE::ATTACK, std::bind(&Player::ChangeStateAttack, this));
	stateChanges_.emplace(STATE::ROWLING, std::bind(&Player::ChangeStateRowling, this));
	stateChanges_.emplace(STATE::DAMAGE, std::bind(&Player::ChangeStateDamage, this));
	stateChanges_.emplace(STATE::HI_DAMAGE, std::bind(&Player::ChangeStateHiDamage, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Player::ChangeStateDead, this));
	stateChanges_.emplace(STATE::GET, std::bind(&Player::ChangeStateGet, this));
	stateChanges_.emplace(STATE::ITEM_PLAY, std::bind(&Player::ChangeStateItemUse, this));
}

Player::~Player(void)
{
	// 画像の解放
	for (const auto& img : uiImgs_)
	{
		DeleteGraph(img.second);
	}

	// モデルの解放
	MV1DeleteModel(transform_.modelId);
	MV1DeleteModel(transWeapon_.modelId);
}

void Player::Init(void)
{
	// 移動速度の初期化
	speed_ = 0.0f;

	// モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::PLAYER_NIGHT));
	transform_.scl = Utility::VECTOR_ONE;	
	transform_.pos = START_POS;
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({
		Utility::Deg2RadF(PLAYER_LOCAL_ROT.x),
		Utility::Deg2RadF(PLAYER_LOCAL_ROT.y),
		Utility::Deg2RadF(PLAYER_LOCAL_ROT.z) });
	transform_.Update();

	// ステータスUI
	uiImgs_.emplace(UI_IMG_TYPE::FREAM, LoadGraph(PATH_IMAGE_FREAM.c_str()));
	uiImgs_.emplace(UI_IMG_TYPE::HP, LoadGraph(PATH_IMAGE_PRAM_HP.c_str()));

	// HP
	uiImgs_.emplace(UI_IMG_TYPE::HP_FREAM, LoadGraph(PATH_IMAGE_HP_FREAM.c_str()));
	uiImgs_.emplace(UI_IMG_TYPE::HP_MASK, LoadGraph(PATH_IMAGE_HP_MASK.c_str()));

	// スタミナ
	uiImgs_.emplace(UI_IMG_TYPE::STA_FREAM, LoadGraph(PATH_IMAGE_STAMINA_FREAM.c_str()));
	uiImgs_.emplace(UI_IMG_TYPE::STA_MASK, LoadGraph(PATH_IMAGE_STAMINA_MASK.c_str()));

	// モデルの基本設定_武器
	auto& nIns = NetManager::GetInstance();
	switch (nIns.GetWeapon(key_))
	{
		// 剣
	case SWORD_ID:
		uiImgs_.emplace(UI_IMG_TYPE::JOB, LoadGraph(PATH_IMAGE_SOWRD_ICON.c_str()));
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SWORD));
		attackDamage_ = SWORD_DAMAGE;
		hp_ = hpAgo_ = hpMax_ = MAX_HP + BONUS_HP;
		break;
		// 大剣
	case GREAT_SWORD_ID:
		uiImgs_.emplace(UI_IMG_TYPE::JOB, LoadGraph(PATH_IMAGE_GREATSOWRD_ICON.c_str()));
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SWORD2));
		attackDamage_ = GREAT_SWORD_DAMAGE;
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		break;
		// 弓
	case BOW_ID:
		uiImgs_.emplace(UI_IMG_TYPE::JOB, LoadGraph(PATH_IMAGE_ARROW_ICON.c_str()));
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::BOW));
		attackDamage_ = BOW_DAMAGE;
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		break;
	default:		
		break;
	}

	// パラメータの初期化
	damage_ = 0;
	stamina_ = staminaMax_ = MAX_STAMINA;
	staminaDir_ = 1.0f;
	isBreak_ = false;

	// シェーダーの初期化
	InitShader();

	// アニメーションの設定
	InitAnimation();

	// 獲得モーション
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::GET), true, BLEND_SPEED);

	// 投げるモーション
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ITEM_THROW), true, BLEND_SPEED);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ITEM_THROW_E), true, BLEND_SPEED);

	// 設置するモーション
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ITEM_SET), true, BLEND_SPEED);
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ITEM_SET_E), true, BLEND_SPEED);

	// 飲むモーション
	animationController_->SetIsBlend(static_cast<int>(ANIM_TYPE::ITEM_DRINK), true, BLEND_SPEED);

	// エフェクトの設定
	InitEffect();

	// BGM.SEの設定
	InitSound();

	// コントローラーの登録
	inputController_ = std::make_unique<InputController>(GameManager::GetInstance().GetControllId());

	// 初期状態
	ChangeState(STATE::PLAY);
	isBattle_ = false;
	isDrawWeapon_ = false;
	isCloseWeapon_ = false;

	// 攻撃管理
	isHit_ = false;
	isHitCheck_ = false;
	changeAttackTime_ = 0.0f;

	// 位置情報の変数
	movedPos_ = Utility::VECTOR_ZERO;
	moveDir_ = Utility::VECTOR_ZERO;
	movePow_ = Utility::VECTOR_ZERO;

	// 重力兼ジャンプ力
	jumpPow_ = Utility::VECTOR_ZERO;

	// 衝突チェック
	gravHitPosDown_ = Utility::VECTOR_ZERO;
	gravHitPosUp_ = Utility::VECTOR_ZERO;

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop(CAP_LOACL_TOP);
	capsule_->SetLocalPosDown(CAP_LOACL_DOWN);
	capsule_->SetRadius(CAP_RADIUS);
	
	// ステータスの初期化
	invisibleTime_ = 0.0f;				// 無敵時間
	flyigDir_ = Utility::VECTOR_ZERO;// 吹っ飛ばされる方向
	flyigTime_ = downTime_ = 0.0f;		// 吹っ飛ばされる時間

	// 武器ごとのパラメータ設定
	InitParam();

	// 採取の際の情報（仮）
	itemId_ = -1;	// 採取アイテムID
	poach_ = std::make_unique<ItemPoach>();
	for (int i = 0; i < START_BOM_HOLD; i++)
	{
		poach_->AddItem(std::make_shared<ItemBase>(ITEM_TYPE_FLASH));
	}
	for (int i = 0; i < START_FLASH_HOLD; i++)
	{
		poach_->AddItem(std::make_shared<ItemBase>(ITEM_TYPE_BOM));
	}
}

void Player::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	auto& nIns = NetManager::GetInstance();

	// ダメージを受けたらエフェクトストップ
	if (hp_ != hpAgo_)
	{
		effectController_->Stop(Player::POWER_UP_EFFECT);
		// 体力が0になっていたら
		if (hp_ <= 0) { gameScene_->DownCountPuls(); }
	}

	// 体力の変化を記録
	hpAgo_ = hp_;

	// アニメーションの種類を記録	
	animeAgoType_ = animeType_;

	// 1秒で0にするための減少速度を毎フレーム求める
	const float deltaTime = SceneManager::GetInstance().GetDeltaTime();

	// 自分のプレイヤーのときだけ入力を処理する
	if (key_ == nIns.GetSelf().key)
	{
		inputController_->Update();

		// アイテム選択
		if (inputController_->IsTriggered(InputController::KEY::ITEM_SELECT))
		{
			poach_->NextItem();
		}

		// 回避処理
		if (inputController_->IsTriggered(InputController::KEY::ROLL) && IsInputPlay()
			&& animeType_ != static_cast<int>(ANIM_TYPE::ROLL) && animeType_ != static_cast<int>(ANIM_TYPE::DAMAGE)
			&& animeType_ != static_cast<int>(ANIM_TYPE::GET) && animeType_ != static_cast<int>(ANIM_TYPE::ITEM_DRINK)
			&& animeType_ != static_cast<int>(ANIM_TYPE::ITEM_THROW)
			&& animeType_ != static_cast<int>(ANIM_TYPE::ITEM_SET)
			&& state_ != STATE::HI_DAMAGE && hp_ > 0 && stamina_ > ROLL_TAF)
		{
			stamina_ -= ROLL_TAF;
			invisibleTime_ = INVISIBLE_SMALL_TIME;
			ChangeState(STATE::ROWLING);
		}
		// 無敵時間の処理
		if (invisibleTime_ > 0.0f)
		{
			invisibleTime_ -= deltaTime;
		}
		// 赤ダメージ
		if (damage_ > 0)
		{
			damage_ -= static_cast<int>(static_cast<float>(damage_) * deltaTime);
			if (damage_ < 0) damage_ = 0;
		}

		// スタミナ
		stamina_ += staminaDir_ * deltaTime;
		if (staminaMax_ < stamina_) 
		{
			stamina_ = staminaMax_;
		}
		if (0.0f >= stamina_)
		{
			stamina_ = 0.0f;
			isBreak_ = true;
		}
		if (STAMINA_BREAK < stamina_ && isBreak_)
		{
			isBreak_ = false;
		}
		staminaDir_ = UP_TAF;

		// 更新ステップ
		stateUpdate_();

		if (isBattle_)
		{
			auto& nIns = NetManager::GetInstance();
			// 戦闘状態度と教える
			nIns.SetAction(PLAYER_ACTION::IS_BATTLE);
		}

		// 移動方向に応じた回転
		Rotate();

		// 重力による移動量
		CalcGravityPow();

		// 衝突判定
		Collision();

		// カメラ操作
		SceneManager::GetInstance().GetCamera().lock()->ProcessPlayRot(
			inputController_->IsNew(InputController::KEY::R_FORWARD),
			inputController_->IsNew(InputController::KEY::R_BACK),
			inputController_->IsNew(InputController::KEY::R_RIGHT),
			inputController_->IsNew(InputController::KEY::R_LEFT)
		);

		// 重力方向に沿って回転させる
		transform_.quaRot = Quaternion();//grvMng_がないので代わりに
		transform_.quaRot = transform_.quaRot.Mult(playerRotY_);


		// 自身のプレイヤーの情報をネットワークに送信
		nIns.SetPostion(key_, transform_.pos);
		nIns.SetPlayRot(key_, transform_.quaRot);

		nIns.SetAnimeType(key_, animeType_);

		nIns.SetNetHp(key_, hp_);
	}
	// 通信プレイヤーの処理
	else
	{
		// 攻撃と抜刀納刀のみループなし
		int nextAnimeType = nIns.GetAnimeType(key_);
		if (animeType_ != nextAnimeType)
		{
			animeType_ = nextAnimeType;
		}

		// 結果の通信を受け取って状態を更新
		// アニメーションの再生	
		bool loop = !IsLoopAnim();
		animationController_->Play(animeType_, loop);
		// 位置と回転の同期
		const auto& pos = nIns.GetPostion(key_);
		transform_.pos = pos;
		const auto& rot = nIns.GetPlayRot(key_);
		transform_.quaRot = rot;

		// チャージ処理
		if (animeType_ == static_cast<int>(ANIM_TYPE::ATTACK1STOP)
			)
		{
			changeAttackTime_ += SceneManager::GetInstance().GetDeltaTime();
		}
		else
		{
			changeAttackTime_ = 0.0f;
			chageCount_ = 1.0f;
		}

		hp_ = nIns.GetNetHp(key_);

		// プレイヤーの座標
		const auto& selfPos = nIns.GetPostion(nIns.GetSelf().key);

		// 音量設定
		float volume = Utility::CalcVolumeByDistance(selfPos, transform_.pos, MAX_EAR_RADIUS);

		// 無音なら停止
		for (int i = 0; i < static_cast<int>(SE::MAX); i++)
		{
			SE se = static_cast<SE>(i);
			// ここで se を使う処理を書く
			soundController_->ChengeVolume(i, volume);		// ボリュームだけ更新
		}
	}
	
	// エフェクト処理
#pragma region エフェクト処理

	float rate = changeAttackTime_ / CHAGE_UP_RATE;
	if (rate >= chageCount_ && animeType_ == static_cast<int>(ANIM_TYPE::ATTACK1STOP)
		)
	{
		effectController_->Play(Player::POWER_UP_EFFECT);
		soundController_->Play(static_cast<int>(SE::CHAGE), Sound::TIMES::ONCE, true);

		chageCount_ += 1.0f;
	}
	else if (animeType_ != static_cast<int>(ANIM_TYPE::ATTACK1STOP)
		)
	{
		chageCount_ = 1.0f;
	}

	// エフェクトの更新
	effectController_->Update(Player::POWER_UP_EFFECT, transform_.pos, Utility::VECTOR_ZERO, Player::PLAYER_EFFECT_SCALE);
	effectController_->LoopUpdate(Player::POWER_SLASH_EFFECT, transWeapon_.pos, Utility::VECTOR_ZERO, Player::PLAYER_EFFECT_SCALE);
	
	// 音の再生
	if (animeAgoType_ != static_cast<int>(ANIM_TYPE::DRAW)
		&& animeType_== static_cast<int>(ANIM_TYPE::DRAW)
		)
	{
		soundController_->Play(static_cast<int>(SE::DRAW), Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != static_cast<int>(ANIM_TYPE::BATTLE_CLOSE)
		&& animeType_== static_cast<int>(ANIM_TYPE::BATTLE_CLOSE)
		)
	{
		soundController_->Play(static_cast<int>(SE::CLOSE), Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != static_cast<int>(ANIM_TYPE::DAMAGE)
		&& animeType_== static_cast<int>(ANIM_TYPE::DAMAGE)
		)
	{
		soundController_->Play(static_cast<int>(SE::DAMAGE), Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != static_cast<int>(ANIM_TYPE::FLYING)
		&& animeType_== static_cast<int>(ANIM_TYPE::FLYING)
		)
	{
		soundController_->Play(static_cast<int>(SE::HI_DAMAGE), Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != static_cast<int>(ANIM_TYPE::DOWN)
		&& animeType_== static_cast<int>(ANIM_TYPE::DOWN)
		)
	{
		soundController_->Play(static_cast<int>(SE::DOWN), Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != static_cast<int>(ANIM_TYPE::ROLL)
		&& animeType_== static_cast<int>(ANIM_TYPE::ROLL)
		)
	{
		soundController_->Play(static_cast<int>(SE::ROLL), Sound::TIMES::ONCE,true);
	}
	// 弾の発射
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ITEM_THROW_E)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ITEM_THROW_E)
		)
	{
		// フレームの取得
		int frmNo = MV1SearchFrame(transform_.modelId, RIGHT_ARROW_HAND_BONE.c_str());
		if (frmNo == -1)
		{
			// エラー処理またはログ出力
			return;
		}

		// 手の位置とグローバルマトリクスを取得
		const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);
		gameScene_->CreateShot(ShotBase::TYPE::ITEM, attackDamage_
			, posHand, transform_.GetForward(), key_);
	}
	// 設置の完了
	else if (animeType_ == static_cast<int>(ANIM_TYPE::ITEM_SET_E)
		&& animeAgoType_ != static_cast<int>(ANIM_TYPE::ITEM_SET_E)
		)
	{
		gameScene_->CreateShot(ShotBase::TYPE::BOM, attackDamage_
			, VAdd(VAdd(transform_.pos
				, VScale(transform_.GetForward(), (capsule_->GetRadius() * 3.0f)))
				, VScale(Utility::DIR_U, 40.0f))
			, Utility::VECTOR_ZERO, key_);
	}

	// 移動時間によるサウンド管理
	walkTime_ -= SceneManager::GetInstance().GetDeltaTime();
	if (animeAgoType_ == animeType_ &&
		walkTime_ <= 0.0f &&
		(animeType_ == static_cast<int>(ANIM_TYPE::BTLLE_RUN)
			|| animeType_ == static_cast<int>(ANIM_TYPE::RUN)))
	{
		walkTime_ = FOOT_SMOKE;
		soundController_->Play(static_cast<int>(SE::WALK), Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ == animeType_ &&
		walkTime_ <= 0.0f &&
		( animeType_ == static_cast<int>(ANIM_TYPE::BTLLE_FAST_RUN)
			|| animeType_ == static_cast<int>(ANIM_TYPE::FAST_RUN)))
	{
		walkTime_ = FAST_FOOT_SMOKE;
		soundController_->Play(static_cast<int>(SE::RUN), Sound::TIMES::ONCE);
	}

	// 攻撃時の音再生
	PlayAttackSound();

#pragma endregion

	// 描画用の位置は、Draw()でNetManagerから取るからOK
	transform_.Update();

	// アニメーション再生
	animationController_->Update();
	if (animationController_->IsEnd() && animeType_ == static_cast<int>(ANIM_TYPE::DEAD)
		&& gameScene_->GetDownCount() >= GameScene::MAX_DOWN_COUNT)
	{
		// ゲームの勝敗判定
		GameManager::GAME_RESULT result = GameManager::GAME_RESULT::GAME_OVER;
		GameManager::GetInstance().SetGameResult(result);
		SceneManager::GetInstance().CaptureMainScreen();
	}

	// 武器の同期
	SyncWeapon();
}

void Player::Draw(void)
{
	auto& nIns = NetManager::GetInstance();

	// 位置の調整（アニメーションのローカル関係）
	transform_.pos = VAdd(transform_.pos, VScale(transform_.GetForward(), animationController_->GetPos().z));
	transform_.pos = VAdd(transform_.pos, VScale(transform_.GetRight(), animationController_->GetPos().x));
	transform_.Update();

	//武器の同期
	SyncWeapon();

	// モデルの描画
	MV1DrawModel(transform_.modelId);

	// 影の描画
	DrawShadow();

	//武器の描画
	DrawWeapon();

	// 位置の調整（アニメーションのローカル関係）
	transform_.pos = VSub(transform_.pos, VScale(transform_.GetForward(), animationController_->GetPos().z));
	transform_.pos = VSub(transform_.pos, VScale(transform_.GetRight(), animationController_->GetPos().x));
	transform_.Update();

	//武器の同期
	SyncWeapon();

#ifdef _DEBUG
	// デバッグ用
	DrawDebug();
	inputController_->DebugDraw();

#endif
}

void Player::DrawUI(int i)
{
	auto& nIns = NetManager::GetInstance();


	// 自身のUI（他描画なし）
	if (key_ == nIns.GetSelf().key)
	{
		hpMaterial_->SetConstBuf(1, { (float)hp_ / (float)hpMax_, ((float)hp_ + damage_) / (float)hpMax_, 1.0f, 1.0f });
		hpRenderer_->Draw(GAGE_POS_X, GAGE_POS_Y);
		staMaterial_->SetConstBuf(1, { stamina_ / staminaMax_, 0.0f, 1.0f, 1.0f });
		staRenderer_->Draw(GAGE_POS_X, STA_POS_Y);

		// アイテムの表示
		poach_->Draw(-1);
	}

	// 共通描画
	statusMaterial_->SetConstBuf(1, { (float)hp_ / (float)hpMax_, 1.0f, 1.0f, 1.0f });
	statusRenderer_->Draw(PRAM_POS_X, PRAM_POS_Y + (PRAM_distance_Y * i));

#ifdef _DEBUG
	const auto& pos = nIns.GetPostion(key_);
	const auto& animeType = nIns.GetAnimeType(key_);

	// プレイヤーデバッグ情報
	DrawFormatString(300, i * 65, 0x000000, L"プレイヤー番号(%d)", key_);
	DrawFormatString(300, i * 65 + 16, 0x000000, L"プレイヤー座標(%.2f, %.2f,%2f)", transform_.pos.x, transform_.pos.y, transform_.pos.z);
	DrawFormatString(300, i * 65 + 32, 0x000000, L"アニメーション(%d, %d,%d)", animeType, areaId_, hp_);

#endif // DEBUG

}

void Player::Release(void)
{
}

void Player::Damage(int dama, const VECTOR atkPos, const VECTOR mixDir)
{
	auto& nIns = NetManager::GetInstance();

	if ((animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::DAMAGE))
		||state_ == STATE::HI_DAMAGE
		||state_ == STATE::DEAD
		|| invisibleTime_ > 0.0f)
	{
		return;
	}

	movePow_ = Utility::VECTOR_ZERO;

	if (!Utility::EqualsVZero(mixDir))
	{
		flyigTime_ = 1.0f;
		
		// 攻撃者の位置（atkPos）に向けて回転する処理
		VECTOR lookDir = VNorm(VSub(atkPos, transform_.pos));	// 自分 → 攻撃者 のベクトル
		float rotRad = atan2f(lookDir.x, lookDir.z);			// XZ平面でのラジアン角

		// rotRad を使ってワールド空間回転を直接設定する（カメラとは無関係）
		Quaternion lookRot = Quaternion::AngleAxis(rotRad, Utility::AXIS_Y);
		goalQuaRot_ = lookRot;
		playerRotY_ = lookRot; // ←すぐ向かせたいならこの行も追加

		// 吹っ飛び方向（攻撃者 → 自分）も正しく設定
		flyigDir_ = VNorm(VSub(transform_.pos, atkPos));
		flyigDir_ = VNorm(VAdd(flyigDir_, mixDir));
		flyigDir_.y = 0.0f;

		attackType_ = static_cast<int>(ANIM_TYPE::FLYING);
		ChangeState(STATE::HI_DAMAGE);
	}
	else
	{
		ChangeState(STATE::DAMAGE);
	}
	
	damage_ = dama;
	// HPからdamage_分減らす
	hp_ -= damage_;
	if (hp_ <= 0)
	{ 
		hp_ = 0; 
		damage_ = 0;
		ChangeState(STATE::DEAD);
		return;
	}
	invisibleTime_ = INVISIBLE_BIG_TIME;// 威力・小のときの無敵時間

	// 攻撃、納刀モーションをリセット
	AttackReset();
}

const bool Player::IsHit(void) const
{
	return (isHitCheck_ && !isHit_);
}

const bool Player::GetHit(void) const
{
	return isHit_;
}

const void Player::SetHit(bool flag)
{
	isHit_ = flag;
	animationController_->SetHitStop(HITSTOP_TIME);
}

std::weak_ptr<Capsule> Player::GetCapsule(void)
{
	return capsuleWeapon_;
}

const PLAYER_TYPE& Player::GetPlayerType(void)const
{
	return type_;
}

void Player::InitParam(void)
{
}

void Player::InitAnimation(void)
{
	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
}

void Player::InitEffect(void)
{	
}

void Player::InitSound(void)
{
	soundController_ = std::make_unique<SoundController>();

	// サウンドの追加
	for (auto& sound : SOUND_LIST)
	{
		soundController_->Add(sound.type, sound.path, sound.vol);
	}

	InitAttackSound();
}

void Player::InitAttackSound(void)
{
	std::wstring path = Application::PATH_SOUND;
}

void Player::InitShader(void)
{
	// ステータスUI
	statusMaterial_ = std::make_unique<PixelMaterial>(SHADER_STATUS.path, SHADER_STATUS.bufNum);
	for (const auto& buf : SHADER_STATUS.bufs)
	{
		statusMaterial_->AddConstBuf(buf);
	}
	statusMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::FREAM]);
	statusMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::HP]);
	statusMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::JOB]);
	statusRenderer_ = std::make_unique<PixelRenderer>(*statusMaterial_);
	statusRenderer_->SetSize(Vector2(PRAM_SIZE_X, PRAM_SIZE_Y));

	// HP_UI
	hpMaterial_ = std::make_unique<PixelMaterial>(SHADER_HP.path, SHADER_HP.bufNum);
	for (const auto& buf : SHADER_HP.bufs)
	{
		hpMaterial_->AddConstBuf(buf);
	}
	hpMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::HP_FREAM]);
	hpMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::HP_MASK]);
	hpRenderer_ = std::make_unique<PixelRenderer>(*hpMaterial_);
	hpRenderer_->SetSize(Vector2(HP_SIZE_X, HP_SIZE_Y));

	// スタミナ_UI
	staMaterial_ = std::make_unique<PixelMaterial>(SHADER_STAMINA.path, SHADER_STAMINA.bufNum);
	for (const auto& buf : SHADER_STAMINA.bufs)
	{
		staMaterial_->AddConstBuf(buf);
	}
	staMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::STA_FREAM]);
	staMaterial_->AddTextureBuf(uiImgs_[UI_IMG_TYPE::STA_MASK]);
	staRenderer_ = std::make_unique<PixelRenderer>(*staMaterial_);
	staRenderer_->SetSize(Vector2(HP_SIZE_X, HP_SIZE_Y));
}

#pragma region StateによるUpdateの切り替え

void Player::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();
}

void Player::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Player::UpdateNone, this);
}

void Player::ChangeStatePlay(void)
{
	isBattle_ = false;
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::ChangeStateWeapon(void)
{
	stateUpdate_ = std::bind(&Player::UpdateWeapon, this);
}

void Player::ChangeStateBattle(void)
{
	isBattle_ = true;
	stateUpdate_ = std::bind(&Player::UpdateBattle, this);
}

void Player::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&Player::UpdateAttack, this);
}

void Player::ChangeStateRowling(void)
{
	stateUpdate_ = std::bind(&Player::UpdateRowling, this);
}

void Player::ChangeStateDamage(void)
{
	stateUpdate_ = std::bind(&Player::UpdateDamage, this);
}

void Player::ChangeStateHiDamage(void)
{
	stateUpdate_ = std::bind(&Player::UpdateHiDamage, this);
}

void Player::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&Player::UpdateDead, this);
}

void Player::ChangeStateGet(void)
{
	stateUpdate_ = std::bind(&Player::UpdateGet, this);
}

void Player::ChangeStateItemUse(void)
{
	stateUpdate_ = std::bind(&Player::UpdateItemUse, this);
}

#pragma endregion

#pragma region StateごとのUpdate

// stateがNONEの時のUpdate
void Player::UpdateNone(void)
{
}

// stateがPLAYの時のUpdate
void Player::UpdatePlay(void)
{
	// 移動処理
	ProcessNormal();
}

// stateがBATTLEの時のUpdate
void Player::UpdateBattle(void)
{
#pragma region 攻撃処理
	
	// 抜刀してすぐ攻撃できるようIsNewを使用
	if (inputController_->IsNew(InputController::KEY::ATTACK) && IsInputPlay())
	{
		isHit_ = false;
		isHitCheck_ = false;
		movePow_ = Utility::VECTOR_ZERO;

		attackType_ = static_cast<int>(ANIM_TYPE::ATTACK1S);
		ChangeState(STATE::ATTACK);
		return;
	}

#pragma endregion

	// 移動処理(攻撃も含む)
	ProcessBattle();
}

void Player::UpdateWeapon(void)
{
	if (isBattle_)
	{
		if (isDrawWeapon_)
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::BATTLE_DRAW), false);
			animeType_ = static_cast<int>(ANIM_TYPE::BATTLE_DRAW);

			if (animationController_->IsEnd())
			{
				isDrawWeapon_ = false;

				ChangeState(STATE::BATTLE);
				
			}
		}
		else if (isCloseWeapon_)
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::BATTLE_CLOSE), false);
			animeType_ = static_cast<int>(ANIM_TYPE::BATTLE_CLOSE);

			if (animationController_->IsEnd())
			{
				isBattle_ = false;
			}
		}
	}
	else
	{
		if (isDrawWeapon_)
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::DRAW), false);
			animeType_ = static_cast<int>(ANIM_TYPE::DRAW);

			if (animationController_->IsEnd())
			{
				
				isBattle_ = true;
				// 抜刀してすぐ攻撃できるようIsNewを使用
				if (inputController_->IsNew(InputController::KEY::ATTACK))
				{
					isDrawWeapon_ = false;

					isHit_ = false;
					isHitCheck_ = false;
					movePow_ = Utility::VECTOR_ZERO;


					attackType_ = static_cast<int>(ANIM_TYPE::ATTACK1S);
					ChangeState(STATE::ATTACK);
					return;
				}
			}
		}
		else if (isCloseWeapon_)
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::CLOSE), false);
			animeType_ = static_cast<int>(ANIM_TYPE::CLOSE);

			if (animationController_->IsEnd())
			{
				isCloseWeapon_ = false;
				ChangeState(STATE::PLAY);
			}
		}
	}

	if (inputController_->IsNew(InputController::KEY::FORWARD)
		|| inputController_->IsNew(InputController::KEY::LEFT)
		|| inputController_->IsNew(InputController::KEY::BACK)
		|| inputController_->IsNew(InputController::KEY::RIGHT))
	{
	}
	else{ movePow_ = Utility::VECTOR_ZERO; }
}

void Player::UpdateAttack(void)
{
	AttackUpdate();
}

void Player::UpdateRowling(void)
{
	staminaDir_ = 0.0f;
	movePow_ = VScale(transform_.GetForward(), SPEED_ROLL);

	ChangeStateAnimeEnd(ANIM_TYPE::ROLL);
}

void Player::UpdateDamage(void)
{
	// アニメーションが終わったらPLAYかBATTLEに...
	ChangeStateAnimeEnd(ANIM_TYPE::DAMAGE);
}

void Player::UpdateHiDamage(void)
{
	if (flyigTime_ > 0.0f)
	{
		flyigTime_ -= SceneManager::GetInstance().GetDeltaTime() * FLING_COUNT_RATE;

		animationController_->Play(attackType_, true);
		movePow_ = VScale(flyigDir_, SPEED_JUMP * (flyigTime_ + FLING_MOVE_RATE));

		if (flyigTime_ < 0.0f)
		{
			movePow_ = Utility::VECTOR_ZERO;
			attackType_ = atkData_[attackType_]->nextAttack;
			animeType_ = attackType_;
			downTime_ = DOWN_MAX;
		}
		else animeType_ = attackType_;

		return;
	}
	else if (downTime_ > 0.0f)
	{
		downTime_ -= SceneManager::GetInstance().GetDeltaTime();

		animationController_->Play(attackType_, true);

		if (downTime_ < 0.0f)
		{
			attackType_ = atkData_[attackType_]->nextAttack;
			animeType_ = attackType_;
		}
		else animeType_ = attackType_;

		return;
	}

	// 吹っ飛ばしが終わった後
	animationController_->Play(attackType_, false);

	if (!animationController_->IsBlend())
	{
		int x = 0;
		if (atkData_[attackType_]->nextAttack <= 0)
		{
			if (isBattle_)
			{
				ChangeState(STATE::BATTLE);
			}
			else
			{
				ChangeState(STATE::PLAY);
			}
			return;
		}

		attackType_ = atkData_[attackType_]->nextAttack;
		animeType_ = attackType_;
	}
	else
	{
		animeType_ = attackType_;
	}
}

void Player::UpdateDead(void)
{
	isBattle_ = false;
	animationController_->Play(static_cast<int>(ANIM_TYPE::DEAD), false);
	animeType_ = static_cast<int>(ANIM_TYPE::DEAD);

	if (animationController_->IsEnd())
	{
		transform_.pos = START_POS;
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		AttackReset();
		ChangeState(STATE::PLAY);
	}
}

void Player::UpdateGet(void)
{
	if (animationController_->IsEnd())
	{
		if (itemId_ >= 0)// 獲得できる
		{
			switch (itemId_)
			{
				case 0:// 投擲アイテム
					poach_->AddItem(std::make_shared<ItemBase>(ITEM_TYPE_FLASH));
					break;
				case 1:// 回復アイテム
					poach_->AddItem(std::make_shared<ItemBase>(ITEM_TYPE_HEEL));
					break;
				case 2:// 設置アイテム
					poach_->AddItem(std::make_shared<ItemBase>(ITEM_TYPE_BOM));
					break;
			}
		}
	}
	ChangeStateAnimeEnd(ANIM_TYPE::GET);
}

void Player::UpdateItemUse(void)
{
	if (poach_->IsSelectedItemName(ITEM_TYPE_HEEL))
	{
		ChangeStateAnimeEnd(ANIM_TYPE::ITEM_DRINK, [this]() { UseItem(); });
	}
	// 使用アイテムが投擲アイテムなら
	else if (poach_->IsSelectedItemName(ITEM_TYPE_FLASH))
	{
		if (animeType_ != static_cast<int>(ANIM_TYPE::ITEM_THROW_E))
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::ITEM_THROW), false);
			animeType_ = static_cast<int>(ANIM_TYPE::ITEM_THROW);
			if (animationController_->IsEnd())
			{
				animeType_ = static_cast<int>(ANIM_TYPE::ITEM_THROW_E);
			}
		}
		else
		{
			ChangeStateAnimeEnd(ANIM_TYPE::ITEM_THROW_E, [this]() { UseItem(); });
		}
	}
	// 使用アイテムが投擲アイテムなら
	else if (poach_->IsSelectedItemName(ITEM_TYPE_BOM))
	{
		if (animeType_ != static_cast<int>(ANIM_TYPE::ITEM_SET_E))
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::ITEM_SET), false);
			animeType_ = static_cast<int>(ANIM_TYPE::ITEM_SET);
			if (animationController_->IsEnd())
			{
				animeType_ = static_cast<int>(ANIM_TYPE::ITEM_SET_E);
			}
		}
		else
		{
			ChangeStateAnimeEnd(ANIM_TYPE::ITEM_SET_E, [this]() { UseItem(); });
		}
	}
}

#pragma endregion

// 使用回復系アイテム
void Player::UseItem(void)
{
	// 回復アイテムのIdは１
	if (poach_->IsSelectedItemName(ITEM_TYPE_HEEL))
	{
		// 体力回復
		hp_ += HEEL_HP;
		if (hp_ > hpMax_)
		{
			hp_ = hpMax_;
		}
	}
	// アイテム使用処理
	poach_->UseSelectedItem();
}

#pragma region 移動関係

void Player::ProcessNormal(void)
{
	VECTOR dir = Utility::VECTOR_ZERO;

	bool isAction = false;

	if (inputController_->IsNew(InputController::KEY::FORWARD)) { dir = VAdd(dir, Utility::DIR_F); }
	if (inputController_->IsNew(InputController::KEY::LEFT)) { dir = VAdd(dir, Utility::DIR_L); }
	if (inputController_->IsNew(InputController::KEY::BACK)) { dir = VAdd(dir, Utility::DIR_B); }
	if (inputController_->IsNew(InputController::KEY::RIGHT)) { dir = VAdd(dir, Utility::DIR_R); }

	// 移動方向の向き設定
	float rotRad = CreateRad(dir);
	
	// 抜刀じゃないなら
	if (IsInputPlay())
	{
		// 抜刀処理
		if (inputController_->IsTriggered(InputController::KEY::DRAW))
		{
			if (!Utility::EqualsVZero(movePow_))
			{
				speed_ = SPEED_MOVE;

				movePow_ = VScale(VNorm(movePow_), speed_);
			}
			movePow_ = Utility::VECTOR_ZERO;// 抜刀時移動しない（帰るなら戻す）

			isDrawWeapon_ = true;
			ChangeState(STATE::WEAPON);
			return;
		}
		// 採取判定
		bool isId = false;
		for (const auto& c : colliders_)
		{
			if (c.lock()->type_ == Collider::TYPE::ITEM)
			{
				if (Utility::IsHitSpheres(transform_.pos, capsule_->GetRadius(), c.lock()->pos_, c.lock()->radius_))
				{
					isId = true;
				}

				// 採取処理
				if (inputController_->IsNew(InputController::KEY::GET) &&
					Utility::IsHitSpheres(transform_.pos, capsule_->GetRadius(), c.lock()->pos_, c.lock()->radius_))
				{
					speed_ = SPEED_MOVE;
					movePow_ = Utility::VECTOR_ZERO;//抜刀時移動しない（帰るなら戻す）

					ChangeState(STATE::GET);
					return;
				}
			}
		}
		if (!isId)
		{
			itemId_ = -1;
		}

		// 使用処理
		if (inputController_->IsTriggered(InputController::KEY::USE) && poach_->HasSelectedItem())
		{
			speed_ = SPEED_MOVE;
			movePow_ = Utility::VECTOR_ZERO;// 抜刀時移動しない（帰るなら戻す）

			ChangeState(STATE::ITEM_PLAY);
			return;
		}

		movePow_ = Utility::VECTOR_ZERO;

		if ((!Utility::EqualsVZero(dir)))
		{
			// 入力された方向をかめらの回転情報を使って
			// カメラの進行方向に変換する
			Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();
			dir = cameraRot.PosAxis(dir);

			// 移動処理
			speed_ = SPEED_MOVE;
			bool isDash = inputController_->IsNew(InputController::KEY::DASH);
			if (isDash && !isBreak_)
			{
				speed_ = SPEED_RUN;
				staminaDir_ = DOWN_TAF;
			}

			movePow_ = VScale(dir, speed_);

			// 回転処理
			SetGoalRotate(rotRad);

			if (IsEndLanding())// モーションが変えていいやつか？
			{
				// アニメーション
				if (isDash)
				{
					animationController_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN));
					animeType_ = static_cast<int>(ANIM_TYPE::FAST_RUN);
				}
				else
				{
					animationController_->Play(static_cast<int>(ANIM_TYPE::RUN));
					animeType_ = static_cast<int>(ANIM_TYPE::RUN);
				}
			}
		}
		else
		{
			if (IsEndLanding())
			{
				animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
				animeType_ = static_cast<int>(ANIM_TYPE::IDLE);
			}
		}
	}

	InputManager& ins = InputManager::GetInstance();
	if (ins.IsNew(KEY_INPUT_N))
	{
		SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FPS);
	}
	else if (ins.IsNew(KEY_INPUT_M))
	{
		SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FOLLOW);
	}
}

void Player::ProcessBattle(void)
{
	InputManager& ins = InputManager::GetInstance();
	VECTOR dir = Utility::VECTOR_ZERO;

	bool isAction = false;
	auto& nIns = NetManager::GetInstance();

	if (inputController_->IsNew(InputController::KEY::FORWARD)) { dir = VAdd(dir, Utility::DIR_F); }
	if (inputController_->IsNew(InputController::KEY::LEFT)) { dir = VAdd(dir, Utility::DIR_L);}
	if (inputController_->IsNew(InputController::KEY::BACK)) { dir = VAdd(dir, Utility::DIR_B);}
	if (inputController_->IsNew(InputController::KEY::RIGHT)) { dir = VAdd(dir, Utility::DIR_R);}
	
	// 移動方向の向き設定
	float rotRad = CreateRad(dir);

	if (IsInputPlay())
	{
		if (inputController_->IsTriggered(InputController::KEY::CLOSE))
		{
			isCloseWeapon_ = true;
			ChangeState(STATE::WEAPON);
			movePow_ = Utility::VECTOR_ZERO;// 納刀時移動しない（帰るなら戻す）

			return;
		}

		// 移動量初期化
		movePow_ = Utility::VECTOR_ZERO;

		if ((!Utility::EqualsVZero(dir)))
		{
			// 入力された方向をかめらの回転情報を使って
			// カメラの進行方向に変換する
			Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();
			dir = cameraRot.PosAxis(dir);

			// 移動処理
			speed_ = SPEED_MOVE;
			bool isDash = (inputController_->IsNew(InputController::KEY::DASH) && isBattleDash_);
			if (isDash && !isBreak_)
			{
				speed_ = SPEED_RUN;
				staminaDir_ = DOWN_TAF;
			}

			// 移動力セット
			movePow_ = VScale(dir, speed_);

			// 回転処理
			SetGoalRotate(rotRad);

			if (IsEndLanding())// モーションが変えていいやつか？
			{
				// アニメーション
				if (isDash)
				{
					animationController_->Play(static_cast<int>(ANIM_TYPE::BTLLE_FAST_RUN));
					animeType_ = static_cast<int>(ANIM_TYPE::BTLLE_FAST_RUN);
				}
				else
				{
					animationController_->Play(static_cast<int>(ANIM_TYPE::BTLLE_RUN));
					animeType_ = static_cast<int>(ANIM_TYPE::BTLLE_RUN);
				}
			}
		}
		else
		{
			if (IsEndLanding())
			{
				animationController_->Play(static_cast<int>(ANIM_TYPE::BTLLE_IDLE));
				animeType_ = static_cast<int>(ANIM_TYPE::BTLLE_IDLE);
			}
		}
	}

	// カメラ切り替え
	if (ins.IsNew(KEY_INPUT_N))
	{
		SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FPS);
	}
	else if (ins.IsNew(KEY_INPUT_M))
	{
		SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FOLLOW);
	}
}

#pragma endregion

#pragma region 回転関係

// プレイヤーに向かせたい、ゴールとなる回転を設定する
void Player::SetGoalRotate(double rotRad)
{
	VECTOR cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetAngles();

	Quaternion axis =
		Quaternion::AngleAxis(
			(double)cameraRot.y + rotRad, Utility::AXIS_Y);

	// 現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// しきい値
	if (angleDiff > ROT_DIFF)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;
}

void Player::Rotate(void)
{
	// 回転の球面補間を行う。
	// TIME_ROT定数で指定された時間をかけて、ゆっくりゴールとなる
	// 回転に向かって近づくような回転を行う
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();
	// 回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}

float Player::CreateRad(const VECTOR& dir)
{
	float angle = atan2f(dir.x, dir.z);
	if (angle < 0.0f)
	{
		angle += static_cast<float>(DX_TWO_PI); // 0~2paiに正規化
	}
	return angle;
}

#pragma endregion

// 武器の描画
void Player::DrawWeapon()
{
	MV1DrawModel(transWeapon_.modelId);
}

// 武器の同期
#pragma region 武器の同期

const void Player::SyncWeapon()
{
	auto& nIns = NetManager::GetInstance();

	// 武器の位置同期
	if (key_ == nIns.GetSelf().key)
	{
		isBattle_ ? SyncWeaponBattle() : SyncWeaponPlay();
	}
	else {
		nIns.IsAction(key_, PLAYER_ACTION::IS_BATTLE) ? SyncWeaponBattle() : SyncWeaponPlay();
	}
}

void Player::SyncWeaponPlay()
{
	// メインウェポン（腰）
	SyncWeaponToFream(RIGHT_SPINE_BONE.c_str(), GSOWRD_SPINE_ROT, GSOWRD_SPINE_POS,
		transform_, transWeapon_);
}

void Player::SyncWeaponBattle()
{
	// メインウェポン（右手）
	SyncWeaponToFream(RIGHT_HAND_BONE.c_str(), GSOWRD_HAND_ROT, GSOWRD_HAND_POS,
		transform_, transWeapon_);
}

#pragma endregion

// オブジェクトのフレーム追従
const void Player::SyncWeaponToFream(const TCHAR* frameName, const VECTOR& offsetRot, const VECTOR& offsetPos,
	const Transform& modelTransform, Transform& outWeaponTransform)
{
	// プレイヤーモデルのスケールを逆にして、武器モデルのスケール計算に利用
	VECTOR scl = modelTransform.scl;
	scl.x = 1.0f / scl.x;
	scl.y = 1.0f / scl.y;
	scl.z = 1.0f / scl.z;

	// フレームの取得
	int frmNo = MV1SearchFrame(modelTransform.modelId, frameName);
	if (frmNo == -1)
	{
		// エラー処理またはログ出力
		return;
	}

	// 手の位置とグローバルマトリクスを取得
	const auto& posFream = MV1GetFramePosition(modelTransform.modelId, frmNo);
	MATRIX handWorldMatrix = MV1GetFrameLocalWorldMatrix(modelTransform.modelId, frmNo);

	// 手のワールド回転を取得
	Quaternion handWorldRot = Quaternion::GetRotation(handWorldMatrix);

	// 手のワールド回転とモデルスケールから、武器の基準となる行列を生成
	MATRIX mixMat = MMult(MGetRotElem(handWorldMatrix), MGetScale(scl));

	// 武器固有のオフセット回転を適用
	// X, Y, Z軸それぞれに回転を適用している現在のロジックを再現
	mixMat = MMult(mixMat,
		MGetRotAxis(VNorm(VTransformSR(Utility::DIR_R, handWorldMatrix)), Utility::Deg2RadF(offsetRot.x)));
	mixMat = MMult(mixMat,
		MGetRotAxis(VNorm(VTransformSR(Utility::DIR_U, handWorldMatrix)), Utility::Deg2RadF(offsetRot.y)));
	mixMat = MMult(mixMat,
		MGetRotAxis(VNorm(VTransformSR(Utility::DIR_F, handWorldMatrix)), Utility::Deg2RadF(offsetRot.z)));

	// 最終的な武器の回転行列を設定
	outWeaponTransform.matRot = mixMat;

	// 最終的な武器の位置を設定
	// オフセット位置は、手のワールド回転のバック方向を基準としていると仮定
	outWeaponTransform.pos = VAdd(posFream, VScale(handWorldRot.GetRight(), offsetPos.x));
	// 必要に応じて offsetPos.x や offsetPos.y も handWorldRot.GetRight() などで加算
	outWeaponTransform.pos = VAdd(outWeaponTransform.pos, VScale(handWorldRot.GetUp(), offsetPos.y));
	outWeaponTransform.pos = VAdd(outWeaponTransform.pos, VScale(handWorldRot.GetForward(), offsetPos.z));

	// モデルの更新
	outWeaponTransform.Update(true);
}

// 当たり判定
#pragma region コリジョン関数

void Player::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = Utility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = Utility::DIR_U;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, COL_CHECK_UP_POW));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, COL_CHECK_DOWN_POW));

	const auto gravHitPosUp = VAdd(gravHitPosUp_, VScale(transform_.GetForward(), COLL_LEG_RATE));
	const auto gravHitPosDown =VAdd(gravHitPosDown_, VScale(transform_.GetForward(), COLL_LEG_RATE));

	for (const auto& c : colliders_)
	{
		if (c.lock()->type_ == Collider::TYPE::STAGE)
		{
			// 地面との衝突
			auto hit = MV1CollCheck_Line(
				c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
			auto hitF = MV1CollCheck_Line(
				c.lock()->modelId_, -1, gravHitPosUp, gravHitPosDown);

			// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
			if ((hit.HitFlag > 0|| hitF.HitFlag > 0) && VDot(dirGravity, jumpPow_) > JUMP_DOT_BORDER)
			{
				// 使用する衝突位置を選ぶ
				VECTOR usedHitPosition = hit.HitFlag > 0 ? hit.HitPosition : VSub(hitF.HitPosition, VScale(transform_.GetForward(), COLL_LEG_RATE));

				// 衝突地点から、少し上に移動
				movedPos_ = VAdd(usedHitPosition, VScale(dirUpGravity, PUSH_BACK_LENGTH));

				// ジャンプリセット
				jumpPow_ = Utility::VECTOR_ZERO;
			}
		}
	}
}

void Player::CollisionStageCapsule(void)
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
						// 法線方向高さなし
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

bool Player::CollisionCapsule(int& modelId)const
{
	// ０番目のフレームのコリジョン情報を更新する
	MV1RefreshCollInfo(modelId, -1);

	bool ret = false;

	// カプセルとの衝突判定
	auto hits = MV1CollCheck_Capsule(
		modelId, -1,
		capsule_->GetPosTop(), capsule_->GetPosDown(), capsule_->GetRadius());

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

const bool Player::CollisionUnderSphere(const VECTOR pos, float r) const
{
	bool ret = false;

	// プレイヤーとの衝突判定
	VECTOR diff = VSub(transform_.pos, pos);
	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	ret = (disPow < capsule_->GetRadius() * r);// ダメージ半径×攻撃半径

	return ret;
}

#pragma endregion

// 攻撃状態の判定
const bool Player::IsAttack(void)const
{
	return(state_ == STATE::ATTACK);
}
const bool Player::IsLoopAnim(void) const
{
	if (animeType_ == static_cast<int>(ANIM_TYPE::ATTACK1S) ||
		animeType_ == static_cast<int>(ANIM_TYPE::ATTACK1STOP) ||
		animeType_ == static_cast<int>(ANIM_TYPE::ATTACK1E) ||
		animeType_ == static_cast<int>(ANIM_TYPE::ATTACK2) ||
		animeType_ == static_cast<int>(ANIM_TYPE::ATTACK3) ||
		animeType_ == static_cast<int>(ANIM_TYPE::BATTLE_CLOSE) ||
		animeType_ == static_cast<int>(ANIM_TYPE::BATTLE_DRAW) ||
		animeType_ == static_cast<int>(ANIM_TYPE::CLOSE) ||
		animeType_ == static_cast<int>(ANIM_TYPE::DRAW) ||
		animeType_ == static_cast<int>(ANIM_TYPE::DEAD)
		)
	{
		return true;
	}

	return false;
}

const bool Player::IsSelf(void) const
{
	auto& nIns = NetManager::GetInstance();

	bool ret = true;

	if (key_ != nIns.GetSelf().key)
	{
		ret = false;
	}

	return ret;
}

bool Player::IsAimSet(void)
{
	return inputController_->IsNew(InputController::KEY::AIM);
}

bool Player::IsTrgAimSet(void)
{
	return inputController_->IsTriggered(InputController::KEY::AIM);
}

bool Player::IsSyncAttack(void)
{
	return (animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK1E)
		|| animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK2)
		|| animationController_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK3)
		);
}

// 操作をしなくても終わるまで動くモーションのときに使用
bool Player::IsEndLanding(void)
{
	bool ret = true;
	return ret;
	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != static_cast<int>(ANIM_TYPE::ATTACK1S))
	{
		return ret;
	}
	if (animationController_->GetPlayType() != static_cast<int>(ANIM_TYPE::ATTACK1S))
	{
		return ret;
	}
	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;
	}
	return false;
}

// 操作が可能か
const bool Player::IsInputPlay(void) const
{
	// アニメーションが終了しているか
	// もし抜刀、納刀、攻撃してないなら操作可能（true）
	if (!isDrawWeapon_ && !isCloseWeapon_ && state_ != STATE::ATTACK)
	{
		return true;
	}
	return false;
}
// 通常状態に戻す
void Player::AttackReset(void)
{
	isDrawWeapon_ = false;
	isCloseWeapon_ = false;
		
	changeAttackTime_ = 0.0f;
}

// 特定のアニメーションが終わったらIdleに戻す処理
void Player::ChangeStateAnimeEnd(const ANIM_TYPE anim, const std::function<void(void)> function)
{
	animationController_->Play(static_cast<int>(anim), false);
	animeType_ = static_cast<int>(anim);

	if (animationController_->IsEnd())
	{
		// 追加処理
		if (function != nullptr)
		{
			function();
		}
		STATE nextState = STATE::PLAY;
		// バトル状態なら
		if (isBattle_)
		{
			nextState = STATE::BATTLE;
		}
		ChangeState(nextState);
	}
}

void Player::DrawDebug(void)
{
	DrawLine3D(prePos_, transform_.pos, 0xf0f000);

	// カプセルコライダ
	capsule_->Draw();
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0xffffff);
	const auto gravHitPosUp = VAdd(gravHitPosUp_, VScale(transform_.GetForward(), COLL_LEG_RATE));
	const auto gravHitPosDown = VAdd(gravHitPosDown_, VScale(transform_.GetForward(), COLL_LEG_RATE));
	DrawLine3D(gravHitPosUp, gravHitPosDown, 0xffffff);

	capsuleWeapon_->Draw();
}

void Player::AttackUpdate(void)
{
	animationController_->Play(attackType_, false);

	// 操作を受け付けるか
	if (animationController_->GetStepTime() > atkData_[attackType_]->sNewTime
		&& atkData_[attackType_]->sNewTime > 0.0f)
	{
		if (inputController_->IsTriggered(InputController::KEY::ATTACK)
			&& atkData_[attackType_]->nextAttack != -1)
		{
			isHitCheck_ = false;
			isHit_ = false;

			attackType_ = atkData_[attackType_]->nextAttack;
			return;
		}
		else if (inputController_->IsTriggered(InputController::KEY::ROLL)
			&& stamina_ > ROLL_TAF)
		{
			stamina_ -= ROLL_TAF;
			isHitCheck_ = false;
			changeAttackTime_ = 0.0f;

			invisibleTime_ = INVISIBLE_SMALL_TIME;
			ChangeState(STATE::ROWLING);
			return;
		}
	}

	// 当たり判定が発生するか
	if (atkData_[attackType_]->sHitTime < animationController_->GetStepTime()
		&& atkData_[attackType_]->HitTime > animationController_->GetStepTime())
	{
		effectController_->Update(Player::POWER_SLASH_EFFECT, capsuleWeapon_->GetCenter());
		isHitCheck_ = true;
	}

	if (animationController_->IsEnd())
	{
		// チャージするものか
		if (atkData_[attackType_]->isCharge)
		{
			if (inputController_->IsNew(InputController::KEY::ATTACK) && changeAttackTime_ <= CHAGE_MAX_TIME)
			{
				// チャージ中なら回転可能
				VECTOR dir = Utility::VECTOR_ZERO;

				if (inputController_->IsNew(InputController::KEY::FORWARD)) { dir = VAdd(dir, Utility::DIR_F); }
				if (inputController_->IsNew(InputController::KEY::LEFT)) { dir = VAdd(dir, Utility::DIR_L); }
				if (inputController_->IsNew(InputController::KEY::BACK)) { dir = VAdd(dir, Utility::DIR_B); }
				if (inputController_->IsNew(InputController::KEY::RIGHT)) { dir = VAdd(dir, Utility::DIR_R); }

				// 移動方向の向き設定
				float rotRad = CreateRad(dir);
				if (!Utility::EqualsVZero(dir))
				{
					// 入力された方向をかめらの回転情報を使って
					// カメラの進行方向に変換する
					Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();
					dir = cameraRot.PosAxis(dir);

					// 回転処理
					SetGoalRotate(rotRad);
				}
				// チャージ処理
				changeAttackTime_ += SceneManager::GetInstance().GetDeltaTime();
				animeType_ = atkData_[attackType_]->chargeId;
			}
			// チャージ終了なら次のアニメへ(攻撃へ)
			else
			{
				// 倍率設定
				float rate = changeAttackTime_ / CHAGE_UP_RATE;
				if (changeAttackTime_ >= CHAGE_MAX_TIME)rate = CHAGE_OVER_ATTACK_RATE;
				attackRate_ = 1.0f + (rate * CHAGE_ATTACK_RATE);

				attackType_ = atkData_[attackType_]->nextAttack;
				animeType_ = attackType_;

				return;
			}
		}
		// アニメーションが終わってチャージ処理もないなら終了
		else
		{
			animationController_->Play(static_cast<int>(ANIM_TYPE::BTLLE_IDLE));
			animeType_ = static_cast<int>(ANIM_TYPE::BTLLE_IDLE);

			changeAttackTime_ = 0.0f;
			ChangeState(STATE::BATTLE);
			return;
		}
	}
	else
	{
		animeType_ = attackType_;
	}
}
