#include <iostream>
#include <string>
#include <thread>

#include <DxLib.h>
#include <EffekseerForDXLib.h>

#include "Manager/InputManager.h"
#include "Manager/InputTextManager.h"
#include "Manager/SceneManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SoundManager.h"

#include "Utility/Measure.h"
#include "./Net/NetManager.h"
#include "Application.h"


Application* Application::instance_ = nullptr;

const std::wstring Application::PATH_IMAGE = L"Data/Image/";
const std::wstring Application::PATH_MODEL = L"Data/Model/";
const std::wstring Application::PATH_EFFECT = L"Data/Effect/";
const std::wstring Application::PATH_SOUND = L"Data/Sound/";
const std::wstring Application::PATH_SHADER = L"Data/Shader/";
const std::wstring Application::PATH_JSON = L"Data/Json/";

const std::wstring Application::PATH_DATA		= L"Data/";
//const std::wstring Application::PATH_MAP_DATA	= PATH_DATA + L"Map.csv";
const std::wstring Application::PATH_MAP_DATA	= PATH_DATA + L"MapBig.csv";
const std::wstring Application::PATH_IMG_PLAYER1 = PATH_IMAGE + L"Player/P1/";
const std::wstring Application::PATH_IMG_PLAYER2 = PATH_IMAGE + L"Player/P2/";

Application::Application(void)
{
	isError_ = false;
}

Application::Application(const Application& ins)
{
	isError_ = false;
}

Application::~Application(void)
{
}

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	

	// アプリケーションの初期設定
	SetWindowText(L"ワイルドハント");

	// ウィンドウサイズ
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);
	ChangeWindowMode(true);

	// ２重起動検査回避用
	SRand(GetNowCount());

	//この2つすごい大事・・・・・・
	SetDoubleStartValidFlag(true);
	SetAlwaysRunFlag(true);

	int rand = GetRand(999);
	std::wstring name = L"UDP Test";

	name += std::to_wstring(rand);
	SetMainWindowClassName(name.c_str());

	if (DxLib_Init() == -1)
	{
		isError_ = true;
		return;
	}

	//----------------------------------
	InitEffekseer();  // ★ここを追加！
	//----------------------------------


	// 入力制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();
	InputTextManager::CreateInstance();

	// ネットワーク管理初期化////////////////////////////////////////
	NetManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	//音の初期化
	SoundManager::CreateInstance();

	// 計測
	Measure::CreateInstance();
	Measure::GetInstance().Start();

}

void Application::Run(void)
{

	// ゲームループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// ネットワーク管理更新(フレームの最初)///////////////////////////////////
		NetManager::GetInstance().Update();

		InputManager::GetInstance().Update();
		InputTextManager::GetInstance().Update();

		SceneManager::GetInstance().Update();
		SceneManager::GetInstance().Draw();





		Measure::GetInstance().Draw();

		// ネットワーク管理更新(フレームの最後)
		NetManager::GetInstance().UpdateEndOfFrame();

		ScreenFlip();
	}
}

void Application::Destroy(void)
{           
	// ネットワーク管理破棄
	NetManager::GetInstance().Destroy();

	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();//なんかエラー出る
	SoundManager::GetInstance().Destroy(); 
	SceneManager::GetInstance().Destroy();

	// Effekseerを終了する。
	Effkseer_End();

	if (DxLib_End() == -1)
	{
		isError_ = true;
		return;
	}

}

bool Application::IsError(void)
{
	return isError_;
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}
