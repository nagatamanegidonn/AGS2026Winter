#include <string>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/GameManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/Camera.h"

#include "../Net/NetManager.h"
#include "../Common/InputTextArea.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Player/ViewPlayer.h"
#include "../Object/Common/InputController.h"
// シェーダ
#include "../Renderer/PixelMaterial.h"
#include "../Renderer/PixelRenderer.h"

#include "TitleScene.h"

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
	agoMouseTrg_(false),
	agoMousePos_(),
	cursorImg_(-1),
	backImg_(-1),
	titleImg_(-1),
	inputController_(nullptr)
{
}

TitleScene::~TitleScene(void)
{
	DeleteGraph(titleImg_);
	DeleteGraph(backImg_);
	DeleteGraph(cursorImg_);
}

void TitleScene::Init(void)
{
	// カメラの設定
	SceneManager::GetInstance().GetCamera().lock()->ChangeMode(Camera::MODE::FIXED_POINT);

	// コントローラーの登録
 	inputController_ = std::make_unique<InputController>(GameManager::GetInstance().GetControllId());

	// 画像の読み込み
	cursorImg_ = LoadGraph((Application::PATH_IMAGE + L"tile_0072.png").c_str());
	backImg_ = LoadGraph((Application::PATH_IMAGE + L"img.png").c_str());
	titleImg_ = LoadGraph((Application::PATH_IMAGE + L"TitleRogo.png").c_str());

	// IPアドレス入力エリア生成
	auto hostIp = NetManager::GetInstance().GetHostIp();
	inputTextArea_ = new InputTextArea(//ここの１５は最大文字数
		{ IP_S_POS.x, IP_S_POS.y }, { IP_E_POS.x - IP_S_POS.x, IP_E_POS.y - IP_S_POS.y }, 15);

	// デフォルトIPアドレス設定
	std::wstring defaultIp =
		std::to_wstring(hostIp.d1) + L"." +
		std::to_wstring(hostIp.d2) + L"." +
		std::to_wstring(hostIp.d3) + L"." +
		std::to_wstring(hostIp.d4);

	// IPアドレス初期設定
	inputTextArea_->SetText(defaultIp);

	// ソケット再生成、ユーザー情報リセット
	NetManager::GetInstance().Init(); 

	// プレイヤー生成
	viewPlayer_ = std::make_unique<ViewPlayer>();
	viewPlayer_->Init();
	viewPlayer_->SetChar(0);
	viewPlayer_->SetWeapon(GameManager::GetInstance().GetWeaponId());

	// 選択中の項目
	selectId_ = (int)MENU::GAME_START;
	weponId_ = GameManager::GetInstance().GetWeaponId();
	isWpSelect_ = false;

	// 武器選択用の位置登録
	AddPosTri(L"片手剣", 0, Vector2(WIDTH, HEIGHT)
		, Vector2(WP_C_POS.x + 50, WP_C_POS.y + (HEIGHT)));
	AddPosTri(L"大剣", 1, Vector2(WIDTH, HEIGHT)
		, Vector2(WP_C_POS.x + 50, WP_C_POS.y + (HEIGHT * 2)));
	AddPosTri(L"弓", 2, Vector2(WIDTH, HEIGHT)
		, Vector2(WP_C_POS.x + 50, WP_C_POS.y + (HEIGHT * 3)));

	// マウス操作の設定
	isPad_ = false;
	
	// 更新ステップの初期設定
	typeUpdate_ = std::bind(&TitleScene::UpdateMouse, this);
	padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
	mouseUpdate_ = std::bind(&TitleScene::MouseUpdate, this);

	// マウスポインタを画面中央に移動
	SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);

	auto& ins = InputManager::GetInstance();
	agoMousePos_ = ins.GetMousePos();
	agoMouseTrg_ = true;

	// カーソル画像
	cursorMaterial_ = std::make_unique<PixelMaterial>(L"Texture.cso", 1);
	cursorMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	cursorMaterial_->AddTextureBuf(cursorImg_);
	cursorRenderer_ = std::make_unique<PixelRenderer>(*cursorMaterial_);
	cursorRenderer_->SetSize(Vector2(HEIGHT, HEIGHT));

	// 背景画像
	backGroundMaterial_ = std::make_unique<PixelMaterial>(L"Texture.cso", 1);
	backGroundMaterial_->AddConstBuf({ 1.0f, 1.0f, 1.0f, 1.0f });
	backGroundMaterial_->AddTextureBuf(backImg_);
	backGroundRenderer_ = std::make_unique<PixelRenderer>(*backGroundMaterial_);
	backGroundRenderer_->MakeSquereVertex(
		Vector2(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2),
		Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y)
	);
	
	// タイトル画像
	titleMaterial_ = std::make_unique<PixelMaterial>(L"Texture.cso", 1);
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
	auto& ins = InputManager::GetInstance();
	Vector2 moPos = ins.GetMousePos();

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
	agoMouseTrg_ = ins.IsClickMouseLeft();
}

void TitleScene::Draw(void)
{
	auto& ins = InputManager::GetInstance();

	// 背景描画
	backGroundRenderer_->Draw();

	// タイトル画面
	if (isTitle_)
	{
		// タイトルロゴの描画
		titleRenderer_->Draw();

		int cx = Application::SCREEN_SIZE_X / 2;

		int len = (int)wcslen(L"PushSpace or B");
		int width = GetDrawStringWidth(L"PushSpace or B", len);
		DrawFormatString(cx - (width / 2), B2_S_POS.y + HEIGHT / 2, 0xffffff, L"PushSpace or B");

		return;
	}

	// プレイヤー描画
	viewPlayer_->Draw();

	// ホストorクライアント
	DrawBox(B1_S_POS.x, B1_S_POS.y, B1_E_POS.x, B1_E_POS.y, 0x000000, true);
	DrawBox(B1_S_POS.x, B1_S_POS.y, B1_E_POS.x, B1_E_POS.y, 0xffffff, false);
	if (GameManager::GetInstance().IsHost())
	{
		DrawString(B1_S_POS.x + 50, B1_S_POS.y + 7, L"HOST", 0xffffff);
	}
	else
	{
		DrawString(B1_S_POS.x + 50, B1_S_POS.y + 7, L"CLIENT", 0xffffff);
	}

	// 武器変更
	DrawBox(WP_S_POS.x, WP_S_POS.y, WP_E_POS.x, WP_E_POS.y, 0x000000, true);
	DrawBox(WP_S_POS.x, WP_S_POS.y, WP_E_POS.x, WP_E_POS.y, 0xffffff, false);
	DrawString(WP_S_POS.x + 50, WP_S_POS.y + 7, L"武器変更", 0xffffff);

	// 出撃
	DrawBox(B2_S_POS.x, B2_S_POS.y, B2_E_POS.x, B2_E_POS.y, 0x000000, true);
	DrawBox(B2_S_POS.x, B2_S_POS.y, B2_E_POS.x, B2_E_POS.y, 0xffffff, false);
	DrawString(B2_S_POS.x + 50, B2_S_POS.y + 7, L"出撃", 0xffffff);

	// IPアドレス
	inputTextArea_->Draw();

#ifdef _DEBUG

	Vector2 moPos = ins.GetMousePos();
	DrawFormatString(0, 32, 0x000000, L"ローカル座標(%d, %d)", moPos.x, moPos.y);
	DrawFormatString(0, 16, 0x000000, L"モードID(%d)", selectId_);
	DrawFormatString(0, 0, 0x000000, L"武器　ID(%d)", weponId_);

#endif // DEBUG

	// カーソル描画
	if (!isWpSelect_)
	{
		if (!isPad_) { return; }

		switch (selectId_)
		{
			// ホストorクライアント選択
		case (int)MENU::NET_SELECT:
			cursorRenderer_->Draw(B1_S_POS.x - HEIGHT, B1_C_POS.y);
			break;
			// ゲームスタート
		case (int)MENU::GAME_START:
			cursorRenderer_->Draw(B2_S_POS.x - HEIGHT, B2_C_POS.y);
			break;
			// 武器設定
		case (int)MENU::WEPON_SELECT:
			cursorRenderer_->Draw(WP_S_POS.x - HEIGHT, WP_C_POS.y);
			break;
			// IPアドレス設定
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
			DrawBox(wPos.second->StartPos.x, wPos.second->StartPos.y
				, wPos.second->EndPos.x, wPos.second->EndPos.y, 0x000000, true);
			DrawString(wPos.second->StartPos.x + 50, wPos.second->StartPos.y + 7
				, wPos.second->Name.c_str(), 0xffffff);
		}
		// カーソル描画
		if (isPad_)
		{
			cursorRenderer_->Draw(weponsPos_.at(weponId_)->StartPos.x - HEIGHT
				, weponsPos_.at(weponId_)->CenterPos.y);
		}
	}
}

void TitleScene::Release(void)
{
	inputTextArea_->Release();
	delete inputTextArea_;
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
		if (IsTrggerdMleft())
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
	auto& ins = InputManager::GetInstance();
	Vector2 moPos = ins.GetMousePos();

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
	auto& ins = InputManager::GetInstance();


	if (inputTextArea_->IsActive())
	{
		selectId_ = (int)MENU::IP_SET;
		padUpdate_ = std::bind(&TitleScene::PIpUpdate, this);
	}

	// クリックしたとき
	if (!inputTextArea_->IsActive() && IsTrggerdMleft())
	{
		Vector2 moPos = ins.GetMousePos();

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
					IPDATA hostIp{ d1, d2, d3, d4 };
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
	else if (inputTextArea_->IsActive() && IsTrggerdMleft())
	{
		SoundManager::GetInstance().Play(SoundManager::SRC::ENTER, Sound::TIMES::ONCE, true);

		if (!inputTextArea_->IsCleckText())
		{
			int handle = inputTextArea_->GetKeyHandle();

			// 現在の入力内容を強制的に文字列に反映
			inputTextArea_->SetKeyInputStringBuffer();

			// 入力を非アクティブにする（＝強制終了）
			SetActiveKeyInput(-1);  // DxLib関数：現在のキー入力を終了

			// 管理クラスに通知して状態を初期化
			InputTextManager::GetInstance().SetTextArea(false); // ←ここでnullptrにしてるはず
		}
	}

}
void TitleScene::MWeaponUpdate(void)
{
	auto& ins = InputManager::GetInstance();
	auto& sns = SceneManager::GetInstance();
	auto& gns = GameManager::GetInstance();

	// クリックしたとき
	if (IsTrggerdMleft())
	{
		Vector2 moPos = ins.GetMousePos();
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
			else
			{
				continue;
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
	auto& ins = InputManager::GetInstance();
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
					IPDATA hostIp{ d1, d2, d3, d4 };
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
	auto& ins = InputManager::GetInstance();
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
		SetActiveKeyInput(-1);  // DxLib関数：現在のキー入力を終了

		// 管理クラスに通知して状態を初期化
		InputTextManager::GetInstance().SetTextArea(false); // ←ここでnullptrにしてるはず

		padUpdate_ = std::bind(&TitleScene::PNormalUpdate, this);
	}
}

const bool TitleScene::IsTrggerdMleft(void) const
{
	auto& ins = InputManager::GetInstance();

	return ins.IsClickMouseLeft() && !agoMouseTrg_;
}

void TitleScene::AddPosTri(std::wstring name, int weponId
	, const Vector2 size, const Vector2 cPos)
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
