#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Utility/Measure.h"
#include "../Manager/SoundManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/Camera.h"
#include "../Common/Fader.h"
#include "../Net/NetStructures.h"
#include "../Net/NetManager.h"
#include "../Object/Common/Timer.h"
// プレイヤー関連
#include "../Object/Player/Player.h"
#include "../Object/Player/Sword.h"
#include "../Object/Player/GreatSword.h"
#include "../Object/Player/Arrow.h"
// 投げ物関連
#include "../Object/Shot/BomObject.h"
#include "../Object/Shot/ItemShot.h"
#include "../Object/Shot/ArrowShot.h"
#include "../Object/Shot/ShotBase.h"
// 敵関連
#include "../Object/Enemy/Boss.h"
#include "../Object/Enemy/SmallMonster.h"
// ステージ関連
#include "../Object/Stage/Stage.h"
#include "../Object/Stage/Planet.h"
#include "../Object/Stage/SkyDome.h"
#include "../Object/Stage/Grid.h"
// シェーダ
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "GameScene.h"

namespace
{
	// タイマーの描画位置
	constexpr int TIME_POS_X = Timer::SIZE_X / 2 + 10;
	constexpr int TIME_POS_Y = Timer::SIZE_Y / 2 + 10;
	// 制限時間
	constexpr float LIMIT_TIME = 60.0f * 5.0f;
	// フェードスピード
	constexpr float FLASH_FADE_SPEED = 15.0f;
	// 武器ID
	constexpr int SOWRD_ID = 0;
	constexpr int GREAT_SOWRD_ID = 1;
	constexpr int ARROW_ID = 2;
	// 爆発ダメージ倍率（プレイヤー用）
	constexpr float BLAST_DAMAGE_RATE = 0.1f;
	// エリア移動用テキストID
	const std::string AREA_FIRST = "1to2";
	const std::string AREA_SECOND = "2to1";
	// デバッグ用
	constexpr int DEBUG_DRAW_POS_X = 100;
	constexpr int DEBUG_DRAW_POS_Y = 100;
	constexpr int WHITE_COLOR = 0xffffff;
	constexpr int RED_COLOR = 0xff0000;
	// サウンドの音量変化
	constexpr float SOUND_RATE_SPEED = 0.3f;
	// 吹っ飛び方向の補正値
	constexpr float DAMAGE_VEC_RATE = 0.3f;
	// カメラシェイク
	constexpr float CAMERA_SHAKE_TIME = 0.5f;
	constexpr float CAMERA_SHAKE_POWER = 1.5f;
}

GameScene::GameScene(void)
	:
	SceneBase(),
	timer_(nullptr),
	stage_(nullptr),
	boss_(nullptr),
	soundRate_(0.0f),
	fader_(nullptr),
	textId_(""),
	stepId_(0),
	downCnt_(0),
	stepCountDown_(0.0f),
	minDist_(START_MIN_DIST),
	grid_(nullptr),
	skyDome_(nullptr)
{
	// プレイヤークラスの解放
	players_.clear();
}

GameScene::~GameScene(void)
{
	// Shotクラスの解放
	shots_.clear();
	// 経過時間のリセット
	SceneManager::GetInstance().SetTotalGameTime(0.0f);
	// 音の停止
	SoundManager::GetInstance().AllStop();
}

void GameScene::Init(void)
{
	// 初期化
	GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);

	auto& users = NetManager::GetInstance().GetNetUsers();
	auto& nIns = NetManager::GetInstance();

	// フラッシュ用フェードクラスの生成
	fader_ = std::make_unique<Fader>(0xffffff);
	fader_->Init(FLASH_FADE_SPEED);

	// ボスの生成
	boss_ = std::make_unique<Boss>(NetManager::GetInstance().GetHost().key, 0);
	boss_->Init();

	// モンスターの設定
	// 1体目
	auto mons = std::make_shared<SmallMonster>(NetManager::GetInstance().GetHost().key, 1);
	mons->Init();
	monsters_.emplace_back(mons);
	// 2体目
	mons = std::make_shared<SmallMonster>(NetManager::GetInstance().GetHost().key, 2);
	mons->Init();
	monsters_.emplace_back(mons);
	// 3体目
	mons = std::make_shared<SmallMonster>(NetManager::GetInstance().GetHost().key, 3);
	mons->Init();
	monsters_.emplace_back(mons);

	// 移動用の変数初期化
	textId_ = "";
	stepId_ = 0;

	for (auto& user : users)
	{
		auto player = std::make_shared<Player>(user.first, this, user.second.playerType);

		// モデルの基本設定
		switch (nIns.GetWeapon(user.first))
		{
		case SOWRD_ID:
			player = std::make_shared<Sword>(user.first, this, user.second.playerType);
			break;
		case GREAT_SOWRD_ID:
			player = std::make_shared<GreatSword>(user.first, this, user.second.playerType);
			break;
		case ARROW_ID:
			player = std::make_shared<Arrow>(user.first, this, user.second.playerType);
			break;
		default:
			break;
		}

		player->Init();

		// 自分用のクラス	
		if (user.first == NetManager::GetInstance().GetSelf().key)
		{
			// ステージの設定
			stage_ = std::make_unique<Stage>(*player, *boss_);
			stage_->SetEnemy(monsters_);
			stage_->Init();
			stage_->ChangeStage(Stage::NAME::MAIN_PLANET);

			// スカイドーム
			skyDome_ = std::make_unique<SkyDome>(player->GetTransform());
			skyDome_->Init();

			// カメラの設定
			SceneManager::GetInstance().GetCamera().lock()->SetFollow(&player->GetTransform());
			SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FOLLOW);
		}
		// プレイヤーを格納
		players_.push_back(std::move(player));
	}
	// ステージにプレイヤーを参照させる
	stage_->SetPlayers(players_);

	// 背景初期化
	grid_ = std::make_unique<Grid>();
	grid_->Init();

	// 空の弾の作成
	auto shot = std::make_unique<ItemShot>(0, AsoUtility::VECTOR_ZERO, AsoUtility::VECTOR_ZERO, -1);
	shot->ChangeState();
	shots_.push_back(std::move(shot));

	// ゲーム開始待機時間
	stepCountDown_ = 1.5f;

	// ダウンした回数
	downCnt_ = 0;

	// 音の設定
	soundRate_ = 0.0f;
	SoundManager::GetInstance().Play(SoundManager::SRC::BATTLE_BGM, Sound::TIMES::LOOP);
	SoundManager::GetInstance().ChengeVolume(SoundManager::SRC::BATTLE_BGM, soundRate_);

	// 計測開始
	timer_ = std::make_unique<Timer>(LIMIT_TIME);
	timer_->Start();

	// マウスポインタ非表示
	SetMouseDispFlag(false);
}

void GameScene::Update(void)
{
	// ホストならゲーム時間を進める
	auto& nIns = NetManager::GetInstance();
	if (nIns.GetMode() == NET_MODE::HOST)
	{
		// ゲーム時間進行
		SceneManager::GetInstance().ForwardGameTime();// 進めるゲーム時間(GameTotalTime　+=　デルタタイム)
	}

	// フェード更新
	fader_->Update();
	Fader::NET_STATE fState = fader_->GetState();
	switch (fState)
	{
		// フェードアウト開始
	case Fader::NET_STATE::FADE_OUT:
		if (fader_->IsEnd())
		{
			fader_->SetFade(Fader::NET_STATE::FADE_IN);
		}
		break;
		// フェードイン開始
	case Fader::NET_STATE::FADE_IN:
		if (fader_->IsEnd())
		{
			// 明転後、シーン遷移終了
			fader_->SetFade(Fader::NET_STATE::NONE);
		}
		break;
	}

	// シーン遷移が間に合ってないプレイヤーのために待機
	float limit = stepCountDown_ - SceneManager::GetInstance().GetTotalGameTime();
	if (limit > 0.0f)
	{
		DrawFormatString(DEBUG_DRAW_POS_X, DEBUG_DRAW_POS_Y, 0xffffff, L"%f", limit);
		return;
	}
	else
	{
		NetManager::GetInstance().ChangeGameState(GAME_STATE::GAME_PLAYING);
	}

#ifdef _DEBUG
	Measure::GetInstance().Start();
	Measure::GetInstance().Watch(L"01 Application START");
#endif

	// 自身のアクション記録(履歴じゃない)をリセット
	NetManager::GetInstance().ResetAction();

	// 背景関係の更新
	skyDome_->Update();
	stage_->Update();
	grid_->Update();

	// ボスと一番近いプレイヤーを保存する変数
	Transform closestTrans;
	Transform trans = Transform();

	bool isBattle = false;
	// プレイヤーの更新
	for (auto& player : players_)
	{
		player->Update();

		// BGM設定
		if (player->GetAreaId() == boss_->GetAreaId() && player->IsSelf()
			&& boss_->IsBattle())
		{
			if (soundRate_ < 1.0f)
			{
				soundRate_ += SceneManager::GetInstance().GetDeltaTime() * SOUND_RATE_SPEED;
			}
			else
			{
				soundRate_ = 1.0f;
			}
			// 音の再生
			SoundManager::GetInstance().ChengeVolume(SoundManager::SRC::BATTLE_BGM, soundRate_);
		}
		else if (player->IsSelf())
		{
			if (soundRate_ > 0.0f) {
				soundRate_ -= SceneManager::GetInstance().GetDeltaTime() * SOUND_RATE_SPEED;
			}
			else {
				soundRate_ = 0.0f;
			}
			// BGMの音量設定
			SoundManager::GetInstance().ChengeVolume(SoundManager::SRC::BATTLE_BGM, soundRate_);
		}

		// 同じエリアにいないなら無視
		if (player->GetAreaId() != boss_->GetAreaId())
		{
			continue;
		}

		isBattle = true;
		auto& pPos = player->GetTransform().pos;
		auto& ePos = boss_->GetTransform().pos;

		// プレイヤーとの衝突判定
		VECTOR diff = VSub(pPos, ePos);
		float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		// 状態別処理
		// PLAY状態でプレイヤーが近づいたらターゲットにする
		if (boss_->IsState(Boss::STATE::PLAY))
		{
			if (disPow < Boss::MOVE_RADIUS * Boss::MOVE_RADIUS)
			{
				boss_->SetFollow(&player->GetTransform());
				for (auto& mons : monsters_)
				{
					mons->SetFollow(&player->GetTransform());
				}

				trans = player->GetTransform(); // BATTLEに引き継ぐ初期ターゲット
			}
		}
		// BATTLE状態で一番近いプレイヤーをターゲットにする
		else if (boss_->IsState(Boss::STATE::BATTLE))
		{
			if (disPow < minDist_)
			{
				minDist_ = disPow;
				closestTrans = player->GetTransform();
				boss_->SetFollow(&player->GetTransform());
				for (auto& mons : monsters_)
				{
					mons->SetFollow(&player->GetTransform());
				}
			}
		}
	}
	for (auto& shot : shots_)
	{
		shot->Update();
	}

	// 同じエリアにいなくても機能
	// バトル中同じリアにいるプレイヤーが1人もいなかったら
	if (!isBattle && boss_->IsState(Boss::STATE::BATTLE))
	{
		boss_->SetBattleCancel();//バトル中止
	}
	// 体力が減ったら移動（線形補間）
	// LerpMove中は使用不可
	if ((boss_->GetLerpTime() <= 0.0f || !stage_->GetActivePlanet().lock()->CheckArea(boss_->GetTransform().pos))
		&& !boss_->IsState(Boss::STATE::LERP_MOVE))
	{
		textId_ = std::to_string(boss_->GetAreaId());
		if (boss_->GetLerpTime() <= 0.0f)
		{
			// ボスの現在エリアから次の移動場所決定のIDを決める
			switch (boss_->GetAreaId())
			{
			case 1:	// エリア1ならエリア2へ
				textId_ = AREA_FIRST;
				break;
			case 2:	// エリア2ならエリア1へ
				textId_ = AREA_SECOND;
				break;
			default:
				break;
			}
		}
		stepId_ = 0;
		boss_->StartLerp();// 移動開始
		boss_->SetLerpPos(stage_->GetActivePlanet().lock()->GetLerpPos(textId_, stepId_));
	}
	// 線形補間が終わったら次があるか調べる
	else if (!boss_->IsLerp() && boss_->IsState(Boss::STATE::LERP_MOVE))
	{
		stepId_ += 1;
		if (stage_->GetActivePlanet().lock()->CheckLerpPos(textId_, stepId_))
		{
			boss_->SetLerpPos(stage_->GetActivePlanet().lock()->GetLerpPos(textId_, stepId_));
		}
		else
		{
			stepId_ = 0;
			boss_->SetBattleCancel();// バトル中止
		}
	}

	// ボスの更新
	boss_->Update();
	for (auto& mons : monsters_)
	{
		mons->Update();
	}

	// 通信を送る
	NetManager::GetInstance().Send(NET_DATA_TYPE::ACTION_HIS_ALL);

	// 衝突判定
	Collision();

	// ゲームの勝敗判定
	GameManager::GAME_RESULT result = GameManager::GAME_RESULT::NONE;

	if (GameManager::GetInstance().IsClear()) {
		// タイマーが動いてたら止める
		if (timer_->IsRunning())timer_->Reset();
		// クリア時間の更新
		GameManager::GetInstance().UpdateClearTime(SceneManager::GetInstance().GetDeltaTime());

		if (GameManager::GetInstance().GetClearTime() <= 0.0f)
		{
			// ゲームの勝敗判定//シーン遷移
			GameManager::GAME_RESULT result = GameManager::GAME_RESULT::GAME_CLEAR;
			GameManager::GetInstance().SetGameResult(result);
		}
	}

	// タイムアップ判定
	if (timer_->IsTimeUp() && !GameManager::GetInstance().IsClear())
	{
		// ゲームの勝敗判定
		result = GameManager::GAME_RESULT::TIME_OVER;
		GameManager::GetInstance().SetGameResult(result);
		SceneManager::GetInstance().CaptureMainScreen();
	}
	// ボス撃破判定
	if (GameManager::GetInstance().GetGameResult() != GameManager::GAME_RESULT::NONE)
	{
		// ゲームオーバーシーンへ遷移
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RSULT);
	}
}

void GameScene::Draw(void)
{
	// 背景、ステージの描画
	skyDome_->Draw();
	stage_->Draw();

#ifdef _DEBUG
	grid_->Draw();
#endif

	for (auto& shot : shots_)
	{
		shot->Draw();
	}

	auto& nIns = NetManager::GetInstance();

	// ボスの描画
	boss_->Draw();
	for (auto& mons : monsters_)
	{
		mons->Draw();
	}

	// プレイヤー情報取得用
	int playerNo = 0;

	// プレイヤーの描画
	for (auto& player : players_)
	{
		player->Draw();
		player->DrawUI(playerNo);
		playerNo++;
	}

	// タイマーの描画
	timer_->DrawTimer(TIME_POS_X, TIME_POS_Y); // 中心位置 (x, y)

	GameManager::GetInstance().DrawClear();

#ifdef _DEBUG

	float remaining = timer_->GetRemainingTime();
	wchar_t  buf[64];
	swprintf_s(buf, L"残り時間: %.2f秒", remaining);

	if (timer_->IsTimeUp())
	{
		DrawString(DEBUG_DRAW_POS_X, DEBUG_DRAW_POS_Y, L"タイムアップ！", RED_COLOR);
	}
	else
	{
		DrawString(DEBUG_DRAW_POS_X, DEBUG_DRAW_POS_Y, buf, WHITE_COLOR);
	}

	float limit = stepCountDown_ - SceneManager::GetInstance().GetTotalGameTime();
	if (limit > 0.0f)
	{
		DrawFormatString(DEBUG_DRAW_POS_X, DEBUG_DRAW_POS_Y, WHITE_COLOR, L"%f", limit);
	}

#endif // DEBUG

	// フェードの描画
	fader_->Draw();
}

void GameScene::Release(void)
{

}

// 弾の生成
void GameScene::CreateShot(ShotBase::TYPE type, int damage, const VECTOR& birthPos, const VECTOR& dir, int key)
{
	bool isEnd = false;

	// 既存の弾を再利用
	for (auto& shot : shots_)
	{
		if (shot->IsEnd() && shot->GetType() == type)
		{
			shot->Create(damage, birthPos, dir, key);
			isEnd = true;
			break;
		}
	}
	if (!isEnd)
	{
		std::unique_ptr<ShotBase> shot = nullptr;
		switch (type)
		{
			case ShotBase::TYPE::ARROW:
				shot = std::make_unique<ArrowShot>(damage, birthPos, dir, key);
				break;
			case ShotBase::TYPE::ITEM:
				shot = std::make_unique<ItemShot>(damage, birthPos, dir, key);
				break;
			case ShotBase::TYPE::BOM:
				shot = std::make_unique<BomObject>(damage, birthPos, dir, key);
				break;
			default:
				break;
		}
		shots_.push_back(std::move(shot));
	}
}

// ステージオブジェクトの生成
void GameScene::CreateObject(const Transform& _trans)
{
	stage_->AddBom(_trans);
}

void GameScene::Collision(void)
{
	auto& nIns = NetManager::GetInstance();

	// プレイヤーの描画
	for (auto& player : players_)
	{
		// 参照型要改善
		int model = player->GetTransWeapon().modelId;

		// 最終的にボスの攻撃判定 // 自分だけ
		if (boss_->CollisionCapsule(player->GetCapsule())
			&& player->IsAttack() && player->IsHit() && player->IsSelf())
		{
			player->SetHit(true);
			SceneManager::GetInstance().GetCamera().lock()->StartShake(CAMERA_SHAKE_TIME, CAMERA_SHAKE_POWER);

			boss_->Damage(player->GetAttrckPow() * player->GetAttrckRate());
			// 音の再生
			SoundManager::GetInstance().Play(SoundManager::SRC::SLASH_DAMAGE, Sound::TIMES::ONCE, true);
		}
		// 小型の処理
		for (auto& mons : monsters_)
		{
			std::weak_ptr<SmallMonster> enemy = std::dynamic_pointer_cast<SmallMonster>(mons);
			if (enemy.lock() == nullptr)continue;

			if (enemy.lock()->IsState(SmallMonster::STATE::END)
				|| enemy.lock()->IsState(SmallMonster::STATE::NONE))
			{
				continue;
			}
			if (enemy.lock()->CollisionCapsule(player->GetCapsule())
				&& player->IsAttack() && player->IsHit() && player->IsSelf())
			{
				player->SetHit(true);

				SceneManager::GetInstance().GetCamera().lock()->StartShake(CAMERA_SHAKE_TIME, CAMERA_SHAKE_POWER);

				enemy.lock()->Damage(player->GetAttrckPow() * player->GetAttrckRate());
				// 音の再生
				SoundManager::GetInstance().Play(SoundManager::SRC::SLASH_DAMAGE, Sound::TIMES::ONCE, true);
			}
		}
		auto& users = NetManager::GetInstance().GetNetUsers();

		// プレイヤーの当たり判定（ボス攻撃）//自分だけ
		if (player->IsSelf())
		{
			// ボスに注目
			if (player->IsAimSet() && player->GetAreaId() == boss_->GetAreaId())
			{
				if (player->IsTrgAimSet())
				{
					SoundManager::GetInstance().Play(SoundManager::SRC::LOCKON, Sound::TIMES::ONCE);
				}
				SceneManager::GetInstance().GetCamera().lock()->LookAtSmoothly(boss_->GetTransform().pos, 0.5f);
			}

			auto& eTrans = boss_->GetTransform();
			// ボス攻撃の処理
			if (boss_->CollisionAttrck(player->GetTransform().modelId))
			{
				VECTOR mixDir = AsoUtility::VECTOR_ZERO;
				if (boss_->GetAnim() == (int)Boss::ANIM_TYPE::ATTRCK_DASH)
				{
					mixDir = VScale(boss_->GetTransform().GetForward(), DAMAGE_VEC_RATE);
				}
				else if (boss_->GetAnim() == (int)Boss::ANIM_TYPE::ATTRCK_L_CLOW)
				{
					mixDir = VScale(boss_->GetTransform().GetRight(), DAMAGE_VEC_RATE);
				}
				else if (boss_->GetAnim() == (int)Boss::ANIM_TYPE::ATTRCK_R_CLOW)
				{
					mixDir = VScale(boss_->GetTransform().GetLeft(), DAMAGE_VEC_RATE);
				}
				player->Damage(boss_->GetAttrckPow() * boss_->GetAttrckRate(), boss_->GetAttrckPos(), mixDir);
			}
			// 小型の処理(攻撃)
			for (auto& mons : monsters_)
			{
				std::weak_ptr<SmallMonster> enemy = std::dynamic_pointer_cast<SmallMonster>(mons);
				if (enemy.lock() == nullptr)continue;

				if (enemy.lock()->CollisionAttrck(player->GetTransform().modelId))
				{
					VECTOR mixDir = AsoUtility::VECTOR_ZERO;
					player->Damage(mons->GetAttrckPow() * mons->GetAttrckRate(), enemy.lock()->GetAttrckPos(), mixDir);
				}
			}
		}
	}

	// 弾の当たり判定
	for (auto& shot : shots_)
	{
		// 矢の処理
		if (shot->GetType() == ShotBase::TYPE::ARROW)
		{
			if (shot->IsShot())
			{
				// --- ボスとの判定 ---
				if (boss_->CollisionCapsule(shot->GetCapsule()))
				{
					if (boss_->IsState(Boss::STATE::PLAY))
					{
						boss_->SetFollow(&players_.front()->GetTransform());
					}
					ShotHitEnemy(*shot, *boss_);
				}

				// --- 小型モンスターとの判定 ---
				for (auto& mons : monsters_)
				{
					std::weak_ptr<SmallMonster> enemy = std::dynamic_pointer_cast<SmallMonster>(mons);
					if (enemy.lock() == nullptr)continue;

					if (enemy.lock()->IsState(SmallMonster::STATE::END)
						|| enemy.lock()->IsState(SmallMonster::STATE::NONE))
					{
						continue;
					}
					if (enemy.lock()->CollisionCapsule(shot->GetCapsule())
						&& !enemy.lock()->IsState(SmallMonster::STATE::END)
						&& !enemy.lock()->IsState(SmallMonster::STATE::NONE))
					{
						ShotHitEnemy(*shot, *enemy.lock());
					}
				}
			}
		}
		else if (shot->GetType() == ShotBase::TYPE::ITEM
			&& shot->IsBlast())
		{
			// 敵ならスタン
			// プレイヤーとの衝突判定
			float disPow = AsoUtility::GetDisPow(boss_->GetTransform().pos, shot->GetTransform().pos);

			if (boss_->IsTargetInFOV(shot->GetTransform().pos, Boss::FOV_RADIUS_FLASH)
				&& disPow < Boss::MOVE_RADIUS * Boss::MOVE_RADIUS)
			{
				boss_->StartStunned();
			}

			// プレイヤーが半径内に入っているなら画面を明転
			// フェードアウト(暗転)を開始する
			for (auto& player : players_)
			{
				if (nIns.GetSelf().key == player->GetKey())
				{
					float disPow = AsoUtility::GetDisPow(player->GetTransform().pos, shot->GetTransform().pos);

					if (disPow < shot->GetRadius() * shot->GetRadius())
					{
						// 画面を暗転
						fader_->SetFade(Fader::NET_STATE::FADE_OUT);
						// 他プレイヤーとは別なので消す
						shot->ChangeState();
					}
				}
			}
		}
		// 爆弾の処理
		else if (shot->GetType() == ShotBase::TYPE::BOM)// ここでほしいのは全てのプレイヤーの攻撃判定と自分の管理しているクラスの判定
		{
			// 通常状態(設置状態)
			if (shot->IsShot())
			{
				for (auto& player : players_)
				{
					// プレイヤーとの衝突判定	
					if (shot->CollisionCapsule(player->GetCapsule()) && player->IsSyncAttack())
					{
						// 爆発開始
						shot->ChangeState(ShotBase::STATE::BLAST);
						// 音・カメラ
						//SoundManager::GetInstance().Play(SoundManager::SRC::BOM_BLAST, Sound::TIMES::ONCE, true);
						break;
					}
					for (auto& arrowShot : shots_)
					{
						if (arrowShot->GetType() == ShotBase::TYPE::ARROW)
						{
							if (shot->CollisionCapsule(arrowShot->GetCapsule()) && arrowShot->IsShot())
							{
								// 爆発開始
								shot->ChangeState(ShotBase::STATE::BLAST);
								// 音・カメラ
								//SoundManager::GetInstance().Play(SoundManager::SRC::BOM_BLAST, Sound::TIMES::ONCE, true);
								break;
							}
						}
					}
				}
			}
			// 爆発してるとき
			else if (shot->IsBlast())
			{
				const VECTOR& ShotPos = shot->GetTransform().pos;

				// プレイヤーが半径内に入っているなら画面を明転
				// フェードアウト(暗転)を開始する
				for (auto& player : players_)
				{
					if (nIns.GetSelf().key == player->GetKey())
					{
						float disPow = AsoUtility::GetDisPow(player->GetTransform().pos, ShotPos);

						if (disPow < shot->GetRadius() * shot->GetRadius())
						{
							// 画面を暗転
							shot->ChangeState();
							// 吹き飛び方向
							VECTOR mixDir = VScale(VNorm(VSub(player->GetTransform().pos, ShotPos)), 0.001f);
							// ダメージ
							player->Damage(shot->GetDamage() * BLAST_DAMAGE_RATE, ShotPos, mixDir);
							// 音・カメラ
							SceneManager::GetInstance().GetCamera().lock()->StartShake(1.0f, 2.0f);
						}
					}
				}
				// 敵ならダメージ
				float disPow = AsoUtility::GetDisPow(boss_->GetTransform().pos, ShotPos);

				if (disPow < shot->GetRadius() * shot->GetRadius()
					&& shot->GetKey() == nIns.GetSelf().key)
				{
					boss_->Damage(shot->GetDamage(), true);
					shot->ChangeState();
				}
			}
		}
	}
}

// 弾が敵に当たったときの処理
void GameScene::ShotHitEnemy(ShotBase& shot, EnemyBase& enemy)
{
	auto& nIns = NetManager::GetInstance();

	for (auto& player : players_)
	{
		// 弾の所有者と自分と同じならダメージ処理
		if (shot.GetKey() == nIns.GetSelf().key && shot.GetKey() == player->GetKey())
		{
			// 音・カメラ・ダメージ
			SoundManager::GetInstance().Play(SoundManager::SRC::SHOT_DAMAGE, Sound::TIMES::ONCE, true);
			SceneManager::GetInstance().GetCamera().lock()->StartShake(CAMERA_SHAKE_TIME, CAMERA_SHAKE_POWER);
			// 敵にダメージ
			enemy.Damage(static_cast<int>(static_cast<float>(shot.GetDamage()) * player->GetAttrckRate()));
		}
		// 戦闘状態へ（Follow設定など）
		enemy.SetFollow(&player->GetTransform());
	}
	// 弾の状態を変更
	shot.ChangeState();
}

