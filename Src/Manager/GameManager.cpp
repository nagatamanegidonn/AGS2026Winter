#include <string>
#include <fstream>

#include "GameManager.h"
#include "SceneManager.h"
#include "../Application.h"


GameManager* GameManager::instance_ = nullptr;

void GameManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new GameManager();
	}
	instance_->Init();
}
GameManager& GameManager::GetInstance(void)
{
	return *instance_;
}

GameManager::GameManager(void)
	:
	gameResult_(GAME_RESULT::NONE),
	clearFlag_(false),
	charId_(0),
	weponId_(0),
	IsHost_(true),
	ControllerId_(DX_INPUT_PAD1),
	clearCount_(0),
	clearMaxCount_(0)
{
}
GameManager::~GameManager(void)
{
	delete instance_;
}

void GameManager::Init(void)
{
	// 初期化
	clearFlag_ = false;//クリアフラグ

	charId_ = 0;
	weponId_ = 0;
	ControllerId_ = DX_INPUT_PAD1;

	Load();
	Save(true);

	//接続はHost
	IsHost_ = true;
}
void GameManager::Update(void)
{
}
void GameManager::Draw(void)
{
}
void GameManager::Destroy(void)
{
	Save(false);
}

GameManager::GAME_RESULT GameManager::GetGameResult(void)
{
	return gameResult_;
}

void GameManager::SetGameResult(GAME_RESULT result)
{
	gameResult_ = result;
}
void GameManager::SetHost(bool value)
{
	// タイトル以外は変更不可
	if (SceneManager::GetInstance().GetSceneID()
		!= SceneManager::SCENE_ID::TITLE) {
		return;
	}

	IsHost_ = value;
}
	

#pragma region 使用コントローラID保存・読み込み

void GameManager::Load(void)
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
void GameManager::Save(bool isUse)
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

#pragma endregion