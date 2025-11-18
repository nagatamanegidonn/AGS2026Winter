#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/SoundManager.h"

#include "../Net/NetManager.h"
#include "../Utility/AsoUtility.h"

#include "../Object/Player/ViewPlayer.h"

#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "ConnectScene.h"

ConnectScene::ConnectScene(void)
{
	imgStart_ = -1;
	imgTitle_ = -1;
	DeleteGraph(backImg_);
}

ConnectScene::~ConnectScene(void)
{
}

void ConnectScene::Init(void)
{
	backImg_ = LoadGraph((Application::PATH_IMAGE + L"img.png").c_str());
	// ポストエフェクト用(モノトーン)
	Material_ = std::make_unique<PixelMaterial>(L"Texture.cso", 1);
	Material_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	Material_->AddTextureBuf(backImg_);
	Renderer_ = std::make_unique<PixelRenderer>(*Material_);
	Renderer_->MakeSquereVertex(
		Vector2(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

#pragma region プレイヤーの作成

	playerNum_ = 1;

	AddPlayer({ -150.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(-20.0f),  AsoUtility::Deg2RadF(0.0f) });
	AddPlayer({ -50.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(0.0f),  AsoUtility::Deg2RadF(0.0f) });
	AddPlayer({ 50.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(0.0f),  AsoUtility::Deg2RadF(0.0f) });
	AddPlayer({ 150.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(20.0f),  AsoUtility::Deg2RadF(0.0f) });

#pragma endregion

	//ネットマネージャの状態変更
	NetManager::GetInstance().ChangeGameState(GAME_STATE::CONNECTING);//ネット通信受け付ける状態に変更

	auto& nIns = NetManager::GetInstance();
	if (nIns.GetMode() == NET_MODE::HOST)//ModeがHost(リーダー)なら
	{
		//音の再生
		SoundManager::GetInstance().Play(SoundManager::SRC::CONECT_START, Sound::TIMES::ONCE, true);
	}
}

void ConnectScene::Update(void)
{
	//インスタンスクラスの取得
	auto& nIns = NetManager::GetInstance();
	const auto& ins = InputManager::GetInstance();
	const auto& users = NetManager::GetInstance().GetNetUsers();

	//武器情報の送信
	nIns.SetWeapon(NetManager::GetInstance().GetSelf().key, GameManager::GetInstance().GetWeponId());

	const InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(GameManager::GetInstance().GetControllId());

	auto& players = NetManager::GetInstance().GetNetUsers();
	if (playerNum_ < players.size())
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::ADD, Sound::TIMES::ONCE, true);
		playerNum_ = players.size();
		
	}
	int i = 0;
	for (const auto& users : players)
	{
		players_.at(i)->SetWepon(nIns.GetWeapon(users.second.key));
		i++;
	}

	for (const auto& player : players_)
	{
		player->Update();
	}

	//ここでプレイヤー同士の接続を行う
	if (nIns.GetMode() == NET_MODE::HOST)//ModeがHost(リーダー)なら
	{
		auto& players = NetManager::GetInstance().GetNetUsers();
		if (players.size() >= 1)//自分のほかにつながっているプレイヤーがいるなら
		{
			// マウスでの操作
			if (ins.IsClickMouseLeft())//左クリックで
			{
				Vector2 moPos = ins.GetMousePos();//リーダーだけがスタート可能

				if (B1_S_POS.x <= moPos.x && B1_E_POS.x >= moPos.x
					&& B1_S_POS.y <= moPos.y && B1_E_POS.y >= moPos.y)
				{
					//音の再生
					SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

					//リザルトの初期化
					GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);
					GameManager::GetInstance().InitQuest(1);

					//ネットマネージャの状態変更
					NetManager::GetInstance().ChangeGameState(GAME_STATE::GOTO_GAME);//ゲーム準備OK!
				}
			}
			//キーボード、コントローラーでの操作
			else if (ins.IsPadBtnTrgDown(jno, InputManager::JOYPAD_BTN::RIGHT)
				|| ins.IsTrgDown(KEY_INPUT_SPACE))
			{
				//音の再生
				SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

				//リザルトの初期化
				GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);
				GameManager::GetInstance().InitQuest(1);

				//ネットマネージャの状態変更
				NetManager::GetInstance().ChangeGameState(GAME_STATE::GOTO_GAME);//ゲーム準備OK!
			}
		}
	}

	//プレイ人数が一人のみの場合タイトルに戻れる
	if (players.size() <= 1) {
		//タイトルへ戻る
		if (ins.IsPadBtnTrgDown(jno, InputManager::JOYPAD_BTN::DOWN)
			|| ins.IsTrgDown(KEY_INPUT_BACK))
		{
			nIns.ResetSync();
			return;
		}
	}

	//ネットマネージャがGOTO_GAMEならゲームスタート
	if (nIns.IsSameGameState(GAME_STATE::GOTO_GAME))
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::GAME_START, Sound::TIMES::ONCE, true);
		//リザルトの初期化
		GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);
		GameManager::GetInstance().InitQuest(1);

		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);//ゲームスタート！
	}

}

void ConnectScene::Draw(void)
{
	Renderer_->Draw();

	int HX = Application::SCREEN_SIZE_X / 2;
	int HY = Application::SCREEN_SIZE_Y / 2;
	
	
	auto& players = NetManager::GetInstance().GetNetUsers();
	int i = 0;
	for (const auto& users : players)
	{
		players_.at(i)->Draw();
		i++;
	}

	std::wstring msg = L"接続中";
	if (NetManager::GetInstance().GetMode() == NET_MODE::HOST)
	{
		msg = L"接続待ち";
	}
	DrawString(HX - 100, HY, msg.c_str(), 0xffffff);

	int y = HY + 50;

	for (const auto& users : players)
	{

		if (users.second.mode == NET_MODE::HOST)
		{
			IPDATA ip = users.second.ip;


			Vector2(HX - WIDTH / 2, B1_Y - HEIGHT / 2);


			std::wstring out = L"接続番号:";
			//out += std::to_wstring(users.second.key) + L" ";
			out += std::to_wstring(ip.d1);
			out += L".";
			out += std::to_wstring(ip.d2);
			out += L".";
			out += std::to_wstring(ip.d3);
			out += L".";
			out += std::to_wstring(ip.d4);
			//out += L" : " + std::to_wstring(users.second.port);

			//DrawString(HX - 100, y, out.c_str(), 0xffffff);
			int len = (int)wcslen(out.c_str());
			int width = GetDrawStringWidth(out.c_str(), len);

			DrawBox(HX - (width / 2) - 10, y - (HEIGHT / 2)
				, HX + (width / 2) + 10, y + (HEIGHT / 2), 0x000000, true);
			DrawFormatString(HX - (width / 2), y- (len/2), 0xffffff, out.c_str());

			y += 20;
		}
	}

	if (NetManager::GetInstance().GetMode() == NET_MODE::HOST)
	{
		auto& players = NetManager::GetInstance().GetNetUsers();
		if (players.size() >= 1)
		{
			DrawBox(B1_S_POS.x, B1_S_POS.y, B1_E_POS.x, B1_E_POS.y, 0x000000, true);
			DrawString(B1_S_POS.x + 50, B1_S_POS.y + 5, L"進む", 0xffffff);
		}
	}

#ifdef DEBUG

	auto& nIns = NetManager::GetInstance();
	std::string text = "";

	switch (nIns.GetWeapon(NetManager::GetInstance().GetSelf().key))
	{
	case 0:
		text = "木の剣";
		break;
	case 1:
		text = "バスターソード";
		break;
	}
	DrawFormatString(100, 250, 0xffffff, "使用武器：%s", text.c_str());

#endif // DEBUG

}

void ConnectScene::Release(void)
{
}

void ConnectScene::AddPlayer(const VECTOR pos, const VECTOR rot)
{
	auto player_ = std::make_unique<ViewPlayer>();
	player_->Init();
	player_->SetChar(0);
	player_->SetWepon(GameManager::GetInstance().GetWeponId());
	player_->SetPos(pos);
	player_->SetLocalQua(rot);
	player_->Update();
	players_.push_back(std::move(player_));
}
