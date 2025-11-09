#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Utility/Measure.h"

#include "../Manager/SoundManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"

#include "../Common/Fader.h"

#include "../Net/NetStructures.h"
#include "../Net/NetManager.h"

#include "../Object/Common/Timer.h"

#include "../Object/Player/Player.h"
#include "../Object/Player/GreatSword.h"
#include "../Object/Player/Arrow.h"

#include "../Object/Shot/ItemShot.h"
#include "../Object/Shot/ArrowShot.h"
#include "../Object/Shot/ShotBase.h"

#include "../Object/Enemy/Boss.h"
#include "../Object/Enemy/SmallMonster.h"
#include "../Object/Stage/Stage.h"
#include "../Object/Stage/Planet.h"
#include "../Object/Stage/SkyDome.h"
#include "../Object/Stage/Grid.h"

#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "GameScene.h"

namespace
{
	//タイマーの描画位置
	constexpr int TIME_POS_X = Timer::SIZE_X / 2 + 10;
	constexpr int TIME_POS_Y = Timer::SIZE_Y / 2 + 10;
	constexpr float LIMIT_TIME = 60.0f * 5.0f;


	constexpr int MAP_SIZE = 200;
	constexpr int MAP_POS_X = MAP_SIZE / 2 + 10;
	constexpr int MAP_POS_Y = Application::SCREEN_SIZE_Y - MAP_SIZE / 2 - 10;
}


GameScene::GameScene(void)
{
	stepCountDown_ = 0;
	players_.clear();

	skyDome_ = nullptr;
	grid_ = nullptr;
}

GameScene::~GameScene(void)
{
	shots_.clear();

	SceneManager::GetInstance().SetTotalGameTime(0.0f);

	SoundManager::GetInstance().AllStop();

	if (playerHandle_ != -1) {
		DeleteGraph(playerHandle_);
	}
	if (bossHandle_ != -1) {
		DeleteGraph(bossHandle_);
	}
	if (mapHandle_ != -1) {
		DeleteGraph(mapHandle_);
	}
}

void GameScene::Init(void)
{
	//初期化
	SceneManager::GetInstance().SetGameResult(SceneManager::GAME_RESULT::NONE);

	auto& users = NetManager::GetInstance().GetNetUsers();
	auto& nIns = NetManager::GetInstance();

	fader_ = std::make_unique<Fader>(0xffffff);
	fader_->Init(15.0f);

	boss_ = std::make_unique<Boss>(NetManager::GetInstance().GetHost().key);
	boss_->Init();
	
	Monsters_[0] = std::make_unique<SmallMonster>(NetManager::GetInstance().GetHost().key,0);
	Monsters_[0]->Init();
	Monsters_[1] = std::make_unique<SmallMonster>(NetManager::GetInstance().GetHost().key,1);
	Monsters_[1]->Init();
	Monsters_[2] = std::make_unique<SmallMonster>(NetManager::GetInstance().GetHost().key,2);
	Monsters_[2]->Init();

	textId_ = "";
	stepId_ = 0;

	for (auto& user : users)
	{
		auto  player = std::make_shared<Player>(user.first);

		// モデルの基本設定
		switch (nIns.GetWeapon(user.first))
		{
		case 0:
			player = std::make_shared<GreatSword>(user.first);
			break;
		case 1:
			player = std::make_shared<Player>(user.first);
			break;
		case 2:
			player = std::make_shared<Arrow>(user.first);
			break;
		default:
			break;
		}
		
		Player::KEY_CONFIG keyP1 = {
			KEY_INPUT_UP, KEY_INPUT_DOWN, KEY_INPUT_LEFT, KEY_INPUT_RIGHT, KEY_INPUT_SPACE, KEY_INPUT_M
		};
		player->Init(this, user.second.playerType);

		//自分用のクラス	
		if (user.first== NetManager::GetInstance().GetSelf().key)
		{

			// ステージ
			stage_ = std::make_unique<Stage>(*player, *boss_);
			stage_->Init();
			// ステージの初期設定
			//stage_->ChangeStage(Stage::NAME::SPECIAL_STAGE);
			stage_->ChangeStage(Stage::NAME::MAIN_PLANET);

			// スカイドーム
			skyDome_ = std::make_unique<SkyDome>(player->GetTransform());
			skyDome_->Init();


			SceneManager::GetInstance().GetCamera().lock()->SetFollow(&player->GetTransform());
			SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FOLLOW);
		}

		players_.push_back(std::move(player));
	}
	stage_->SetPlayers(players_);


	//背景初期化
	grid_ = std::make_unique<Grid>(); 
	grid_->Init();

	//空の弾の作成
	auto  shot = std::make_unique<ItemShot>(0, AsoUtility::VECTOR_ZERO, AsoUtility::VECTOR_ZERO, -1);
	shot->Destroy();
	shots_.push_back(std::move(shot));

	//ゲーム開始待機時間
	stepCountDown_ = 1.5f;

	//ダウンした回数
	downCnt_ = 0;

	//音の設定
	soundRate_ = 0.0f;
	SoundManager::GetInstance().Play(SoundManager::SRC::BATTLE_BGM, Sound::TIMES::LOOP);
	SoundManager::GetInstance().ChengeVolume(SoundManager::SRC::BATTLE_BGM, soundRate_);
	//SoundManager::GetInstance().Pause(SoundManager::SRC::BATTLE_BGM, Sound::TIMES::LOOP);

	//計測開始
	timer_ = std::make_unique<Timer>(LIMIT_TIME);
	timer_->Start();

	//マップ
	mapHandle_ = LoadGraph((Application::PATH_IMAGE + L"UI/Map.png").c_str());
	bossHandle_ = LoadGraph((Application::PATH_IMAGE + L"UI/Boss.png").c_str());
	playerHandle_ = LoadGraph((Application::PATH_IMAGE + L"UI/Player.png").c_str());

	//ミニマップシェーダ
	Material_ = std::make_unique<PixelMaterial>(L"Map.cso", 3);
	Material_->AddConstBuf({ 0.5f, 0.5f, 0.2f, 0.2f });//ボス
	Material_->AddConstBuf({ 0.5f, 0.5f, 0.2f, 0.2f });//プレイヤー
	Material_->AddConstBuf({ 0.0f, 0.0f, 0.0f, 0.0f });//プレイヤー
	Material_->AddTextureBuf(mapHandle_);
	Material_->AddTextureBuf(bossHandle_);
	Material_->AddTextureBuf(playerHandle_);
	Renderer_ = std::make_unique<PixelRenderer>(*Material_);
	Renderer_->SetSize(Vector2(MAP_SIZE, MAP_SIZE));
}

void GameScene::Update(void)
{

	auto& nIns = NetManager::GetInstance();
	if (nIns.GetMode() == NET_MODE::HOST)
	{
		// ゲーム時間進行
		SceneManager::GetInstance().ForwardGameTime();//進めるゲーム時間(GameTotalTime　+=　デルタタイム)
	}

	// フェード更新
	fader_->Update();
	Fader::NET_STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::NET_STATE::FADE_OUT:
		if (fader_->IsEnd())
		{
			fader_->SetFade(Fader::NET_STATE::FADE_IN);
		}
		break;
	case Fader::NET_STATE::FADE_IN:
		if (fader_->IsEnd())
		{
			// 明転後、シーン遷移終了
			fader_->SetFade(Fader::NET_STATE::NONE);
		}
		break;
	}

	//シーン遷移が間に合ってないプレイヤーのために待機
	float limit = stepCountDown_ - SceneManager::GetInstance().GetTotalGameTime();
	if (limit > 0.0f)
	{
		DrawFormatString(100, 100, 0xffffff, L"%f", limit);
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


	//自身のアクション記録(履歴じゃない)をリセット
	NetManager::GetInstance().ResetAction();

	//背景関係の更新
	skyDome_->Update();
	stage_->Update();
	grid_->Update();


	Transform closestTrans;
	Transform trans = Transform();


	bool isBattle = false;
	// プレイヤーの更新
	for (auto& player : players_)
	{
		player->Update();

		//BGM設定
		if (player->GetAreaId() == boss_->GetAreaId() && player->IsSelf()
			&& boss_->IsBattle())
		{
			if (soundRate_ < 1.0f)
			{
				soundRate_ += SceneManager::GetInstance().GetDeltaTime() * 0.3f;
			}
			else
			{
				soundRate_ = 1.0f;
			}
			//音の再生
			SoundManager::GetInstance().ChengeVolume(SoundManager::SRC::BATTLE_BGM, soundRate_);
			//SoundManager::GetInstance().Resume(SoundManager::SRC::BATTLE_BGM, Sound::TIMES::LOOP);
		}
		else if(player->IsSelf())
		{
			if (soundRate_ > 0.0f)
			{
				soundRate_ -= SceneManager::GetInstance().GetDeltaTime() * 0.3f;
			}
			else {
				soundRate_ = 0.0f;
				//SoundManager::GetInstance().Pause(SoundManager::SRC::BATTLE_BGM, Sound::TIMES::LOOP);
			}
			SoundManager::GetInstance().ChengeVolume(SoundManager::SRC::BATTLE_BGM, soundRate_);
		}

		//同じエリアにいないなら無視
		if (player->GetAreaId() != boss_->GetAreaId())
		{
			continue;
		}

		isBattle = true;

		auto& pPos = player->GetTransform().pos;
		auto& ePos = boss_->GetTransform().pos;

		// playerとの衝突判定
		VECTOR diff = VSub(pPos, ePos);
		float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		// 状態別処理
		// PLAY状態でプレイヤーが近づいたらターゲットにする
		if (boss_->IsState(Boss::STATE::PLAY))
		{
			if (disPow < Boss::MOVE_RADIUS * Boss::MOVE_RADIUS)
			{
				boss_->SetFollow(&player->GetTransform());
				for (auto& mons : Monsters_)
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
				for (auto& mons : Monsters_)
				{
					mons->SetFollow(&player->GetTransform());
				}
			}
		}
		else if (boss_->IsState(Boss::STATE::FOLLOW))
		{
			//if (disPow < 100.0f * 100.0f)//ダメージ半径×攻撃半径
			//{
			//	boss_->SetFollow(&player->GetTransform());
			//}
		}
	}
	for (auto& shot : shots_)
	{
		shot->Update();
	}

	//同じエリアにいなくても機能
	//バトル中同じリアにいるプレイヤーが1人もいなかったら
	if (!isBattle && boss_->IsState(Boss::STATE::BATTLE))
	{
		boss_->SetBattleCancel();//バトル中止
	}
	//体力が減ったら移動（線形補間）
	//LerpMove中は使用不可
	//if (boss_->GetHp() <= Boss::MAX_HP && !boss_->IsState(Boss::STATE::LERP_MOVE))
	if ((boss_->GetLerpTime() <= 0.0f|| !stage_->GetActivePlanet().lock()->CheckArea(boss_->GetTransform().pos))
		&& !boss_->IsState(Boss::STATE::LERP_MOVE))
	{
		textId_ = std::to_string(boss_->GetAreaId());
		if (boss_->GetLerpTime() <= 0.0f)
		{
			switch (boss_->GetAreaId())
			{
			case 1:
				textId_ = "1to2";
				break;
			case 2:
				textId_ = "2to1";
				break;
			default:
				break;
			}
		}
		stepId_ = 0;
		boss_->StartLerp();//移動開始
		boss_->SetLerpPos(stage_->GetActivePlanet().lock()->GetLerpPos(textId_, stepId_));
	}
	//線形補間が終わったら次があるか調べる
	else if ( !boss_->IsLerp() && boss_->IsState(Boss::STATE::LERP_MOVE))
	{
		stepId_ += 1;
		if (stage_->GetActivePlanet().lock()->CheckLerpPos(textId_, stepId_))
		{
			boss_->SetLerpPos(stage_->GetActivePlanet().lock()->GetLerpPos(textId_, stepId_));
		}
		else
		{
			stepId_ = 0;
			boss_->SetBattleCancel();//バトル中止
		}
	}

	//ボスの更新
	boss_->Update();
	for (auto& mons : Monsters_)
	{
		mons->Update();
	}


	// 通信を送る
	NetManager::GetInstance().Send(NET_DATA_TYPE::ACTION_HIS_ALL);

	// 衝突判定
	Collision();




	// ゲームの勝敗判定
	SceneManager::GAME_RESULT result = SceneManager::GAME_RESULT::NONE;

	if (timer_->IsTimeUp())
	{
		// ゲームの勝敗判定
		SceneManager::GAME_RESULT result = SceneManager::GAME_RESULT::GAME_OVER;
		SceneManager::GetInstance().SetGameResult(result);
		SceneManager::GetInstance().CaptureMainScreen();
	}
	if (SceneManager::GetInstance().GetGameResult() != SceneManager::GAME_RESULT::NONE)
	{

		// ゲームオーバーシーンへ遷移
		//SceneManager::GetInstance().SetGameResult(result);
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RSULT);
	}
	//if (!nIns.IsSameGameState(GAME_STATE::GAME_PLAYING))//最初の一回目がダメ
	//{
	//	SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);//ゲームスタート！
	//}
}

void GameScene::Draw(void)
{
	//背景、ステージの描画
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

	// UIの描画
	//DrawUI();

	int x = 0;
	int y = 0;

	//ボスの描画]
	boss_->Draw();
	for (auto& mons : Monsters_)
	{
		mons->Draw();
	}

	VECTOR playerPos{};
	VECTOR playerDirF{};
	// プレイヤーの描画
	for (auto& player : players_)
	{

		player->Draw();
		player->DrawUI(x);
		x++;
		if (player->IsSelf())
		{
			playerPos = player->GetTransform().pos;
			playerDirF = player->GetTransform().GetForward();
		}
	}

	// Draw内
	timer_->DrawNeedle(TIME_POS_X, TIME_POS_Y); // 中心位置 (x, y)

	//マップ
	const VECTOR bossPos = boss_->GetTransform().pos;
	const VECTOR cameraF = SceneManager::GetInstance().GetCamera().lock()->GetForward();
	// カメラの右ベクトル
	VECTOR camRight = VCross(VGet(0, -1, 0), cameraF);
	camRight = VNorm(camRight);

	// 相対位置（プレイヤーを原点としたときのボスの位置）
	float relX = bossPos.x - playerPos.x;
	float relZ = bossPos.z - playerPos.z;
	// 例: ミニマップや2D表示で使うために、相対位置をスケーリング
	float scale = 0.0001f; // ミニマップの縮小率など
	float screenX = relX * scale + 0.5f;
	float screenY = -relZ * scale + 0.5f; // ZはY軸にマッピング

	Material_->SetConstBuf(0,{ screenX, screenY, 0.05f, 0.05f });//ボス
	Material_->SetConstBuf(1,{ -playerDirF.x, playerDirF.z, 0.05f, 0.05f });//
	Material_->SetConstBuf(2,{ -camRight.x, camRight.z, 0.0f, 0.0f });//

	//ミニマップの描画
	/*SetTextureAddressModeUV(DX_TEXADDRESS_CLAMP, DX_TEXADDRESS_CLAMP);
	Renderer_->Draw(MAP_POS_X, MAP_POS_Y);
	SetTextureAddressModeUV(DX_TEXADDRESS_CLAMP, DX_TEXADDRESS_CLAMP);*/

#ifdef _DEBUG

	float remaining = timer_->GetRemainingTime();
	wchar_t  buf[64];
	swprintf_s(buf, L"残り時間: %.2f秒", remaining);
	DrawString(100, 100, buf, GetColor(255, 255, 255));

	if (timer_->IsTimeUp())
	{
		DrawString(100, 140, L"タイムアップ！", GetColor(255, 0, 0));
	}

	float limit = stepCountDown_ - SceneManager::GetInstance().GetTotalGameTime();
	if (limit > 0.0f)
	{
		//DrawFormatString(100, 100, 0xffffff, L"%f", limit);
	}

#endif // DEBUG

	// 最後
	fader_->Draw();

}

void GameScene::Release(void)
{

}




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
				shot = std::make_unique<ItemShot>(damage, birthPos, dir, key);
				break;
			default:
				break;
		}
		//auto shot = std::make_unique<ShotBase>(damage, birthPos, dir, key);
		shots_.push_back(std::move(shot));
	}
}
//ステージオブジェクトの生成
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
		//参照型要改善
		int model = player->GetTransWepon().modelId;

		//味方への攻撃判定（未実装）
		for (auto& playerVS : players_)
		{
			if (player == playerVS)
			{
				continue;
			}

			if (playerVS->CollisionCapsule(model) && player->IsAttrck())
			{
				//playerVS->Damage(3);
			}
		}


		//最終的にボスの攻撃判定//自分だけ
		if (boss_->CollisionCapsule(player->GetCapsule())
			&& player->IsAttrck() && player->IsHit() && player->IsSelf())
		{
			player->SetHit(true);
			SceneManager::GetInstance().GetCamera().lock()->StartShake();

			boss_->Damage(player->GetAttrckPow() * player->GetAttrckRate());
			//音の再生
			SoundManager::GetInstance().Play(SoundManager::SRC::SLASH_DAMAGE, Sound::TIMES::ONCE, true);

		}
		//小型の処理
		for (auto& mons : Monsters_)
		{
			if (mons->CollisionCapsule(player->GetCapsule())
				&& player->IsAttrck() && player->IsHit() && player->IsSelf())
			{
				player->SetHit(true);
				
				SceneManager::GetInstance().GetCamera().lock()->StartShake();

				mons->Damage(player->GetAttrckPow() * player->GetAttrckRate());
				//音の再生
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

			VECTOR attrckPos = VAdd(eTrans.pos, VScale(eTrans.GetForward(), 410));
			attrckPos = VAdd(attrckPos, VScale(eTrans.GetUp(), 170));

			// ボス攻撃の処理
			if (boss_->CollisionAttrck(player->GetTransform().modelId))
			{
				VECTOR mixDir = AsoUtility::VECTOR_ZERO;
				if (boss_->GetAnim() == (int)Boss::ANIM_TYPE::ATTRCK_DASH)
				{
					mixDir = VScale(boss_->GetTransform().GetForward(), 0.3f);
				}
				else if (boss_->GetAnim() == (int)Boss::ANIM_TYPE::ATTRCK_L_CLOW)
				{
					mixDir = VScale(boss_->GetTransform().GetRight(), 0.3f);
				}
				else if (boss_->GetAnim() == (int)Boss::ANIM_TYPE::ATTRCK_R_CLOW)
				{
					mixDir = VScale(boss_->GetTransform().GetLeft(), 0.3f);
				}
				player->Damage(boss_->GetAttrckPow() * boss_->GetAttrckRate(), boss_->GetAttrckPos(), mixDir);
				// playerが無敵か判定した後無敵じゃないならエフェクト

			}
			// 小型の処理(攻撃)
			for (auto& mons : Monsters_)
			{
				if (mons->CollisionAttrck(player->GetTransform().modelId)
					/*&& boss_->IsHit()*/)
				{
					VECTOR mixDir = AsoUtility::VECTOR_ZERO;

					player->Damage(mons->GetAttrckPow() * mons->GetAttrckRate(), mons->GetAttrckPos(), mixDir);
					// playerが無敵か判定した後無敵じゃないならエフェクト

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
			// --- ボスとの判定 ---
			if (boss_->CollisionCapsule(shot->GetCapsule())
				&& shot->IsShot())
			{
				if (boss_->IsState(Boss::STATE::PLAY))
				{
					boss_->SetFollow(&players_.front()->GetTransform());
				}
				ShotHitEnemy(*shot, *boss_);
			}

			// --- 小型モンスターとの判定 ---
			for (auto& mons : Monsters_)
			{
				if (mons->CollisionCapsule(shot->GetCapsule()))
				{
					ShotHitEnemy(*shot, *mons);
				}
			}
		}
		else if (shot->GetType() == ShotBase::TYPE::ITEM
			&& shot->IsBlast())
		{
			// 敵ならスタン
			// playerとの衝突判定
			float disPow = AsoUtility::GetDisPow(boss_->GetTransform().pos, shot->GetTransform().pos);

			if(boss_->IsTargetInFOV(shot->GetTransform().pos, Boss::FOV_RADIUS_FLASH)
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
						shot->Destroy();
					}
				}
			}
		}
		// 爆弾の処理
		else if (shot->GetType() == ShotBase::TYPE::BOM)
		{
			//　通常状態(設置状態)
			if (shot->IsShot())
			{

			}
			// 爆発してるとき
			else if(shot->IsBlast())
			{
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
							shot->Destroy();
						}
					}
				}
			}
			// 敵ならスタン
			// playerとの衝突判定
			float disPow = AsoUtility::GetDisPow(boss_->GetTransform().pos, shot->GetTransform().pos);

			if(boss_->IsTargetInFOV(shot->GetTransform().pos, Boss::FOV_RADIUS_FLASH)
				&& disPow < Boss::MOVE_RADIUS * Boss::MOVE_RADIUS)
			{
				boss_->StartStunned();
			}

			
		}
	}
}

void GameScene::ShotHitEnemy(ShotBase& shot, EnemyBase& enemy)
{
	auto& nIns = NetManager::GetInstance();

	for (auto& player : players_)
	{
		if (shot.GetKey() == nIns.GetSelf().key && shot.GetKey() == player->GetKey())
		{
			// 音・カメラ・ダメージ
			SoundManager::GetInstance().Play(SoundManager::SRC::SHOT_DAMAGE, Sound::TIMES::ONCE, true);
			SceneManager::GetInstance().GetCamera().lock()->StartShake();

			enemy.Damage(static_cast<int>(static_cast<float>(shot.GetDamage()) * player->GetAttrckRate()));
		}

		// 戦闘状態へ（Follow設定など）
		enemy.SetFollow(&player->GetTransform());
	}

	shot.Destroy();
}

