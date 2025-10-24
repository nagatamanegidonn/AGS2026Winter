#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Common/Vector2.h"

#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
//#include "../Manager/SoundManager.h"

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

#include "../Statge/Planet.h"

#include "../Item/ItemBase.h"
#include "../Item/ItemPoach.h"

#include "Player.h"

namespace
{
	//ステータスの描画位置、サイズ
	constexpr int PRAM_SIZE_X = 100;
	constexpr int PRAM_SIZE_Y = 30;
	constexpr int PRAM_POS_X = 60;
	constexpr int PRAM_POS_Y = 150;
	constexpr int PRAM_distance_Y = PRAM_SIZE_Y + 4;

	constexpr int HP_SIZE_X = 512;
	constexpr int HP_SIZE_Y = 16;


	constexpr int GAGE_POS_X = 60 + 40 + HP_SIZE_X / 2;
	constexpr int GAGE_POS_Y = 60 - 2 - HP_SIZE_Y / 2;
	constexpr int STA_POS_Y = 60 + 2 + HP_SIZE_Y / 2;


}



MATRIX GetFrameGlobalMatrix(int modelHandle, int frameIndex) {
	MATRIX localMat = MV1GetFrameLocalMatrix(modelHandle, frameIndex);

	int parentIndex = MV1GetFrameParent(modelHandle, frameIndex);
	if (parentIndex == -1) {
		return localMat; // ルートなのでそのまま返す
	}

	MATRIX parentMat = GetFrameGlobalMatrix(modelHandle, parentIndex);
	return MMult(localMat, parentMat); // ※ DxLibの行列乗算
}

Player::Player(int key)
	:
	CharaBase(),
	type_(PLAYER_TYPE::PLAYER_1),
	key_(0),
	keyConfig_(),
	itemId_(-1),
	isHit_(false),
	walkTime_(0.0f),
	isBattle_(false),
	isDrawWepon_(false),
	isCloseWepon_(false),

	hp_(0),
	hpAgo_(0),
	hpMax_(0),

	stateChanges_(),
	isBattleDash_(false),
	soundController_(nullptr),
	jobImg_(-1),
	freamImg_(-1),
	hpImg_(-1),
	hpFreamImg_(-1),
	hpMaskImg_(-1),
	staFreamImg_(-1),
	staMaskImg_(-1),
	Material_(nullptr),
	Renderer_(nullptr),
	hpMaterial_(nullptr),
	hpRenderer_(nullptr),
	staMaterial_(nullptr),
	staRenderer_(nullptr),
	demoRot_(),
	transWeapon_(),
	animationController_(nullptr),
	inputController_(nullptr),
	effectController_(nullptr),
	gameScene_(nullptr),
	
	chageCount_(0.0f)
{
	key_ = key;

	animationController_ = nullptr;
	inputController_ = nullptr;
	effectController_ = nullptr;
	soundController_.reset();
	soundController_ = nullptr;
	state_ = STATE::NONE;

	isBattleDash_ = false;

	// 状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::BATTLE, std::bind(&Player::ChangeStateBattle, this));
	stateChanges_.emplace(STATE::WEPON, std::bind(&Player::ChangeStateWepon, this));
	stateChanges_.emplace(STATE::ATTRCK, std::bind(&Player::ChangeStateAttrck, this));
	stateChanges_.emplace(STATE::ROWLING, std::bind(&Player::ChangeStateRowling, this));
	stateChanges_.emplace(STATE::DAMAGE, std::bind(&Player::ChangeStateDamage, this));
	stateChanges_.emplace(STATE::HI_DAMAGE, std::bind(&Player::ChangeStateHiDamage, this));
	stateChanges_.emplace(STATE::DEAD, std::bind(&Player::ChangeStateDead, this));
	stateChanges_.emplace(STATE::GET, std::bind(&Player::ChangeStateGet, this));
	stateChanges_.emplace(STATE::ITEM_PLAY, std::bind(&Player::ChangeStateItemUse, this));

	walkTime_ = 0.0f;
}

Player::~Player(void)
{
	DeleteGraph(freamImg_);
	DeleteGraph(jobImg_);
	DeleteGraph(hpImg_);
	DeleteGraph(hpFreamImg_);
	DeleteGraph(hpMaskImg_);
	DeleteGraph(staFreamImg_);
	DeleteGraph(staMaskImg_);

	MV1DeleteModel(transform_.modelId);
	MV1DeleteModel(transWeapon_.modelId);
}

void Player::Init(GameScene* scene, PLAYER_TYPE type, KEY_CONFIG config)
{
	demoRot_ = AsoUtility::VECTOR_ZERO;

	// ゲームシーンの機能を使えるように
	//ここ＆（参照）のほうがいいのか？
	gameScene_ = scene;

	// プレイヤー種別(プレイヤー番号)
	type_ = type;

	// キー設定
	keyConfig_ = config;


	// 移動速度の初期化
	speed_ = 0.0f;

	// モデルの基本設定
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
		ResourceManager::SRC::PLAYER_NIGHT));
	transform_.scl = AsoUtility::VECTOR_ONE;
	// 初期座標
	//transform_.pos = { -500.0f, -30.0f, -1500.0f };
	transform_.pos = { 1200.0f,0.0f,-5000.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	auto& nIns = NetManager::GetInstance();

	freamImg_ = LoadGraph((Application::PATH_IMAGE + L"Fream.png").c_str());
	hpImg_ = LoadGraph((Application::PATH_IMAGE + L"hp.png").c_str());
	//HP
	hpFreamImg_ = LoadGraph((Application::PATH_IMAGE + L"UI/hpFream.png").c_str());
	hpMaskImg_ = LoadGraph((Application::PATH_IMAGE + L"UI/hpMask.png").c_str());
	//スタミナ
	staFreamImg_ = LoadGraph((Application::PATH_IMAGE + L"UI/staFream.png").c_str());
	staMaskImg_ = LoadGraph((Application::PATH_IMAGE + L"UI/staMask.png").c_str());

	// モデルの基本設定//武器の設定
	switch (nIns.GetWeapon(key_))
	{
	case 0:
		jobImg_ = LoadGraph((Application::PATH_IMAGE + L"Job/Sowrd.png").c_str());
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SWORD));
		attrckDamage_ = 10;
		hp_ = hpAgo_ = hpMax_ = MAX_HP + 20;
		break;
	case 1:
		jobImg_ = LoadGraph((Application::PATH_IMAGE + L"Job/GreatSowrd.png").c_str());
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SWORD2));
		attrckDamage_ = 20;
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		break;
	case 2:
		jobImg_ = LoadGraph((Application::PATH_IMAGE + L"Job/Arrow.png").c_str());
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::BOW));
		attrckDamage_ = 5;
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		break;
	default:
		transWeapon_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SWORD2));
		attrckDamage_ = 20;
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		break;
	}

	damage_ = 0;
	stamina_ = staminaMax_ = MAX_STAMINA;
	staminaDir_ = 1.0f;
	isBreak_ = false;


	//ステータスUI
	Material_ = std::make_unique<PixelMaterial>(L"PlayerStatus.cso", 2);
	Material_->AddConstBuf({ 0.3f, 0.7f, 0.5f, 0.05f });//HP位置(左上)サイズ
	Material_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	Material_->AddTextureBuf(freamImg_);
	Material_->AddTextureBuf(hpImg_);
	Material_->AddTextureBuf(jobImg_);
	Renderer_ = std::make_unique<PixelRenderer>(*Material_);
	Renderer_->SetSize(Vector2(PRAM_SIZE_X, PRAM_SIZE_Y));

	//HP_UI
	hpMaterial_ = std::make_unique<PixelMaterial>(L"PlayerHp.cso", 2);
	hpMaterial_->AddConstBuf({ 0.0f, 1.0f,0.0f,  1.0f });//HP位置(左上)サイズ
	hpMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	hpMaterial_->AddTextureBuf(hpFreamImg_);
	hpMaterial_->AddTextureBuf(hpMaskImg_);
	hpRenderer_ = std::make_unique<PixelRenderer>(*hpMaterial_);
	hpRenderer_->SetSize(Vector2(HP_SIZE_X, HP_SIZE_Y));

	//HP_UI
	staMaterial_ = std::make_unique<PixelMaterial>(L"PlayerHp.cso", 2);
	staMaterial_->AddConstBuf({ 1.0f, 0.9f,0.0f,  1.0f });//HP位置(左上)サイズ
	staMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	staMaterial_->AddTextureBuf(staFreamImg_);
	staMaterial_->AddTextureBuf(staMaskImg_);
	staRenderer_ = std::make_unique<PixelRenderer>(*staMaterial_);
	staRenderer_->SetSize(Vector2(HP_SIZE_X, HP_SIZE_Y));


	// アニメーションの設定
	InitAnimation();
	animationController_->Add((int)ANIM_TYPE::GET, Application::PATH_MODEL + L"Player2/Normal/Picking Up.mv1", 50.0f, 0, 0.0f, 210.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::GET, true, 5.0f);
	animationController_->Add((int)ANIM_TYPE::ITEM_DRINK, Application::PATH_MODEL + L"Player2/Normal/Drinking.mv1", 50.0f, 0, 40.0f, 160.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ITEM_DRINK, true, 5.0f);


	//エフェクトの設定
	InitEffect();
	//BGM.SEの設定
	InitSound();
	InitAttrckSound();

	//コントローラーの登録
	inputController_ = std::make_unique<InputController>(SceneManager::GetInstance().GetControllId());


	// 初期状態
	ChangeState(STATE::PLAY);
	isBattle_ = false;
	isDrawWepon_ = false;
	isCloseWepon_ = false;

	//攻撃管理
	isHit_ = false;
	isHitCheck_ = false;
	chageAttrckTime_ = 0.0f;

	//位置情報の変数
	movedPos_ = AsoUtility::VECTOR_ZERO;
	moveDir_ = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;

	//重力兼ジャンプ力
	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 30.0f, 0.0f });
	capsule_->SetRadius(20.0f);
	
	
	//ステータスの初期化
	invisibleTime_ = 0.0f;//無敵時間
	flyigDir_ = AsoUtility::VECTOR_ZERO;
	flyigTime_ = downTime_ = 0.0f;

	//武器ごとのパラメータ設定
	InitPram();


	capsuleWepon_ = std::make_shared<Capsule>(transWeapon_);
	capsuleWepon_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsuleWepon_->SetLocalPosDown({ 0.0f, -30.0f, 0.0f });
	capsuleWepon_->SetRadius(10.0f);



	//採取の際の情報（仮）
	itemId_ = -1;
	poach_ = std::make_unique<ItemPoach>();
}

void Player::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsNew(KEY_INPUT_1)) { demoRot_.x += 1.0f; }
	if (ins.IsNew(KEY_INPUT_2)) { demoRot_.x -= 1.0f; }
	if (ins.IsNew(KEY_INPUT_3)) { demoRot_.y += 1.0f; }
	if (ins.IsNew(KEY_INPUT_4)) { demoRot_.y -= 1.0f; }
	if (ins.IsNew(KEY_INPUT_5)) { demoRot_.z += 1.0f; }
	if (ins.IsNew(KEY_INPUT_6)) { demoRot_.z -= 1.0f; }

	auto& nIns = NetManager::GetInstance();

	//ダメージを受けたらエフェクトストップ
	if (hp_ != hpAgo_) {
		effectController_->Stop(0);
		//体力が０になっていたら
		if (hp_ <= 0) { gameScene_->DownCountPuls(); }
	}
	hpAgo_ = hp_;
	animeAgoType_ = animeType_;

	// 1秒で0にするための減少速度を毎フレーム求める
	const float deltaTime = SceneManager::GetInstance().GetDeltaTime();

	// 自分のプレイヤーのときだけ入力を処理する
	if (key_ == nIns.GetSelf().key) {

		inputController_->Update();

		//回避処理
		if (inputController_->IsTriggered(InputController::KEY::ROLL) && IsInputPlay()
			&& animeType_ != (int)ANIM_TYPE::ROLL && animeType_ != (int)ANIM_TYPE::DAMAGE
			&& animeType_ != (int)ANIM_TYPE::GET&& animeType_ != (int)ANIM_TYPE::ITEM_DRINK
			&& state_ != STATE::HI_DAMAGE && hp_ > 0 && stamina_ > ROLL_TAF)
		{
			stamina_ -= ROLL_TAF;
			invisibleTime_ = INVISIBLE_SMALL_TIME;
			ChangeState(STATE::ROWLING);

		}
		//無敵時間の処理
		if (invisibleTime_ > 0.0f)
		{
			invisibleTime_ -= deltaTime;
		}
		//赤ダメージ
		if (damage_ > 0.0f)
		{
			damage_ -= damage_ * deltaTime;
			if (damage_ < 0.0f)
				damage_ = 0.0f;
		}
		//スタミナ
		stamina_ += staminaDir_ * deltaTime;
		if (staminaMax_ < stamina_) {
			stamina_ = staminaMax_;
		}
		if (0.0f >= stamina_) {
			stamina_ = 0.0f;
			isBreak_ = true;
		}
		if (15.0f < stamina_ && isBreak_)
		{
			isBreak_ = false;
		}
		staminaDir_ = UP_TAF;


		// 更新ステップ
		stateUpdate_();

		if (isBattle_)
		{
			auto& nIns = NetManager::GetInstance();
			//戦闘状態度と教える
			nIns.SetAction(PLAYER_ACTION::IS_BATTLE);
		}

		// 移動方向に応じた回転
		Rotate();

		// ジャンプ処理
		//ProcessJump();

		//スロープ移動量の判定
		//CalcSlope();
		// 重力による移動量
		CalcGravityPow();

		// 衝突判定
		Collision();

		// 歩きエフェクト
		//EffectFootSmoke();


		//カメラ操作
		SceneManager::GetInstance().GetCamera().lock()->ProcessPlayRot(
			inputController_->IsNew(InputController::KEY::R_FORWARD),
			inputController_->IsNew(InputController::KEY::R_BACK),
			inputController_->IsNew(InputController::KEY::R_RIGHT),
			inputController_->IsNew(InputController::KEY::R_LEFT)
		);

		// 重力方向に沿って回転させる
		//transform_.quaRot = grvMng_.GetTransform().quaRot;
		transform_.quaRot = Quaternion();//grvMng_がないので代わりに
		transform_.quaRot = transform_.quaRot.Mult(playerRotY_);


		// 位置送信もここでOK（ProcessMove内でも呼ばれてるけど念のため）
		nIns.SetPostion(key_, transform_.pos);
		nIns.SetPlayRot(key_, transform_.quaRot);

		nIns.SetAnimeType(key_, animeType_);

		nIns.SetNetHp(key_, hp_);
	}
	//通信プレイヤーの処理
	else
	{
		//攻撃と抜刀納刀のみループなし
		int nextAnimeType = nIns.GetAnimeType(key_);
		if (animeType_ != nextAnimeType) {
			animeType_ = nextAnimeType;
		}

		bool loop = !IsLoopAnim();
		animationController_->Play(animeType_, loop);


		const auto& pos = nIns.GetPostion(key_);
		transform_.pos = pos;
		const auto& rot = nIns.GetPlayRot(key_);
		transform_.quaRot = rot;

		/*if (nIns.IsAction(key_, PLAYER_ACTION::IS_ATTRCK))
		{

		}*/

		//チャージ処理
		if (animeType_ == (int)ANIM_TYPE::ATTRCK1STOP)
		{
			chageAttrckTime_ += SceneManager::GetInstance().GetDeltaTime();
			//printfDx("( count=%f,chageAttrckTime_=%f)\n", chageCount_,chageAttrckTime_);
		}
		else
		{
			chageAttrckTime_ = 0.0f;
			chageCount_ = 1.0f;
		}

		hp_ = nIns.GetNetHp(key_);

		//プレイヤーの座標
		const auto& selfPos = nIns.GetPostion(nIns.GetSelf().key);

		//音量設定
		float volume = AsoUtility::CalcVolumeByDistance(selfPos, transform_.pos, MAX_EAR_RADIUS);

		// 無音なら停止
		for (int i = 0; i < static_cast<int>(SE::MAX); i++) {
			SE se = static_cast<SE>(i);
			// ここで se を使う処理を書く
			soundController_->ChengeVolume(i, volume);		// ボリュームだけ更新
		}

	}
	
	//エフェクト処理
#pragma region エフェクト処理

	float rate = chageAttrckTime_ / (CHAGE_MAX_TIME / 4);
	if (rate >= chageCount_ && animeType_ == (int)ANIM_TYPE::ATTRCK1STOP)
	{
		effectController_->Play(0);
		soundController_->Play((int)SE::CHAGE, Sound::TIMES::ONCE, true);

		chageCount_ += 1.0f;
	}
	else if (animeType_ != (int)ANIM_TYPE::ATTRCK1STOP)
	{
		chageCount_ = 1.0f;
	}
	effectController_->Update(0, transform_.pos, AsoUtility::VECTOR_ZERO, 30.0f);
	effectController_->LoopUpdate(1, transWeapon_.pos, AsoUtility::VECTOR_ZERO, 30.0f);
	
	//音の再生
	if (animeAgoType_ != (int)ANIM_TYPE::DRAW 
		&& animeType_== (int)ANIM_TYPE::DRAW)
	{
		soundController_->Play((int)SE::DRAW, Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != (int)ANIM_TYPE::BATTLE_CLOSE 
		&& animeType_== (int)ANIM_TYPE::BATTLE_CLOSE)
	{
		soundController_->Play((int)SE::CLOSE, Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != (int)ANIM_TYPE::DAMAGE 
		&& animeType_== (int)ANIM_TYPE::DAMAGE)
	{
		soundController_->Play((int)SE::DAMAGE, Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != (int)ANIM_TYPE::FLYING 
		&& animeType_== (int)ANIM_TYPE::FLYING)
	{
		soundController_->Play((int)SE::HI_DAMAGE, Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != (int)ANIM_TYPE::DOWN 
		&& animeType_== (int)ANIM_TYPE::DOWN)
	{
		soundController_->Play((int)SE::DOWN, Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ != (int)ANIM_TYPE::ROLL 
		&& animeType_== (int)ANIM_TYPE::ROLL)
	{
		soundController_->Play((int)SE::ROLL, Sound::TIMES::ONCE,true);
	}

	walkTime_ -= SceneManager::GetInstance().GetDeltaTime();
	if (animeAgoType_ == animeType_ &&
		walkTime_ <= 0.0f &&
		(animeType_ == (int)ANIM_TYPE::BTLLE_RUN
			|| animeType_ == (int)ANIM_TYPE::RUN))
	{
		walkTime_ = FOOT_SMOKE;
		soundController_->Play((int)SE::WALK, Sound::TIMES::ONCE);
	}
	else if (animeAgoType_ == animeType_ &&
		walkTime_ <= 0.0f &&
		( animeType_ == (int)ANIM_TYPE::BTLLE_FAST_RUN
			|| animeType_ == (int)ANIM_TYPE::FAST_RUN))
	{
		walkTime_ = FAST_FOOT_SMOKE;
		soundController_->Play((int)SE::RUN, Sound::TIMES::ONCE);
	}

	//攻撃時の音再生
	PlayAttrckSound();

#pragma endregion

	// 描画用の位置は、Draw()でNetManagerから取るからOK
	transform_.Update();

	// アニメーション再生
	animationController_->Update();
	if (animationController_->IsEnd() && animeType_ == (int)ANIM_TYPE::DEAD
		&& gameScene_->GetDownCount() >= GameScene::MAX_DOWN_COUNT)
	{
		// ゲームの勝敗判定
		SceneManager::GAME_RESULT result = SceneManager::GAME_RESULT::GAME_OVER;
		SceneManager::GetInstance().SetGameResult(result);
		SceneManager::GetInstance().CaptureMainScreen();
	}

	//武器の同期
	SyncWeapon();

}

void Player::Draw(void)
{
	auto& nIns = NetManager::GetInstance();

	//const auto& pos = nIns.GetPostion(key_);

	transform_.pos = VAdd(transform_.pos, VScale(transform_.GetForward(), animationController_->GetPos().z));
	transform_.pos = VAdd(transform_.pos, VScale(transform_.GetRight(), animationController_->GetPos().x));
	transform_.Update();
	//武器の同期
	SyncWeapon();

	// モデルの描画
	switch (nIns.GetWeapon(key_))
	{
	case 0:
		MV1DrawModel(transform_.modelId);
		break;
	case 1:
		MV1DrawModel(transform_.modelId);

		//MV1DrawFrame(transform_.modelId, 67);
		break;
	default:
		MV1DrawModel(transform_.modelId);
		break;
	}
	//武器の描画
	WeaponDraw();

	transform_.pos = VSub(transform_.pos, VScale(transform_.GetForward(), animationController_->GetPos().z));
	transform_.pos = VSub(transform_.pos, VScale(transform_.GetRight(), animationController_->GetPos().x));
	transform_.Update();
	//武器の同期
	SyncWeapon();


#ifdef _DEBUG
	// デバッグ用
	DrawDebug();

	auto& pos = animationController_->GetPos();
	DrawFormatString(0, 380, 0x000000, L"ローカル座標(%.2f, %.2f,%2f)", pos.x, pos.y, pos.z);

	inputController_->DebugDraw();

#endif
}
void Player::DrawUI(int i)
{
	
	auto& nIns = NetManager::GetInstance();


	int x = 16;
	int dy = (i * 16);
	int scale = 3;
	int scaleY = 10;

	//自身のUI（他描画なし）
	if (key_ == nIns.GetSelf().key)
	{
		hpMaterial_->SetConstBuf(1, { (float)hp_ / (float)hpMax_, ((float)hp_ + damage_) / (float)hpMax_, 1.0f, 1.0f });
		hpRenderer_->Draw(GAGE_POS_X, GAGE_POS_Y);
		staMaterial_->SetConstBuf(1, { stamina_ / staminaMax_, 0.0f, 1.0f, 1.0f });
		staRenderer_->Draw(GAGE_POS_X, STA_POS_Y);

		//アイテムの表示
		poach_->Draw(0);

	}

	//共通描画
	Material_->SetConstBuf(1, { (float)hp_ / (float)hpMax_, 1.0f, 1.0f, 1.0f });
	Renderer_->Draw(PRAM_POS_X, PRAM_POS_Y + (PRAM_distance_Y * i));


	const auto& pos = nIns.GetPostion(key_);
	const auto& animeType = nIns.GetAnimeType(key_);


	//プレイヤーの座標
	/*const auto& selfPos = nIns.GetPostion(nIns.GetSelf().key);
	DrawFormatString(300, i * 65 + 48, 0x000000, "剣ウェポン回転(%.2f, %.2f,%2f)", ((float)hp_ + damage_) / (float)hpMax_
		, demoRot_.y, demoRot_.z);*/
#ifdef DEBUG

	DrawFormatString(300, i * 65, 0x000000, L"プレイヤー番号(%d)", key_);
	DrawFormatString(300, i * 65 + 16, 0x000000, L"プレイヤー座標(%.2f, %.2f,%2f)", transform_.pos.x, transform_.pos.y, transform_.pos.z);
	DrawFormatString(300, i * 65 + 48, 0x000000, L"剣ウェポン回転(%.2f, %.2f,%2f)", AsoUtility::GetDisPow(selfPos,transform_.pos)
		, demoRot_.y, demoRot_.z);
	DrawFormatString(300, i * 65 + 32, 0x000000, L"アニメーション(%d, %d,%d)", animeType, areaId_, hp_);

#endif // DEBUG

}


void Player::Release(void)
{
}

void Player::Damage(int dama, const VECTOR atkPos, const VECTOR mixDir)
{
	auto& nIns = NetManager::GetInstance();

	if ((animationController_->GetPlayType() == (int)ANIM_TYPE::DAMAGE)
		||state_ == STATE::HI_DAMAGE
		||state_ == STATE::DEAD
		|| invisibleTime_ > 0.0f)
	{
		return;
	}

	movePow_ = AsoUtility::VECTOR_ZERO;


	if (!AsoUtility::EqualsVZero(mixDir))
	{
		flyigTime_ = 1.0f;
		
		// 攻撃者の位置（atkPos）に向けて回転する処理
		VECTOR lookDir = VNorm(VSub(atkPos, transform_.pos)); // 自分 → 攻撃者 のベクトル
		float rotRad = atan2f(lookDir.x, lookDir.z);          // XZ平面でのラジアン角

		// rotRad を使ってワールド空間回転を直接設定する（カメラとは無関係）
		Quaternion lookRot = Quaternion::AngleAxis(rotRad, AsoUtility::AXIS_Y);
		goalQuaRot_ = lookRot;
		playerRotY_ = lookRot; // ←すぐ向かせたいならこの行も追加

		// 吹っ飛び方向（攻撃者 → 自分）も正しく設定
		flyigDir_ = VNorm(VSub(transform_.pos, atkPos));
		flyigDir_ = VNorm(VAdd(flyigDir_, mixDir));
		flyigDir_.y = 0.0f;

		attrckType_ = (int)ANIM_TYPE::FLYING;
		ChangeState(STATE::HI_DAMAGE);
	}
	else
	{
		ChangeState(STATE::DAMAGE);
	}
	
	damage_ = (float)dama;
	//HPからdamage_分減らす
	hp_ -= damage_;
	if (hp_ <= 0) { 
		hp_ = 0; 
		damage_ = 0;
		ChangeState(STATE::DEAD);
		return;
	}
	invisibleTime_ = INVISIBLE_BIG_TIME;//威力・小のときの無敵時間

	//攻撃、納刀モーションをリセット
	AttrckReset();
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
}

std::weak_ptr<Capsule> Player::GetCapsule(void)
{
	return capsuleWepon_;
}


const PLAYER_TYPE& Player::GetPlayerType(void)const
{
	return type_;
}

void Player::InitPram(void)
{
	//メインウェポン
	transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 2.0f);
	// 初期座標
	transWeapon_.pos = prePos_ = { 0.0f, -30.0f, 0.0f };
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal =
		Quaternion::Euler({ AsoUtility::Deg2RadF(WEPON_LOCAL_ROT.x), AsoUtility::Deg2RadF(WEPON_LOCAL_ROT.y), AsoUtility::Deg2RadF(WEPON_LOCAL_ROT.z) });
	transWeapon_.Update();

	//サブウェポン
	//なし

	atkData_.emplace((int)ANIM_TYPE::ATTRCK1S, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK1E, -1.0f, -1.0f
		, -1.0f, true, (int)ANIM_TYPE::ATTRCK1STOP)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK1E, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK2, 16.0f, 24.0f, 22.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK2, std::move(SetAtrckData((int)ANIM_TYPE::ATTRCK3, 21.0f, 40.0f, 38.0f)));
	atkData_.emplace((int)ANIM_TYPE::ATTRCK3, std::move(SetAtrckData(-1, 60.0f, 78.0f)));

	atkData_.emplace((int)ANIM_TYPE::FLYING, std::move(SetAtrckData((int)ANIM_TYPE::DOWN)));
	atkData_.emplace((int)ANIM_TYPE::DOWN, std::move(SetAtrckData((int)ANIM_TYPE::IDLE)));
	atkData_.emplace((int)ANIM_TYPE::IDLE, std::move(SetAtrckData(-1)));
}
void Player::InitAnimation(void)
{

	std::wstring path = Application::PATH_MODEL + L"Player2/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + L"Idle.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::IDLE, true, 5.0f);

	animationController_->Add((int)ANIM_TYPE::RUN, path + L"Run.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + L"FastRun.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::ROLL, path + L"Sprinting Forward Roll.mv1", 45.0f, -1, 0.0f, 32.0f);

	//冬からの新規ブレンド
	animationController_->SetIsBlend((int)ANIM_TYPE::RUN, true, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::FAST_RUN, true, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ROLL, true);


	animationController_->Add((int)ANIM_TYPE::DRAW, path + L"Draw Great Sword 1.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::BATTLE_DRAW, path + L"Draw Great Sword 2.mv1", 30.0f);

	animationController_->Add((int)ANIM_TYPE::CLOSE, path + L"Draw Great Sword 1.mv1", 30.0f, -1, 13.0f, 0.1f);
	animationController_->Add((int)ANIM_TYPE::BATTLE_CLOSE, path + L"Draw Great Sword 2.mv1", 30.0f, -1, 24.0f, 0.1f);

	animationController_->SetIsBlend((int)ANIM_TYPE::DRAW, true);
	animationController_->SetIsBlend((int)ANIM_TYPE::BATTLE_CLOSE, true);


	animationController_->Add((int)ANIM_TYPE::BTLLE_IDLE, path + L"BattleIdle.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_IDLE, true, 10.0f);

	animationController_->Add((int)ANIM_TYPE::BTLLE_RUN, path + L"BattleRun.mv1", 30.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_RUN, true, 10.0f);

	animationController_->Add((int)ANIM_TYPE::BTLLE_FAST_RUN, path + L"Sword And Shield Run.mv1", 30.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::BTLLE_FAST_RUN, true, 10.0f);

	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + L"Great Sword Impact.mv1", 30.0f);

	animationController_->Add((int)ANIM_TYPE::FLYING, path + L"Flying.mv1", 20.0f, -1, 20.0f, 0.1f);
	animationController_->SetIsBlend((int)ANIM_TYPE::FLYING, true);

	animationController_->Add((int)ANIM_TYPE::DOWN, path + L"Down.mv1", 20.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::DOWN, true, 3.0f);

	//攻撃
	animationController_->Add((int)ANIM_TYPE::ATTRCK1S, path + L"Great Sword Slash (1).mv1", 20.0f, -1, 0.0f, 13.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK1S, true);
	animationController_->Add((int)ANIM_TYPE::ATTRCK1STOP, path + L"Great Sword Slash (1).mv1", 20.0f, -1, 13.0f, 13.0f);
	animationController_->Add((int)ANIM_TYPE::ATTRCK1E, path + L"Great Sword Slash (1).mv1", 20.0f, -1, 13.0f);

	animationController_->Add((int)ANIM_TYPE::ATTRCK2, path + L"Great Sword Slash (2).mv1", 30.0f, -1, 10.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK2, true);

	animationController_->Add((int)ANIM_TYPE::ATTRCK3, path + L"Great Sword Casting.mv1", 40.0f);
	animationController_->SetIsBlend((int)ANIM_TYPE::ATTRCK3, true);

	animationController_->Add((int)ANIM_TYPE::DEAD, path + L"Dying.mv1", 30.0f);

	animationController_->Play((int)ANIM_TYPE::IDLE);
	animeType_ = (int)ANIM_TYPE::IDLE;
	animeAgoType_ = animeType_;
}
void Player::InitEffect(void)
{
	std::wstring path = Application::PATH_EFFECT;
	effectController_ = std::make_unique<EffectController>();
	//ためアニメーション
	effectController_->Add(0, path + L"PowerUp/PowerUp.efkefc");

	effectController_->Add(1, path + L"Slash/Slash.efkefc");
	effectController_->Play(1);

}
void Player::InitSound(void)
{
	std::wstring path = Application::PATH_SOUND;
	soundController_ = std::make_unique<SoundController>();

	soundController_->Add((int)SE::CHAGE, path + L"Player/Chage.mp3", 1.0f);
	soundController_->Add((int)SE::DRAW, path + L"Wepon/Draw.mp3", 0.6f);
	soundController_->Add((int)SE::CLOSE, path + L"Wepon/Close.mp3", 0.6f);
	soundController_->Add((int)SE::WALK, path + L"Player/Walk.mp3", 0.6f);
	soundController_->Add((int)SE::RUN, path + L"Player/Run.mp3", 0.6f);
	soundController_->Add((int)SE::ROLL, path + L"Player/Roll.mp3", 0.6f);
	soundController_->Add((int)SE::DAMAGE, path + L"Player/Damage.mp3", 0.6f);
	soundController_->Add((int)SE::HI_DAMAGE, path + L"Player/HiDamage.mp3", 0.6f);
	soundController_->Add((int)SE::DOWN, path + L"Player/Down.mp3.mp3", 0.6f);

}
void Player::InitAttrckSound(void)
{
	std::wstring path = Application::PATH_SOUND;

	soundController_->Add((int)SE::ATTRCK1, path + L"Player/GreatSowrd.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK2, path + L"Player/GreatSowrd.mp3", 0.6f);
	soundController_->Add((int)SE::ATTRCK3, path + L"Player/GreatSowrd.mp3", 0.6f);
}

void Player::PlayAttrckSound(void)
{
	if (animeType_ == (int)ANIM_TYPE::ATTRCK1E
		&& animationController_->GetStepTime() > 16.0f
		&& animationController_->GetStepTime() < 16.5f)
	{
		soundController_->Play((int)SE::ATTRCK1, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK2
		&& animationController_->GetStepTime() > 21.0f
		&& animationController_->GetStepTime() < 21.5f)
	{
		soundController_->Play((int)SE::ATTRCK2, Sound::TIMES::ONCE);
	}
	else if (animeType_ == (int)ANIM_TYPE::ATTRCK3
		&& animationController_->GetStepTime() > 60.0f
		&& animationController_->GetStepTime() < 60.5f)
	{
		soundController_->Play((int)SE::ATTRCK3, Sound::TIMES::ONCE);
	}

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
void Player::ChangeStateWepon(void)
{
	stateUpdate_ = std::bind(&Player::UpdateWepon, this);
}
void Player::ChangeStateBattle(void)
{
	isBattle_ = true;
	stateUpdate_ = std::bind(&Player::UpdateBattle, this);
}
void Player::ChangeStateAttrck(void)
{
	stateUpdate_ = std::bind(&Player::UpdateAttrck, this);
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

//stateがNONEの時のUpdate
void Player::UpdateNone(void)
{
}
//stateがPLAYの時のUpdate
void Player::UpdatePlay(void)
{
	// 移動処理
	ProcessMove();
}
//stateがBATTLEの時のUpdate
void Player::UpdateBattle(void)
{
#pragma region 攻撃処理
	
	//抜刀してすぐ攻撃できるようIsNewを使用
	if (inputController_->IsNew(InputController::KEY::ATTRCK) && IsInputPlay())
	{
		isHit_ = false;
		isHitCheck_ = false;
		movePow_ = AsoUtility::VECTOR_ZERO;


		attrckType_ = (int)ANIM_TYPE::ATTRCK1S;
		ChangeState(STATE::ATTRCK);
		return;
	}

#pragma endregion


	// 移動処理(攻撃も含む)
	ProcessBattleMove();

#ifdef _DEBUG

	InputManager& ins = InputManager::GetInstance();

	if (ins.IsNew(KEY_INPUT_1)) { demoRot_.x += 1.0f; }
	if (ins.IsNew(KEY_INPUT_2)) { demoRot_.x -= 1.0f; }
	if (ins.IsNew(KEY_INPUT_3)) { demoRot_.y += 1.0f; }
	if (ins.IsNew(KEY_INPUT_4)) { demoRot_.y -= 1.0f; }
	if (ins.IsNew(KEY_INPUT_5)) { demoRot_.z += 1.0f; }
	if (ins.IsNew(KEY_INPUT_6)) { demoRot_.z -= 1.0f; }

#endif // _DEBUG
}
void Player::UpdateWepon(void)
{
	if (isBattle_)
	{
		if (isDrawWepon_)
		{
			animationController_->Play((int)ANIM_TYPE::BATTLE_DRAW, false);
			animeType_ = (int)ANIM_TYPE::BATTLE_DRAW;

			if (animationController_->IsEnd())
			{
				isDrawWepon_ = false;

				ChangeState(STATE::BATTLE);
				
			}
		}
		else if (isCloseWepon_)
		{
			animationController_->Play((int)ANIM_TYPE::BATTLE_CLOSE, false);
			animeType_ = (int)ANIM_TYPE::BATTLE_CLOSE;

			if (animationController_->IsEnd())
			{
				isBattle_ = false;
			}
		}
	}
	else
	{
		if (isDrawWepon_)
		{
			animationController_->Play((int)ANIM_TYPE::DRAW, false);
			animeType_ = (int)ANIM_TYPE::DRAW;

			if (animationController_->IsEnd())
			{
				
				isBattle_ = true;
				//抜刀してすぐ攻撃できるようIsNewを使用
				if (inputController_->IsNew(InputController::KEY::ATTRCK))
				{
					isDrawWepon_ = false;

					isHit_ = false;
					isHitCheck_ = false;
					movePow_ = AsoUtility::VECTOR_ZERO;


					attrckType_ = (int)ANIM_TYPE::ATTRCK1S;
					ChangeState(STATE::ATTRCK);
					return;
				}
			}
		}
		else if (isCloseWepon_)
		{
			animationController_->Play((int)ANIM_TYPE::CLOSE, false);
			animeType_ = (int)ANIM_TYPE::CLOSE;

			if (animationController_->IsEnd())
			{
				isCloseWepon_ = false;
				ChangeState(STATE::PLAY);
			}
		}
	}

	if (inputController_->IsNew(InputController::KEY::FORWARD)
		|| inputController_->IsNew(InputController::KEY::LEFT)
		|| inputController_->IsNew(InputController::KEY::BACK)
		|| inputController_->IsNew(InputController::KEY::RIGHT)) {
	}
	else{ movePow_ = AsoUtility::VECTOR_ZERO; }

}
void Player::UpdateAttrck(void)
{
	AttrckUpdate();
}
void Player::UpdateRowling(void)
{
	staminaDir_ = 0.0f;
	movePow_ = VScale(transform_.GetForward(), SPEED_ROLL);

	ChangeStateAnimeEnd(ANIM_TYPE::ROLL);


}
void Player::UpdateDamage(void)
{
	//アニメーションが終わったらPLAYかBATTLEに...
	ChangeStateAnimeEnd(ANIM_TYPE::DAMAGE);

}
void Player::UpdateHiDamage(void)
{
	if (flyigTime_ > 0.0f)
	{
		flyigTime_ -= SceneManager::GetInstance().GetDeltaTime() * 2.0f;

		animationController_->Play(attrckType_, true);
		movePow_ = VScale(flyigDir_, SPEED_JUMP * (flyigTime_ + 0.3f));

		if (flyigTime_ < 0.0f)
		{
			movePow_ = AsoUtility::VECTOR_ZERO;
			attrckType_ = atkData_[attrckType_]->nextAttrck;
			animeType_ = attrckType_;
			downTime_ = DOWN_MAX;
		}
		else animeType_ = attrckType_;

		return;
	}
	else if (downTime_ > 0.0f)
	{
		downTime_ -= SceneManager::GetInstance().GetDeltaTime();

		animationController_->Play(attrckType_, true);

		if (downTime_ < 0.0f)
		{
			attrckType_ = atkData_[attrckType_]->nextAttrck;
			animeType_ = attrckType_;
		}
		else animeType_ = attrckType_;

		return;
	}

	//吹っ飛ばしが終わった後
	animationController_->Play(attrckType_, false);

	if (!animationController_->IsBlend())
	{
		int x = 0;
		if (atkData_[attrckType_]->nextAttrck <= 0)
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

		attrckType_ = atkData_[attrckType_]->nextAttrck;
		animeType_ = attrckType_;

	}
	else
	{
		animeType_ = attrckType_;
	}

}
void Player::UpdateDead(void)
{
	isBattle_ = false;
	animationController_->Play((int)ANIM_TYPE::DEAD, false);
	animeType_ = (int)ANIM_TYPE::DEAD;

	if (animationController_->IsEnd())
	{
		transform_.pos = { 1200.0f,0.0f,-5000.0f };
		hp_ = hpAgo_ = hpMax_ = MAX_HP;
		AttrckReset();
		ChangeState(STATE::PLAY);
	}
}

void Player::UpdateGet(void)
{
	if (animationController_->IsEnd())
	{
		if (itemId_ == 1)
		{
			poach_->AddItem(0);
		}
	}
	ChangeStateAnimeEnd(ANIM_TYPE::GET);
}

void Player::UpdateItemUse(void)
{
	if (animationController_->IsEnd())
	{
		hp_ += 20;
		if (hp_ > hpMax_)
		{
			hp_ = hpMax_;
		}
		poach_->PlayItem(0);

	}
	ChangeStateAnimeEnd(ANIM_TYPE::ITEM_DRINK);
}

#pragma endregion



#pragma region 移動関係

void Player::ProcessMove(void)
{
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	bool isAction = false;
	//auto& nIns = NetManager::GetInstance();

	if (inputController_->IsNew(InputController::KEY::FORWARD)) { dir = VAdd(dir, AsoUtility::DIR_F); }
	if (inputController_->IsNew(InputController::KEY::LEFT)) { dir = VAdd(dir, AsoUtility::DIR_L); }
	if (inputController_->IsNew(InputController::KEY::BACK)) { dir = VAdd(dir, AsoUtility::DIR_B); }
	if (inputController_->IsNew(InputController::KEY::RIGHT)) { dir = VAdd(dir, AsoUtility::DIR_R); }

	//移動方向の向き設定
	float rotRad = CreateRad(dir);
	
	//抜刀じゃないなら
	if (IsInputPlay())
	{
		//抜刀処理
		if (inputController_->IsTriggered(InputController::KEY::DRAW))
		{
			if (!AsoUtility::EqualsVZero(movePow_))
			{
				speed_ = SPEED_MOVE;

				movePow_ = VScale(VNorm(movePow_), speed_);
			}
			movePow_ = AsoUtility::VECTOR_ZERO;//抜刀時移動しない（帰るなら戻す）

			isDrawWepon_ = true;
			ChangeState(STATE::WEPON);
			return;
		}
		bool isId = false;
		for (const auto c : colliders_)
		{
			if (c.lock()->type_ == Collider::TYPE::ITEM)
			{
				
				if (AsoUtility::IsHitSpheres(transform_.pos, capsule_->GetRadius(), c.lock()->pos_, c.lock()->radius_))
				{
					isId = true;
				}
				
				//採取処理
				if (inputController_->IsNew(InputController::KEY::GET)&& 
					AsoUtility::IsHitSpheres(transform_.pos, capsule_->GetRadius(), c.lock()->pos_, c.lock()->radius_))
				{
					speed_ = SPEED_MOVE;
					movePow_ = AsoUtility::VECTOR_ZERO;//抜刀時移動しない（帰るなら戻す）

					ChangeState(STATE::GET);
					return;
				}
			}
		}
		if (!isId)
		{
			itemId_ = -1;
		}

		//採取処理
		if (inputController_->IsTriggered(InputController::KEY::USE))
		{
			speed_ = SPEED_MOVE;
			movePow_ = AsoUtility::VECTOR_ZERO;//抜刀時移動しない（帰るなら戻す）

			ChangeState(STATE::ITEM_PLAY);
			return;
		}

		movePow_ = AsoUtility::VECTOR_ZERO;

		if ((!AsoUtility::EqualsVZero(dir)) /*&& (isJump_ || IsEndLanding())*/)
		{
			//入力された方向をかめらの回転情報を使って
			//カメラの進行方向に変換する
			Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();
			dir = cameraRot.PosAxis(dir);


			// 移動処理
			speed_ = SPEED_MOVE;
			bool isDash = inputController_->IsNew(InputController::KEY::DASH);
			if (isDash && !isBreak_) {
				speed_ = SPEED_RUN;
				staminaDir_ = DOWN_TAF;
			}

			movePow_ = VScale(dir, speed_);

			// 回転処理
			SetGoalRotate(rotRad);


			if (IsEndLanding())//モーションが変えていいやつか？
			{
				// アニメーション
				if (isDash)
				{
					animationController_->Play((int)ANIM_TYPE::FAST_RUN);
					animeType_ = (int)ANIM_TYPE::FAST_RUN;
				}
				else
				{
					animationController_->Play((int)ANIM_TYPE::RUN);
					animeType_ = (int)ANIM_TYPE::RUN;
				}
			}
		}
		else
		{
			if (IsEndLanding())
			{
				animationController_->Play((int)ANIM_TYPE::IDLE);
				animeType_ = (int)ANIM_TYPE::IDLE;
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
	

	// 左向き
	//isAction = InputManager::GetInstance().IsNew(keyConfig_.LEFT);
	//if (nIns.IsAction(key_, PLAYER_ACTION::MOVE_LEFT, isAction))
	//{
	//	transform_.pos.x -= speed_;
	//	if (key_ == nIns.GetSelf().key) 
	//	{
	//		//ネットに左へ移動したと伝える
	//		nIns.SetAction(PLAYER_ACTION::MOVE_LEFT);
	//	}
	//}


}
void Player::ProcessBattleMove(void)
{
	InputManager& ins = InputManager::GetInstance();
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	bool isAction = false;
	auto& nIns = NetManager::GetInstance();


	if (inputController_->IsNew(InputController::KEY::FORWARD)) { dir = VAdd(dir, AsoUtility::DIR_F); }
	if (inputController_->IsNew(InputController::KEY::LEFT)) { dir = VAdd(dir, AsoUtility::DIR_L);}
	if (inputController_->IsNew(InputController::KEY::BACK)) { dir = VAdd(dir, AsoUtility::DIR_B);}
	if (inputController_->IsNew(InputController::KEY::RIGHT)) { dir = VAdd(dir, AsoUtility::DIR_R);}
	
	//移動方向の向き設定
	float rotRad = CreateRad(dir);

	if (IsInputPlay())
	{
		if (inputController_->IsTriggered(InputController::KEY::CLOSE))
		{
			isCloseWepon_ = true;
			ChangeState(STATE::WEPON);
			movePow_ = AsoUtility::VECTOR_ZERO;//納刀時移動しない（帰るなら戻す）

			return;
		}

		movePow_ = AsoUtility::VECTOR_ZERO;


		if ((!AsoUtility::EqualsVZero(dir)) /*&& (isJump_ || IsEndLanding())*/)
		{
			//入力された方向をかめらの回転情報を使って
			//カメラの進行方向に変換する
			Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();
			dir = cameraRot.PosAxis(dir);


			// 移動処理
			speed_ = SPEED_MOVE;
			bool isDash = (inputController_->IsNew(InputController::KEY::DASH) && isBattleDash_);
			if (isDash && !isBreak_) {
				speed_ = SPEED_RUN;
				staminaDir_ = DOWN_TAF;
			}

			movePow_ = VScale(dir, speed_);

			// 回転処理
			SetGoalRotate(rotRad);


			if (IsEndLanding())//モーションが変えていいやつか？
			{
				// アニメーション
				if (isDash)
				{
					animationController_->Play((int)ANIM_TYPE::BTLLE_FAST_RUN);
					animeType_ = (int)ANIM_TYPE::BTLLE_FAST_RUN;
				}
				else
				{
					animationController_->Play((int)ANIM_TYPE::BTLLE_RUN);
					animeType_ = (int)ANIM_TYPE::BTLLE_RUN;
				}
			}
		}
		else
		{
			if (IsEndLanding())
			{
				animationController_->Play((int)ANIM_TYPE::BTLLE_IDLE);
				animeType_ = (int)ANIM_TYPE::BTLLE_IDLE;
			}
		}
	}



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

void Player::SetGoalRotate(double rotRad)
{
	VECTOR cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetAngles();

	Quaternion axis =
		Quaternion::AngleAxis(
			(double)cameraRot.y + rotRad, AsoUtility::AXIS_Y);

	// 現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;
}
//↑プレイヤーに向かせたい、ゴールとなる回転を設定する
void Player::Rotate(void)
{
	//回転の球面補間を行う。
	//TIME_ROT定数で指定された時間をかけて、ゆっくりゴールとなる
	//回転に向かって近づくような回転を行う
	stepRotTime_ -= SceneManager::GetInstance().GetDeltaTime();
	// 回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}
float Player::CreateRad(const VECTOR& dir)
{
	float angle = atan2f(dir.x, dir.z);
	if (angle < 0.0f) {
		angle += DX_TWO_PI; // 0~2paiに正規化
	}
	return angle;
}

#pragma endregion

//武器の同期
const void Player::SyncWeapon()
{
	auto& nIns = NetManager::GetInstance();
#pragma region 武器の同期

	if (key_ == nIns.GetSelf().key) {
		if (isBattle_)
		{
			SyncWeaponBattle();
		}
		else
		{
			SyncWeaponPlay();
		}
	}
	else
	{
		if (nIns.IsAction(key_, PLAYER_ACTION::IS_BATTLE))
		{
			SyncWeaponBattle();
		}
		else
		{
			SyncWeaponPlay();
		}
	}

#pragma endregion
}


void Player::WeaponDraw()
{
	MV1DrawModel(transWeapon_.modelId);
}
void Player::SyncWeaponPlay()
{
#pragma region 武器の同期＿非戦闘時

	// メインウェポン（腰）
	SyncWeaponToFream(L"mixamorig:Spine2", GSOWRD_SPINE_ROT, GSOWRD_SPINE_POS,
		transform_, transWeapon_);

#pragma endregion
}
void Player::SyncWeaponBattle()
{
#pragma region 武器の同期（戦闘時）

	// メインウェポン（右手）
	SyncWeaponToFream(L"mixamorig:RightHandMiddle1", GSOWRD_HAND_ROT, GSOWRD_HAND_POS,
		transform_, transWeapon_);

#pragma endregion
}

//オブジェクトのフレーム追従♭
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
	if (frmNo == -1) {
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
		MGetRotAxis(VNorm(VTransformSR(AsoUtility::DIR_R, handWorldMatrix)), AsoUtility::Deg2RadF(offsetRot.x)));
	mixMat = MMult(mixMat,
		MGetRotAxis(VNorm(VTransformSR(AsoUtility::DIR_U, handWorldMatrix)), AsoUtility::Deg2RadF(offsetRot.y)));
	mixMat = MMult(mixMat,
		MGetRotAxis(VNorm(VTransformSR(AsoUtility::DIR_F, handWorldMatrix)), AsoUtility::Deg2RadF(offsetRot.z)));

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

//当たり判定
#pragma region コリジョン関数

void Player::CollisionGravity(void)
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

	const auto gravHitPosUp = VAdd(gravHitPosUp_, VScale(transform_.GetForward(), COLL_LEG_RATE));
	const auto gravHitPosDown =VAdd(gravHitPosDown_, VScale(transform_.GetForward(), COLL_LEG_RATE));

	for (const auto c : colliders_)
	{
		if (c.lock()->type_ == Collider::TYPE::STAGE)
		{
			// 地面との衝突
			auto hit = MV1CollCheck_Line(
				c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
			auto hitF = MV1CollCheck_Line(
				c.lock()->modelId_, -1, gravHitPosUp, gravHitPosDown);

			// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
			//if (hit.HitFlag > 0)
			if ((hit.HitFlag > 0|| hitF.HitFlag > 0) && VDot(dirGravity, jumpPow_) > 0.9f)
			{
				// 使用する衝突位置を選ぶ
				//VECTOR usedHitPosition = hit.HitFlag > 0 ? hit.HitPosition : hitF.HitPosition;
				VECTOR usedHitPosition = hit.HitFlag > 0 ? hit.HitPosition : VSub(hitF.HitPosition, VScale(transform_.GetForward(), COLL_LEG_RATE));

				// 衝突地点から、少し上に移動
				movedPos_ = VAdd(usedHitPosition, VScale(dirUpGravity, 2.0f));

				// ジャンプリセット
				jumpPow_ = AsoUtility::VECTOR_ZERO;

				// 着地モーション

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

	// playerとの衝突判定
	VECTOR diff = VSub(transform_.pos, pos);

	float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	if (disPow < capsule_->GetRadius() * r)//ダメージ半径×攻撃半径
	{
		return true;
	}

	return false;
}

#pragma endregion


//攻撃状態の判定
const bool Player::IsAttrck(void)const
{
	return(state_ == STATE::ATTRCK);
}
const bool Player::IsLoopAnim(void)
{
	if (animeType_ == (int)ANIM_TYPE::ATTRCK1S ||
		animeType_ == (int)ANIM_TYPE::ATTRCK1STOP ||
		animeType_ == (int)ANIM_TYPE::ATTRCK1E ||
		animeType_ == (int)ANIM_TYPE::ATTRCK2 ||
		animeType_ == (int)ANIM_TYPE::ATTRCK3 ||
		animeType_ == (int)ANIM_TYPE::BATTLE_CLOSE ||
		animeType_ == (int)ANIM_TYPE::BATTLE_DRAW ||
		animeType_ == (int)ANIM_TYPE::CLOSE ||
		animeType_ == (int)ANIM_TYPE::DRAW ||
		animeType_ == (int)ANIM_TYPE::DEAD
		)
	{
		return true;
	}

	return false;
}

bool Player::IsSelf(void)
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

//操作をしなくても終わるまで動くモーションのときに使用
bool Player::IsEndLanding(void)
{
	bool ret = true;
	return ret;
	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::ATTRCK1S)
	{
		return ret;
	}
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::ATTRCK1S)
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
//操作が可能か
const bool Player::IsInputPlay(void) const
{
	// アニメーションが終了しているか
	//もし抜刀、納刀、攻撃してないなら操作可能（true）
	if (!isDrawWepon_ && !isCloseWepon_ && state_ != STATE::ATTRCK)
	{
		return true;
	}
	return false;
}
//通常状態に戻す
void Player::AttrckReset(void)
{
	isDrawWepon_ = false;
	isCloseWepon_ = false;

	chageAttrckTime_ = 0.0f;

}

//特定のアニメーションが終わったらIdleに戻す処理
void Player::ChangeStateAnimeEnd(const ANIM_TYPE anim)
{
	//バトル状態なら
	if (isBattle_)
	{
		animationController_->Play((int)anim, false);
		animeType_ = (int)anim;

		if (animationController_->IsEnd())
		{
			ChangeState(STATE::BATTLE);
		}
	}
	//通常状態なら
	else
	{
		animationController_->Play((int)anim, false);
		animeType_ = (int)anim;

		if (animationController_->IsEnd())
		{
			ChangeState(STATE::PLAY);
		}
	}

}

void Player::DrawDebug(void)
{
	//DrawFormatString(0, 20, 0x000000, "移動速度　：%.2f", speed_);
	DrawLine3D(prePos_, transform_.pos, 0xf0f000);

	// カプセルコライダ
	capsule_->Draw();
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0xffffff);
	const auto gravHitPosUp = VAdd(gravHitPosUp_, VScale(transform_.GetForward(), COLL_LEG_RATE));
	const auto gravHitPosDown = VAdd(gravHitPosDown_, VScale(transform_.GetForward(), COLL_LEG_RATE));
	DrawLine3D(gravHitPosUp, gravHitPosDown, 0xffffff);

	//InputManager& ins = InputManager::GetInstance();
	capsuleWepon_->Draw();

	auto r = inputController_->IsNew(InputController::KEY::DASH);

	//DrawFormatString(100, 132, 0xFFFFFF, "Draw : %s", isDrawWepon_ ? "true" : "false");
	//DrawFormatString(100, 116, 0xFFFFFF, "Close: %s", isCloseWepon_ ? "true" : "false");
	DrawFormatString(100, 116, 0xFFFFFF, L"Attrck: %s", inputController_->IsNew(InputController::KEY::ATTRCK) ? L"true" : L"false");
	DrawFormatString(100, 132, 0xFFFFFF, L"ChageT: %.2f", chageAttrckTime_);
	//倍率設定
	int rate = chageAttrckTime_ / (CHAGE_MAX_TIME / 4.0f);
	
	DrawFormatString(100, 148, 0xFFFFFF, L"ChageR: %d", rate);
	DrawFormatString(100, 164, 0xFFFFFF, L"Damage: %.2f", attrckDamage_ * attrckRate_);
	DrawFormatString(100, 180, 0xFFFFFF, L"Postion: X_%.2fY_%.2fZ_%.2f", transform_.pos.x,transform_.pos.y,transform_.pos.z);
}

void Player::AttrckUpdate(void)
{
	animationController_->Play(attrckType_, false);

	//操作を受け付けるか
	if (animationController_->GetStepTime() > atkData_[attrckType_]->sNewTime 
		&& atkData_[attrckType_]->sNewTime > 0.0f)
	{
		if (inputController_->IsTriggered(InputController::KEY::ATTRCK)
			&& atkData_[attrckType_]->nextAttrck != -1)
		{
			isHitCheck_ = false;
			isHit_ = false;

			attrckType_ = atkData_[attrckType_]->nextAttrck;
			return;
		}
		else if (inputController_->IsTriggered(InputController::KEY::ROLL)
			&&stamina_> ROLL_TAF)
		{
			stamina_ -= ROLL_TAF;
			isHitCheck_ = false;
			chageAttrckTime_ = 0.0f;

			invisibleTime_ = INVISIBLE_SMALL_TIME;
			ChangeState(STATE::ROWLING);
			return;
		}
	}

	//当たり判定が発生するか
	if (atkData_[attrckType_]->sHitTime < animationController_->GetStepTime()
		&& atkData_[attrckType_]->HitTime > animationController_->GetStepTime())
	{

		effectController_->Update(1, capsuleWepon_->GetCenter());

		isHitCheck_ = true;
	}

	if (animationController_->IsEnd())
	{
		//チャージするものか
		if (atkData_[attrckType_]->isCharge)
		{
			if (inputController_->IsNew(InputController::KEY::ATTRCK) && chageAttrckTime_ <= CHAGE_MAX_TIME)
			{
				//チャージ中なら回転可能
				VECTOR dir = AsoUtility::VECTOR_ZERO;

				if (inputController_->IsNew(InputController::KEY::FORWARD)) { dir = VAdd(dir, AsoUtility::DIR_F); }
				if (inputController_->IsNew(InputController::KEY::LEFT)) { dir = VAdd(dir, AsoUtility::DIR_L); }
				if (inputController_->IsNew(InputController::KEY::BACK)) { dir = VAdd(dir, AsoUtility::DIR_B); }
				if (inputController_->IsNew(InputController::KEY::RIGHT)) { dir = VAdd(dir, AsoUtility::DIR_R); }

				//移動方向の向き設定
				float rotRad = CreateRad(dir);
				if ((!AsoUtility::EqualsVZero(dir)) /*&& (isJump_ || IsEndLanding())*/)
				{
					//入力された方向をかめらの回転情報を使って
					//カメラの進行方向に変換する
					
					Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();
					dir = cameraRot.PosAxis(dir);

					// 回転処理
					SetGoalRotate(rotRad);
				}
				//チャージ処理
				chageAttrckTime_ += SceneManager::GetInstance().GetDeltaTime();
				animeType_ = atkData_[attrckType_]->chargeId;
			}
			else//チャージ終了なら次のアニメへ(攻撃へ)
			{
				//倍率設定
				float rate = chageAttrckTime_ / (CHAGE_MAX_TIME / 4);
				if (rate >= 4)rate = 0.5f;
				attrckRate_ = 1.0f + (rate * 0.3f);

				
				attrckType_ = atkData_[attrckType_]->nextAttrck;
				animeType_ = attrckType_;

				return;
			}
		}
		else//アニメーションが終わってチャージ処理もないなら終了
		{
			animationController_->Play((int)ANIM_TYPE::BTLLE_IDLE);
			animeType_ = (int)ANIM_TYPE::BTLLE_IDLE;

			chageAttrckTime_ = 0.0f;
			ChangeState(STATE::BATTLE);
			return;

		}
	}
	else
	{
		animeType_ = attrckType_;
	}
}
