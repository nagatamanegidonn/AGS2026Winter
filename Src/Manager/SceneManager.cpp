#include <EffekseerForDXLib.h>
#include <DxLib.h>

#include <string>
#include <fstream>
#include "../Lib/nlohmann/json.hpp"

#include "../Application.h"
#include "../Common/Fader.h"

#include "../Scene/SceneBase.h"
#include "../Scene/TitleScene.h"
#include "../Scene/ConnectScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/RefreshScene.h"

#include "../Manager/Camera.h"

#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

SceneManager::SceneManager(void)
{

	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	charId_ = 0;
	weponId_ = 0;

	ControllerId_ = DX_INPUT_PAD1;


	isSceneChanging_ = false;

	camera_ = nullptr;

	IsHost_ = true;
}

SceneManager::~SceneManager(void)
{
	delete instance_;
}

void SceneManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

void SceneManager::Init(void)
{

	sceneId_ = SCENE_ID::TITLE;
	waitSceneId_ = SCENE_ID::NONE;

	fader_ = std::make_unique<Fader>(0x000000);
	fader_->Init();

	// カメラ
	camera_ = std::make_shared<Camera>();
	camera_->Init();

	isSceneChanging_ = false;

	totalGameTime_ = 0.0f;

	Load();
	Save(true);

	// 3D用の設定
	Init3D();

	//接続はHost
	IsHost_ = true;

	// 初期シーンの設定
	DoChangeScene(SCENE_ID::TITLE);

}
void SceneManager::Init3D(void)
{

	// 背景色設定
	SetBackgroundColor(0, 139, 139);

	// Zバッファを有効にする
	SetUseZBuffer3D(true);

	// Zバッファへの書き込みを有効にする
	SetWriteZBuffer3D(true);

	// バックカリングを有効にする
	SetUseBackCulling(true);

	// ライトの設定
	SetUseLighting(true);

	// ライトの設定
	ChangeLightTypeDir({ 0.3f, -0.7f, 0.8f });
	//ChangeLightTypePoint({ 0.0f, -100.0f, 0.0f },20000.0f, 0.0f, 0.0006f, 0.0f);


	// フォグ設定
	SetFogEnable(true);
	SetFogColor(5, 5, 5);
	SetFogStartEnd(10000.0f, 20000.0f);

	mainScreen_ = MakeScreen(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y);
}

void SceneManager::Update(void)
{

	// デルタタイム
	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	// フェード更新
	fader_->Update();
	if (isSceneChanging_)
	{
		Fade();
	}
	else
	{
		// 更新
		for (auto& scene : scene_) {
			scene->Update();
		}
	}

	// カメラ更新
	camera_->Update();
}

void SceneManager::Draw(void)
{

	// 描画先グラフィック領域の指定
	// (３Ｄ描画で使用するカメラの設定などがリセットされる)
	SetDrawScreen(mainScreen_);

	// 画面を初期化
	ClearDrawScreen();

	// カメラ設定
	camera_->SetBeforeDraw();

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	// 描画
	for (auto& scene : scene_) {
		scene->Draw();
	}

	// Effekseerにより再生中のエフェクトを描画する。
	DrawEffekseer3D();

	// 最後
	fader_->Draw();

	SetDrawScreen(DX_SCREEN_BACK);
	DrawGraph(0, 0, mainScreen_, true);

}

void SceneManager::Destroy(void)
{
	Save(false);

	DeleteGraph(mainScreen_);

	if (capturedScreenGraph_ != -1)
	{
		DeleteGraph(capturedScreenGraph_);
		capturedScreenGraph_ = -1;
	}
}

void SceneManager::ChangeScene(SCENE_ID nextId)
{

	// フェード処理が終わってからシーンを変える場合もあるため、
	// 遷移先シーンをメンバ変数に保持
	waitSceneId_ = nextId;

	// フェードアウト(暗転)を開始する
	fader_->SetFade(Fader::NET_STATE::FADE_OUT);
	isSceneChanging_ = true;

}

SceneManager::SCENE_ID SceneManager::GetSceneID(void)
{
	return sceneId_;
}

float SceneManager::GetDeltaTime(void) const
{
	//return 1.0f / DEFAULT_FPS;
	return deltaTime_;
}

float SceneManager::GetTotalGameTime(void)
{
	return totalGameTime_;
}

void SceneManager::SetTotalGameTime(float time)
{
	totalGameTime_ = time;
}

void SceneManager::ForwardGameTime(void)
{
	totalGameTime_ += GetDeltaTime();
}

SceneManager::GAME_RESULT SceneManager::GetGameResult(void)
{
	return gameResult_;
}

void SceneManager::SetGameResult(GAME_RESULT result)
{
	gameResult_ = result;
}

std::weak_ptr<Camera> SceneManager::GetCamera(void) const
{
	return camera_;
}

const int& SceneManager::GetMainScreen(void)
{
	return mainScreen_;
}

const void SceneManager::CaptureMainScreen(void)
{
	// 古い画像があれば削除
	if (capturedScreenGraph_ != -1)
	{
		DeleteGraph(capturedScreenGraph_);
		capturedScreenGraph_ = -1;
	}

	// mainScreen_ の内容を新しいグラフィックとして保存
	capturedScreenGraph_ = MakeScreen(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, TRUE);

	// mainScreen_ の内容をコピー
	GetDrawScreenGraph(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, capturedScreenGraph_, FALSE);
}
const void SceneManager::DrawCapturedScreen(int x, int y)
{
	if (capturedScreenGraph_ != -1)
	{
		DrawGraph(x, y, capturedScreenGraph_, TRUE);
	}
}

void SceneManager::PushScene(std::shared_ptr<SceneBase> scene)
{
	scene_.push_back(scene);
}
void SceneManager::PopScene()
{
	if (scene_.size() > 1) {
		scene_.pop_back();
	}
}

void SceneManager::SetHost(bool value)
{
	//タイトル以外は変更不可
	if (sceneId_ != SCENE_ID::TITLE) {
		return;
	}

	IsHost_ = value;
}

void SceneManager::DoChangeScene(SCENE_ID sceneId)
{


	// リソースの解放
	//ResourceManager::GetInstance().Release();

	// シーンを変更する
	sceneId_ = sceneId;

	// 現在のシーンを解放
	if (!scene_.empty())
	{
		scene_.clear();
	}

	switch (sceneId_)
	{
	case SCENE_ID::TITLE:
		scene_.push_back(std::make_shared<TitleScene>());
		//scene_ = std::make_unique<TitleScene>();
		break;
	case SCENE_ID::CONNECT:
		scene_.push_back(std::make_shared<ConnectScene>());
		//scene_ = std::make_unique<ConnectScene>();
		break;
	case SCENE_ID::GAME:
		scene_.push_back(std::make_shared<GameScene>());
		//scene_ = std::make_unique<GameScene>();
		break;
	case SCENE_ID::RSULT:
		scene_.push_back(std::make_shared<RefreshScene>());
		//scene_ = std::make_unique<RefreshScene>();
		break;
	}

	// 挿入したシーンだけ Init
	if (!scene_.empty())
	{
		scene_.back()->Init();   // ← これでOK
	}

	//ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;


	// フォントサイズを元に戻す
	SetFontSize(16);

}

void SceneManager::Fade(void)
{

	Fader::NET_STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::NET_STATE::FADE_OUT:
		if (fader_->IsEnd())
		{
			// 暗転後、シーン変更
			DoChangeScene(waitSceneId_);
			fader_->SetFade(Fader::NET_STATE::FADE_IN);
		}
		break;
	case Fader::NET_STATE::FADE_IN:
		if (fader_->IsEnd())
		{
			// 明転後、シーン遷移終了
			fader_->SetFade(Fader::NET_STATE::NONE);
			isSceneChanging_ = false;
		}
		break;
	}

}

void SceneManager::Load(void)
{
	std::ifstream ifs(Application::PATH_JSON + L"ControllerListDate.json");
	json ControllerData;
	if (ifs)
	{
		ifs >> ControllerData;
	}
	else { return; }

	// 連想配列の値を順にチェックする
	for (const auto& [key, valueArray] : ControllerData.items()) {
		for (const auto& controller : valueArray) {
			if (!controller["isUse"]) {
				ControllerId_ = controller["ID"];
				return;
			}
		}
	}
}
void SceneManager::Save(bool isUse)
{
	// JSON読み込み（既存ファイル）
	std::ifstream ifs(Application::PATH_JSON + L"ControllerListDate.json");
	if (!ifs.is_open()) {
		printfDx(L"Save失敗: ファイルが開けませんでした\n");
		return;
	}

	json LoadData;
	try {
		ifs >> LoadData;
	}
	catch (const json::parse_error& e) {
		printfDx(L"Save失敗: JSON構文エラー (%s)\n", e.what());
		return;
	}
	ifs.close();

	// 保存データ作成
	json saveData;

	for (const auto& [key, valueArray] : LoadData.items()) {
		// 配列でない or 空ならスキップ
		if (!valueArray.is_array() || valueArray.empty()) continue;

		const auto& Data = valueArray[0];

		json ControllerData;

		if (Data.contains("ID") && Data["ID"].get<int>() == ControllerId_) {
			ControllerData["ID"] = ControllerId_;
			ControllerData["isUse"] = isUse;
		}
		else {
			ControllerData["ID"] = Data.contains("ID") ? Data["ID"].get<int>() : -1;
			ControllerData["isUse"] = Data.contains("isUse") ? Data["isUse"].get<bool>() : false;
		}

		saveData[key].push_back(ControllerData);
	}

	// 書き込み
	std::ofstream ofs(Application::PATH_JSON + L"ControllerListDate.json");
	if (!ofs.is_open()) {
		printfDx(L"Save失敗: 書き込み用ファイルを開けませんでした\n");
		return;
	}

	ofs << saveData.dump(4);
	ofs.close();
}
