#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/ResourceManager.h"

#include "../Net/NetManager.h"
#include "../Utility/AsoUtility.h"

#include "../Object/Player/ViewPlayer.h"

#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "ConnectScene.h"

namespace
{
	// ファイルパス・リソース名
	const std::wstring SHADER_TEX = L"Texture.cso";
	// UI表示用テキスト
	const std::wstring TEXT_CONNECTING = L"接続中";
	const std::wstring TEXT_WAIT_CONNECT = L"接続待ち";
	const std::wstring TEXT_PREFIX_NUM = L"接続番号:";
	const std::wstring TEXT_DOT = L".";
	const std::wstring TEXT_GO_AHEAD = L"進む";
	// UI配置・文字列描画用の微調整（オフセット）
	constexpr int UI_TEXT_OFFSET_X = 50;	// ボタン左端からテキスト開始位置までの余白
	constexpr int UI_TEXT_OFFSET_Y = 5;		// ボタン上端からテキスト開始位置までの余白
	constexpr int UI_BOX_MARGIN_X = 10;		// IPアドレス背景ボックスの横方向の余白
	constexpr int UI_Y_STEP = 20;			// IPアドレス一覧を下にずらす幅
	constexpr int INITIAL_Y_OFFSET = 50;	// 画面中央からIPアドレス表示開始位置までの初期オフセット
	// カラーコード (RGB) 
	constexpr unsigned int COLOR_BLACK = 0x000000;
	constexpr unsigned int COLOR_WHITE = 0xffffff;
	// キャラクター初期設定
	constexpr int INITIAL_CHAR_ID = 0;       // ナイト
	constexpr int INITIAL_PLAYER_COUNT = 1;
	// クエスト設定
	constexpr int FIRST_QUEST_ID = 1;        // 最初に読み込むクエスト番号
	// シェーダ・レンダラー設定
	constexpr int SHADER_BUFFER_NUM = 1;
}

ConnectScene::ConnectScene(void)
{
	DeleteGraph(backImg_);
}

ConnectScene::~ConnectScene(void)
{
}

void ConnectScene::Init(void)
{
	// 画像の読み込み
	backImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_BACK).handleId_;

	// ポストエフェクト用(モノトーン)
	backGroundMaterial_ = std::make_unique<PixelMaterial>(SHADER_TEX.c_str(), SHADER_BUFFER_NUM);
	backGroundMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	backGroundMaterial_->AddTextureBuf(backImg_);
	backGroundRenderer_ = std::make_unique<PixelRenderer>(*backGroundMaterial_);
	backGroundRenderer_->MakeSquereVertex(
		Vector2(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

#pragma region プレイヤーの作成

	playerNum_ = INITIAL_PLAYER_COUNT;

	AddPlayer({ -150.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(-20.0f), AsoUtility::Deg2RadF(0.0f) });
	AddPlayer({ -50.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(0.0f), AsoUtility::Deg2RadF(0.0f) });
	AddPlayer({ 50.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(0.0f), AsoUtility::Deg2RadF(0.0f) });
	AddPlayer({ 150.0f,-50.0f,-250.0f },
		{ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(20.0f), AsoUtility::Deg2RadF(0.0f) });

#pragma endregion

	// ネットマネージャの状態変更
	NetManager::GetInstance().ChangeGameState(GAME_STATE::CONNECTING); // ネット通信受け付ける状態に変更

	auto& nIns = NetManager::GetInstance();
	if (nIns.GetMode() == NET_MODE::HOST) // ModeがHost(リーダー)なら
	{
		// 音の再生
		SoundManager::GetInstance().Play(SoundManager::SRC::CONECT_START, Sound::TIMES::ONCE, true);
	}
}

void ConnectScene::Update(void)
{
	// インスタンスクラスの取得
	auto& nIns = NetManager::GetInstance();
	const auto& ins = InputManager::GetInstance();

	// 武器情報の送信
	nIns.SetWeapon(NetManager::GetInstance().GetSelf().key, GameManager::GetInstance().GetWeaponId());

	const InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(GameManager::GetInstance().GetControllId());
	auto& users = NetManager::GetInstance().GetNetUsers();
	if (playerNum_ < users.size())
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::ADD, Sound::TIMES::ONCE, true);
		playerNum_ = static_cast<int>(users.size());
	}

	int i = 0;
	for (const auto& user : users)
	{
		// プレイヤーごとの武器の設定
		players_.at(i)->SetWeapon(nIns.GetWeapon(user.second.key));
		i++;
	}

	// プレイヤーの更新
	for (const auto& player : players_)
	{
		player->Update();
	}

	// ここでプレイヤー同士の接続を行う
	if (nIns.GetMode() == NET_MODE::HOST) // ModeがHost(リーダー)なら
	{
		auto& players = NetManager::GetInstance().GetNetUsers();
		if (players.size() >= 1) // 自分のほかにつながっているプレイヤーがいるなら
		{
			// マウスでの操作
			if (ins.IsClickMouseLeft()) // 左クリックで
			{
				Vector2 moPos = ins.GetMousePos();

				if (B1_S_POS.x <= moPos.x && B1_E_POS.x >= moPos.x
					&& B1_S_POS.y <= moPos.y && B1_E_POS.y >= moPos.y)
				{
					// 音の再生
					SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

					// リザルトの初期化
					GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);
					GameManager::GetInstance().InitQuest(FIRST_QUEST_ID);

					// ネットマネージャの状態変更
					NetManager::GetInstance().ChangeGameState(GAME_STATE::GOTO_GAME); // ゲーム準備OK!
				}
			}
			// キーボード、コントローラーでの操作
			else if (ins.IsPadBtnTrgDown(jno, InputManager::JOYPAD_BTN::RIGHT)
				|| ins.IsTrgDown(KEY_INPUT_SPACE))
			{
				// 音の再生
				SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

				// リザルトの初期化
				GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);
				GameManager::GetInstance().InitQuest(FIRST_QUEST_ID);

				// ネットマネージャの状態変更
				NetManager::GetInstance().ChangeGameState(GAME_STATE::GOTO_GAME); // ゲーム準備OK!
			}
		}
	}

	// プレイ人数が一人のみの場合タイトルに戻れる
	if (users.size() <= 1) {
		// タイトルへ戻る
		if (ins.IsPadBtnTrgDown(jno, InputManager::JOYPAD_BTN::DOWN)
			|| ins.IsTrgDown(KEY_INPUT_BACK))
		{
			nIns.ResetSync();
			return;
		}
	}

	// ネットマネージャがGOTO_GAMEならゲームスタート
	if (nIns.IsSameGameState(GAME_STATE::GOTO_GAME))
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::GAME_START, Sound::TIMES::ONCE, true);
		// リザルトの初期化
		GameManager::GetInstance().SetGameResult(GameManager::GAME_RESULT::NONE);
		GameManager::GetInstance().InitQuest(FIRST_QUEST_ID);

		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME); // ゲームスタート！
	}
}

void ConnectScene::Draw(void)
{
	backGroundRenderer_->Draw();

	int HX = Application::SCREEN_SIZE_X / 2;
	int HY = Application::SCREEN_SIZE_Y / 2;

	auto& users = NetManager::GetInstance().GetNetUsers();
	int i = 0;
	for (const auto& user : users)
	{
		players_.at(i)->Draw();
		i++;
	}

	std::wstring msg = TEXT_CONNECTING;
	if (NetManager::GetInstance().GetMode() == NET_MODE::HOST)
	{
		msg = TEXT_WAIT_CONNECT;
	}

	// 接続待ちの表示
	DrawString(HX - 100, HY, msg.c_str(), COLOR_WHITE);

	int y = HY + INITIAL_Y_OFFSET;

	for (const auto& user : users)
	{
		if (user.second.mode == NET_MODE::HOST)
		{
			IPDATA ip = user.second.ip;
			Vector2(HX - WIDTH / 2, B1_Y - HEIGHT / 2);

			std::wstring out = TEXT_PREFIX_NUM;
			out += std::to_wstring(ip.d1);
			out += TEXT_DOT;
			out += std::to_wstring(ip.d2);
			out += TEXT_DOT;
			out += std::to_wstring(ip.d3);
			out += TEXT_DOT;
			out += std::to_wstring(ip.d4);

			int len = (int)wcslen(out.c_str());
			int width = GetDrawStringWidth(out.c_str(), len);

			DrawBox(HX - (width / 2) - UI_BOX_MARGIN_X, y - (HEIGHT / 2),
				HX + (width / 2) + UI_BOX_MARGIN_X, y + (HEIGHT / 2), COLOR_BLACK, true);
			DrawFormatString(HX - (width / 2), y - (len / 2), COLOR_WHITE, out.c_str());

			y += UI_Y_STEP;
		}
	}

	if (NetManager::GetInstance().GetMode() == NET_MODE::HOST)
	{
		auto& players = NetManager::GetInstance().GetNetUsers();
		if (players.size() >= 1)
		{
			DrawBox(B1_S_POS.x, B1_S_POS.y, B1_E_POS.x, B1_E_POS.y, COLOR_BLACK, true);
			DrawString(B1_S_POS.x + UI_TEXT_OFFSET_X, B1_S_POS.y + UI_TEXT_OFFSET_Y, TEXT_GO_AHEAD.c_str(), COLOR_WHITE);
		}
	}
}

void ConnectScene::Release(void)
{
}

// プレイヤーの追加
void ConnectScene::AddPlayer(const VECTOR pos, const VECTOR rot)
{
	auto player_ = std::make_unique<ViewPlayer>();
	player_->Init();
	player_->SetChar(INITIAL_CHAR_ID);
	player_->SetWeapon(GameManager::GetInstance().GetWeaponId());
	player_->SetPos(pos);
	player_->SetLocalQua(rot);
	player_->Update();
	players_.push_back(std::move(player_));
}