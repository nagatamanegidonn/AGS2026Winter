#include <string>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Camera.h"

#include "../Net/NetManager.h"
#include "../Common/InputTextArea.h"
#include "../Utility/Utility.h"
#include "../Object/Player/ViewPlayer.h"
#include "../Object/Common/InputController.h"
// シェーダ
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "TitleScene.h"

namespace
{
	// ファイルパス・リソース名
	const std::wstring SHADER_TEX = L"Texture.cso";
	// UI表示用文字列
	const std::wstring TEXT_PUSH_START = L"PushSpace or B";
	const std::wstring TEXT_HOST = L"HOST";
	const std::wstring TEXT_CLIENT = L"CLIENT";
	const std::wstring TEXT_DOT = L".";
	const std::wstring TEXT_CHANGE_WP = L"武器変更";
	const std::wstring TEXT_GAME_START = L"出撃";
	const std::wstring WP_NAME_SWORD = L"片手剣";
	const std::wstring WP_NAME_GREAT = L"大剣";
	const std::wstring WP_NAME_BOW = L"弓";
	// UI配置・文字列描画用の微調整（オフセット）
	constexpr int UI_TEXT_OFFSET_X = 50;	// ボタン左端からテキスト開始位置までの余白
	constexpr int UI_TEXT_OFFSET_Y = 7;		// ボタン上端からテキスト開始位置までの余白
	constexpr int IP_MAX_LENGTH = 15;	// IPアドレスの最大文字数 ("xxx.xxx.xxx.xxx")
	// カラーコード (RGB)
	constexpr unsigned int COLOR_BLACK = 0x000000;
	constexpr unsigned int COLOR_WHITE = 0xffffff;
	// キャラクター初期設定
	constexpr int INITIAL_CHAR_ID = 0; // ナイト
	// シェーダ・レンダラー設定
	constexpr int SHADER_BUFFER_NUM = 1;
	// ボタンサイズ
	const int WIDTH = 200;
	const int HALF_WIDTH = 200 / 2;
	const int HEIGHT = 30;
	const int HALF_HEIGHT = 30 / 2;
	// 画面中心位置
	int HX = Application::SCREEN_SIZE_X / 2;
	int HY = Application::SCREEN_SIZE_Y / 2;
	// ボタン位置
	const int B1_Y = Application::SCREEN_SIZE_Y - 100;
	const Vector2 B1_C_POS = Vector2(845, 80);
	const Vector2 B1_S_POS = Vector2(B1_C_POS.x - HALF_WIDTH, B1_C_POS.y - HALF_HEIGHT);
	const Vector2 B1_E_POS = Vector2(B1_C_POS.x + HALF_WIDTH, B1_C_POS.y + HALF_HEIGHT);
	// IPアドレスボタンPos
	const Vector2 IP_C_POS = Vector2(845, 80 + 60);
	const Vector2 IP_S_POS = Vector2(IP_C_POS.x - HALF_WIDTH, IP_C_POS.y - HALF_HEIGHT);
	const Vector2 IP_E_POS = Vector2(IP_C_POS.x + HALF_WIDTH, IP_C_POS.y + HALF_HEIGHT);
	// weaponボタンPos
	const Vector2 WP_C_POS = Vector2(IP_C_POS.x, IP_C_POS.y + 60);
	const Vector2 WP_S_POS = Vector2(WP_C_POS.x - HALF_WIDTH, WP_C_POS.y - HALF_HEIGHT);
	const Vector2 WP_E_POS = Vector2(WP_C_POS.x + HALF_WIDTH, WP_C_POS.y + HALF_HEIGHT);
	// startボタンPos
	const int B2_Y = B1_Y + 40;
	const Vector2 B2_C_POS = Vector2(HX, B2_Y);
	const Vector2 B2_S_POS = Vector2(B2_C_POS.x - HALF_WIDTH, B2_C_POS.y - HALF_HEIGHT);
	const Vector2 B2_E_POS = Vector2(B2_C_POS.x + HALF_WIDTH, B2_C_POS.y + HALF_HEIGHT);
}

TitleScene::TitleScene(void)
	:
	SceneBase(),
	viewPlayer_(nullptr),
	inputTextArea_(nullptr),
	cursorMaterial_(nullptr),
	cursorRenderer_(nullptr),
	backGroundMaterial_(nullptr),
	backGroundRenderer_(nullptr),
	titleMaterial_(nullptr),
	titleRenderer_(nullptr),
	isTitle_(false),
	selectId_(0),
	weponId_(0),
	isWpSelect_(false),
	typeUpdate_(nullptr),
	padUpdate_(nullptr),
	mouseUpdate_(nullptr),
	isPad_(false),
	agoMousePos_(),
	cursorImg_(-1),
	backImg_(-1),
	titleImg_(-1),
	inputController_(nullptr)
{
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// カメラの設定
	SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FIXED_POINT);

	// コントローラーの登録
	inputController_ = std::make_unique<InputController>(GameManager::GetInstance().GetControllId());

	// 画像の読み込み
	cursorImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::CURSOR).handleId_;
	backImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_BACK).handleId_;
	titleImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TITLE_LOGO).handleId_;

	// IPアドレス入力エリア生成
	auto hostIp = NetManager::GetInstance().GetHostIp();
	inputTextArea_ = std::make_unique<InputTextArea>(
		Vector2{ IP_S_POS.x, IP_S_POS.y }, // 引数が構造体の場合は明示的に型を書くか、そのまま渡す
		Vector2{ IP_E_POS.x - IP_S_POS.x, IP_E_POS.y - IP_S_POS.y },
		IP_MAX_LENGTH
	);

	// デフォルトIPアドレス設定
	std::wstring defaultIp =
		std::to_wstring(hostIp.d1) + TEXT_DOT +
		std::to_wstring(hostIp.d2) + TEXT_DOT +
		std::to_wstring(hostIp.d3) + TEXT_DOT +
		std::to_wstring(hostIp.d4);

	// IPアドレス初期設定
	inputTextArea_->SetText(defaultIp);

	// ソケット再生成、ユーザー情報リセット
	NetManager::GetInstance().Init();

	// プレイヤー生成
	viewPlayer_ = std::make_unique<ViewPlayer>();
	viewPlayer_->Init();
	viewPlayer_->SetChar(INITIAL_CHAR_ID);
	viewPlayer_->SetWeapon(GameManager::GetInstance().GetWeaponId());

	// 選択中の項目
	selectId_ = (int)MENU::GAME_START;
	weponId_ = GameManager::GetInstance().GetWeaponId();
	isWpSelect_ = false;

	// 武器選択用の位置登録
	AddPosTri(WP_NAME_SWORD, 0, Vector2(WIDTH, HEIGHT), Vector2(WP_C_POS.x + UI_TEXT_OFFSET_X, WP_C_POS.y + (HEIGHT)));
	AddPosTri(WP_NAME_GREAT, 1, Vector2(WIDTH, HEIGHT), Vector2(WP_C_POS.x + UI_TEXT_OFFSET_X, WP_C_POS.y + (HEIGHT * 2)));
	AddPosTri(WP_NAME_BOW, 2, Vector2(WIDTH, HEIGHT), Vector2(WP_C_POS.x + UI_TEXT_OFFSET_X, WP_C_POS.y + (HEIGHT * 3)));

	// マウス操作の設定
	isPad_ = false;

	// 更新ステップの初期設定
	typeUpdate_ = std::bind(&TitleScene::UpdateMouse, this);
	padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
	mouseUpdate_ = std::bind(&TitleScene::MouseUpdate, this);

	// マウスポインタを画面中央に移動
	SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);

	auto& input = InputManager::GetInstance();
	agoMousePos_ = input.GetMousePos();

	// カーソル画像
	cursorMaterial_ = std::make_unique<PixelMaterial>(SHADER_TEX.c_str(), SHADER_BUFFER_NUM);
	cursorMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	cursorMaterial_->AddTextureBuf(cursorImg_);
	cursorRenderer_ = std::make_unique<PixelRenderer>(*cursorMaterial_);
	cursorRenderer_->SetSize(Vector2(HEIGHT, HEIGHT));

	// 背景画像
	backGroundMaterial_ = std::make_unique<PixelMaterial>(SHADER_TEX.c_str(), SHADER_BUFFER_NUM);
	backGroundMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	backGroundMaterial_->AddTextureBuf(backImg_);
	backGroundRenderer_ = std::make_unique<PixelRenderer>(*backGroundMaterial_);
	backGroundRenderer_->MakeSquereVertex(
		Vector2(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

	// タイトル画像
	titleMaterial_ = std::make_unique<PixelMaterial>(SHADER_TEX.c_str(), SHADER_BUFFER_NUM);
	titleMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	titleMaterial_->AddTextureBuf(titleImg_);
	titleRenderer_ = std::make_unique<PixelRenderer>(*titleMaterial_);
	titleRenderer_->MakeSquereVertex(
		Vector2(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);

	// タイトル画面フラグ
	isTitle_ = true;
}

void TitleScene::Update(void)
{
	auto& input = InputManager::GetInstance();
	Vector2 moPos = input.GetMousePos();

	// 入力情報の更新
	inputController_->Update();

	// プレイヤーの更新
	viewPlayer_->Update();

	// IPアドレス入力エリアの更新
	inputTextArea_->Update();

	// 更新処理の呼び出し
	typeUpdate_();

	// 前フレームのマウス情報として保存
	agoMousePos_ = moPos;
}

void TitleScene::Draw(void)
{
	auto& input = InputManager::GetInstance();

	// 背景描画
	backGroundRenderer_->Draw();

	// タイトル画面
	if (isTitle_)
	{
		// タイトルロゴの描画
		titleRenderer_->Draw();
		int cx = Application::SCREEN_SIZE_X / 2;
		int len = (int)wcslen(TEXT_PUSH_START.c_str());
		int width = GetDrawStringWidth(TEXT_PUSH_START.c_str(), len);
		DrawFormatString(cx - (width / 2), B2_S_POS.y + HEIGHT / 2, COLOR_WHITE, TEXT_PUSH_START.c_str());

		return;
	}

	// プレイヤー描画
	viewPlayer_->Draw();

	// ホストorクライアント
	DrawBox(B1_S_POS.x, B1_S_POS.y, B1_E_POS.x, B1_E_POS.y, COLOR_BLACK, true);
	DrawBox(B1_S_POS.x, B1_S_POS.y, B1_E_POS.x, B1_E_POS.y, COLOR_WHITE, false);
	if (GameManager::GetInstance().IsHost())
	{
		DrawString(B1_S_POS.x + UI_TEXT_OFFSET_X, B1_S_POS.y + UI_TEXT_OFFSET_Y, TEXT_HOST.c_str(), COLOR_WHITE);
	}
	else
	{
		DrawString(B1_S_POS.x + UI_TEXT_OFFSET_X, B1_S_POS.y + UI_TEXT_OFFSET_Y, TEXT_CLIENT.c_str(), COLOR_WHITE);
	}

	// 武器変更
	DrawBox(WP_S_POS.x, WP_S_POS.y, WP_E_POS.x, WP_E_POS.y, COLOR_BLACK, true);
	DrawBox(WP_S_POS.x, WP_S_POS.y, WP_E_POS.x, WP_E_POS.y, COLOR_WHITE, false);
	DrawString(WP_S_POS.x + UI_TEXT_OFFSET_X, WP_S_POS.y + UI_TEXT_OFFSET_Y, TEXT_CHANGE_WP.c_str(), COLOR_WHITE);

	// 出撃
	DrawBox(B2_S_POS.x, B2_S_POS.y, B2_E_POS.x, B2_E_POS.y, COLOR_BLACK, true);
	DrawBox(B2_S_POS.x, B2_S_POS.y, B2_E_POS.x, B2_E_POS.y, COLOR_WHITE, false);
	DrawString(B2_S_POS.x + UI_TEXT_OFFSET_X, B2_S_POS.y + UI_TEXT_OFFSET_Y, TEXT_GAME_START.c_str(), COLOR_WHITE);

	// IPアドレス
	inputTextArea_->Draw();

	// カーソル描画
	if (!isWpSelect_)
	{
		if (!isPad_) { return; }

		switch (selectId_)
		{
		case (int)MENU::NET_SELECT:
			cursorRenderer_->Draw(B1_S_POS.x - HEIGHT, B1_C_POS.y);
			break;
		case (int)MENU::GAME_START:
			cursorRenderer_->Draw(B2_S_POS.x - HEIGHT, B2_C_POS.y);
			break;
		case (int)MENU::WEPON_SELECT:
			cursorRenderer_->Draw(WP_S_POS.x - HEIGHT, WP_C_POS.y);
			break;
		case (int)MENU::IP_SET:
			cursorRenderer_->Draw(IP_S_POS.x - HEIGHT, IP_C_POS.y);
			break;
		default:
			break;
		}
	}
	// 武器選択中
	else
	{
		// 武器選択肢描画
		for (const auto& wPos : weponsPos_)
		{
			DrawBox(wPos.second->StartPos.x, wPos.second->StartPos.y, wPos.second->EndPos.x, wPos.second->EndPos.y, COLOR_BLACK, true);
			DrawString(wPos.second->StartPos.x + UI_TEXT_OFFSET_X, wPos.second->StartPos.y + UI_TEXT_OFFSET_Y, wPos.second->Name.c_str(), COLOR_WHITE);
		}
		// カーソル描画
		if (isPad_)
		{
			cursorRenderer_->Draw(weponsPos_.at(weponId_)->StartPos.x - HEIGHT, weponsPos_.at(weponId_)->CenterPos.y);
		}
	}
}

void TitleScene::Release(void)
{
	if (inputTextArea_)
	{
		inputTextArea_->Release();
	}
}

#pragma region マウス更新かパッド更新

void TitleScene::UpdateMouse(void)
{
	if (inputController_->IsPeripheralTriggered(InputController::PeripheralType::GAME_PAD)
		|| inputController_->IsPeripheralTriggered(InputController::PeripheralType::GAME_PAD_INS)
		|| inputController_->IsPeripheralTriggered(InputController::PeripheralType::GAME_PAD_STICK)
		|| inputController_->IsPeripheralTriggered(InputController::PeripheralType::KEYBOARD)
		)
	{
		isPad_ = true;
		// マウスを非表示状態にする
		SetMouseDispFlag(false);
		typeUpdate_ = std::bind(&TitleScene::UpdatePad, this);
		return;
	}

	if (isTitle_)
	{
		if (InputManager::GetInstance().IsTrgMouseLeft())
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);
			isTitle_ = false;
		}
		return;
	}

	mouseUpdate_();
}

void TitleScene::UpdatePad(void)
{
	auto& input = InputManager::GetInstance();
	Vector2 moPos = input.GetMousePos();

	if (inputController_->IsPeripheralTriggered(InputController::PeripheralType::MOUSE)
		|| (agoMousePos_.x != moPos.x && agoMousePos_.y != moPos.y)
		)
	{
		isPad_ = false;
		SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);
		// マウスを表示状態にする
		SetMouseDispFlag(true);
		typeUpdate_ = std::bind(&TitleScene::UpdateMouse, this);
		return;
	}

	if (isTitle_)
	{
		if (inputController_->IsTriggered(InputController::KEY::OK_SECOND))
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);
			isTitle_ = false;
		}
		return;
	}

	// 更新処理
	padUpdate_();
}

#pragma endregion

// マウス更新
void TitleScene::MouseUpdate(void)
{
	auto& input = InputManager::GetInstance();

	if (inputTextArea_->IsActive())
	{
		selectId_ = (int)MENU::IP_SET;
		padUpdate_ = std::bind(&TitleScene::PIpUpdate, this);
	}

	// クリックしたとき
	if (!inputTextArea_->IsActive() && input.IsTrgMouseLeft())
	{
		Vector2 moPos = input.GetMousePos();

		// ホストになることを選択orクライアントになることを選択
		if (B1_S_POS.x <= moPos.x && B1_E_POS.x >= moPos.x
			&& B1_S_POS.y <= moPos.y && B1_E_POS.y >= moPos.y)
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

			selectId_ = (int)MENU::NET_SELECT;
			if (GameManager::GetInstance().IsHost())
			{
				GameManager::GetInstance().SetHost(false);
			}
			else
			{
				GameManager::GetInstance().SetHost(true);
			}
		}
		// ゲームスタート
		else if (B2_S_POS.x <= moPos.x && B2_E_POS.x >= moPos.x
			&& B2_S_POS.y <= moPos.y && B2_E_POS.y >= moPos.y)
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

			selectId_ = (int)MENU::GAME_START;
			if (GameManager::GetInstance().IsHost())
			{
				NetManager::GetInstance().Run(NET_MODE::HOST);
				SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CONNECT);
			}
			else
			{
				std::wstring ipStr = inputTextArea_->GetText();
				int d1, d2, d3, d4;
				if (swscanf_s(ipStr.c_str(), L"%d.%d.%d.%d", &d1, &d2, &d3, &d4) == 4) {
					IPDATA hostIp{
						static_cast<unsigned char>(d1),
						static_cast<unsigned char>(d2),
						static_cast<unsigned char>(d3),
						static_cast<unsigned char>(d4)
					};
					NetManager::GetInstance().SetHostIp(hostIp);

					NetManager::GetInstance().Run(NET_MODE::CLIENT);
					SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CONNECT);
				}
			}
		}
		// 武器選択
		else if (WP_S_POS.x <= moPos.x && WP_E_POS.x >= moPos.x
			&& WP_S_POS.y <= moPos.y && WP_E_POS.y >= moPos.y)
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

			selectId_ = (int)MENU::WEPON_SELECT;
			isWpSelect_ = true;
			padUpdate_ = std::bind(&TitleScene::PWeaponUpdate, this);
			mouseUpdate_ = std::bind(&TitleScene::MWeaponUpdate, this);
		}
	}
	else if (inputTextArea_->IsActive() && input.IsTrgMouseLeft())
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

		if (!inputTextArea_->IsCleckText())
		{
			int handle = inputTextArea_->GetKeyHandle();

			// 現在の入力内容を強制的に文字列に反映
			inputTextArea_->SetKeyInputStringBuffer();

			// 入力を非アクティブにする（＝強制終了）
			SetActiveKeyInput(-1);	// DxLib関数：現在のキー入力を終了

			// 管理クラスに通知して状態を初期化
			InputTextManager::GetInstance().SetTextArea(false);
		}
	}
}

void TitleScene::MWeaponUpdate(void)
{
	auto& input = InputManager::GetInstance();
	auto& sns = SceneManager::GetInstance();
	auto& gns = GameManager::GetInstance();

	// クリックしたとき
	if (input.IsTrgMouseLeft())
	{
		Vector2 moPos = input.GetMousePos();
		bool isSlect = false;

		for (const auto& wPos : weponsPos_)
		{
			if (wPos.second->StartPos.x <= moPos.x && wPos.second->EndPos.x >= moPos.x
				&& wPos.second->StartPos.y <= moPos.y && wPos.second->EndPos.y >= moPos.y)
			{
				SoundManager::GetInstance().Play(SoundManager::SRC::SELECT, Sound::TIMES::ONCE, true);

				isSlect = true;
				weponId_ = wPos.first;
				gns.SetWeaponId(weponId_);
				viewPlayer_->SetWeapon(GameManager::GetInstance().GetWeaponId());
				break;
			}
		}
		// 選択してないなら戻る
		if (!isSlect)
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

			isWpSelect_ = false;
			padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
			mouseUpdate_ = std::bind(&TitleScene::MouseUpdate, this);
			return;
		}
	}
}

// 通常更新（コントロ－ラー）
void TitleScene::PNormalUpdate(void)
{
	auto& input = InputManager::GetInstance();
	auto& sns = SceneManager::GetInstance();

	if (inputTextArea_->IsActive())
	{
		selectId_ = (int)MENU::IP_SET;
	}

	if (inputController_->IsTriggered(InputController::KEY::FORWARD) && selectId_ > (int)MENU::NET_SELECT)
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::SELECT, Sound::TIMES::ONCE, true);
		selectId_--;
	}
	else if (inputController_->IsTriggered(InputController::KEY::BACK) && selectId_ < (int)MENU::MAX - 1)
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::SELECT, Sound::TIMES::ONCE, true);
		selectId_++;
	}
	if (!inputTextArea_->IsActive() && inputController_->IsTriggered(InputController::KEY::OK_SECOND))
	{
		switch (selectId_)
		{
		case (int)MENU::NET_SELECT:
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

			if (GameManager::GetInstance().IsHost())
			{
				GameManager::GetInstance().SetHost(false);
			}
			else
			{
				GameManager::GetInstance().SetHost(true);
			}
		}
		break;
		case (int)MENU::GAME_START:
		{
			SoundManager::GetInstance().Play(SoundManager::SRC::START, Sound::TIMES::ONCE, true);

			// マウスを表示状態にする
			SetMouseDispFlag(true);

			if (GameManager::GetInstance().IsHost())
			{
				NetManager::GetInstance().Run(NET_MODE::HOST);
				SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CONNECT);
			}
			else
			{
				std::wstring ipStr = inputTextArea_->GetText();
				int d1, d2, d3, d4;
				if (swscanf_s(ipStr.c_str(), L"%d.%d.%d.%d", &d1, &d2, &d3, &d4) == 4) {
					IPDATA hostIp{
						static_cast<unsigned char>(d1),
						static_cast<unsigned char>(d2),
						static_cast<unsigned char>(d3),
						static_cast<unsigned char>(d4)
					};
					NetManager::GetInstance().SetHostIp(hostIp);

					NetManager::GetInstance().Run(NET_MODE::CLIENT);
					SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CONNECT);
				}
			}
		}
		break;
		case (int)MENU::WEPON_SELECT:
			SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

			isWpSelect_ = true;
			padUpdate_ = std::bind(&TitleScene::PWeaponUpdate, this);
			mouseUpdate_ = std::bind(&TitleScene::MWeaponUpdate, this);
			break;
		case (int)MENU::IP_SET:
			SoundManager::GetInstance().Play(SoundManager::SRC::SELECT, Sound::TIMES::ONCE, true);

			inputTextArea_->TextActive();
			padUpdate_ = std::bind(&TitleScene::PIpUpdate, this);
			break;
		default:
			break;
		}
	}
}

void TitleScene::PWeaponUpdate(void)
{
	auto& input = InputManager::GetInstance();
	auto& sns = SceneManager::GetInstance();
	auto& gns = GameManager::GetInstance();

	if (inputController_->IsTriggered(InputController::KEY::OK_SECOND))
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

		isWpSelect_ = false;
		padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
		mouseUpdate_ = std::bind(&TitleScene::MouseUpdate, this);
		return;
	}

	if (inputController_->IsTriggered(InputController::KEY::FORWARD))
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::SELECT, Sound::TIMES::ONCE, true);
		weponId_ = (weponId_ - 1 + (int)WEPON_ID::MAX) % ((int)WEPON_ID::MAX);

		gns.SetWeaponId(weponId_);
		viewPlayer_->SetWeapon(GameManager::GetInstance().GetWeaponId());
	}
	else if (inputController_->IsTriggered(InputController::KEY::BACK))
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::SELECT, Sound::TIMES::ONCE, true);
		weponId_ = (weponId_ + 1) % ((int)WEPON_ID::MAX);

		gns.SetWeaponId(weponId_);
		viewPlayer_->SetWeapon(GameManager::GetInstance().GetWeaponId());
	}
}

void TitleScene::PIpUpdate(void)
{
	if (!inputTextArea_->IsActive())
	{
		padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
	}
	if (inputTextArea_->IsActive() && inputController_->IsTriggered(InputController::KEY::OK_SECOND))
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

		int handle = inputTextArea_->GetKeyHandle();

		// 現在の入力内容を強制的に文字列に反映
		inputTextArea_->SetKeyInputStringBuffer();

		// 入力を非アクティブにする（＝強制終了）
		SetActiveKeyInput(-1);

		// 管理クラスに通知して状態を初期化
		InputTextManager::GetInstance().SetTextArea(false);

		padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
	}
}

void TitleScene::AddPosTri(std::wstring name, int weponId, const Vector2 size, const Vector2 cPos)
{
	auto posTri = std::make_unique<PosTri>();
	posTri->Name = name;
	posTri->WIDTH = size.x;
	posTri->HEIGHT = size.y;

	posTri->CenterPos = cPos;
	posTri->StartPos = Vector2(cPos.x - size.x / 2, cPos.y - size.y / 2);
	posTri->EndPos = Vector2(cPos.x + size.x / 2, cPos.y + size.y / 2);

	weponsPos_.emplace(weponId, std::move(posTri));
}