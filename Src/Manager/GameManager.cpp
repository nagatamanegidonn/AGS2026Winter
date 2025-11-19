#include <string>
#include <fstream>

#include "GameManager.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "../Application.h"


GameManager* GameManager::instance_ = nullptr;

namespace {
	constexpr float CLEAR_TIME_DEFAULT = 3.0f; // クリア演出のデフォルト時間
	constexpr GameManager::ClearParam FIRST_PRAM = { 0, 1.0f, 0.98f ,20.0f };
	constexpr GameManager::ClearParam SECOND_PRAM = { 1, FIRST_PRAM.eValue, 1.05f,7.5f };
}

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
	clearCount_(0),
	clearMaxCount_(0),
	clearTime_(0.0f),

	paramRate_(0.0f),
	clearPramList_(),
	currentParam_({ 0,1.0f,1.0f,1.0f }),
	clearImg_(-1),

	charId_(0),
	weponId_(0),
	IsHost_(true),
	ControllerId_(DX_INPUT_PAD1)
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

	//　クリアロゴの読み込み
	clearImg_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::CLEAR_LOGO).handleId_;

	clearPramList_.push_back(FIRST_PRAM);
	clearPramList_.push_back(SECOND_PRAM);

	currentParam_ = clearPramList_.front();

	//接続はHost
	IsHost_ = true;
}
void GameManager::Update(void)
{
}
void GameManager::Draw(void)
{
}
//　clearTime_が一定の数値以下になったときに呼ばれる
void GameManager::ClearDraw(void)
{
	if(clearTime_ <= 2.5f&&IsClear())
	{
		// クリア演出
		float ExRate = std::lerp(currentParam_.sValue, currentParam_.eValue, paramRate_);
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2,
			ExRate, 0.0f, clearImg_, true);

		// パラメーターの割合更新
		paramRate_ += currentParam_.speed * SceneManager::GetInstance().GetDeltaTime();

		// 次のパラメーターへ
		if(paramRate_ >= 1.0f)
		{
			paramRate_ = 0.0f;
			for(const auto& param : clearPramList_)
			{
				// 現在のパラメーターと最後のパラメーターが同じなら終了
				if(clearPramList_.back().type == currentParam_.type)
				{
					// パラメーターの割合
					paramRate_ = 1.0f;
					break;
				}
				// 次のパラメーターへ
				else if (param.type == currentParam_.type + 1)
				{
					currentParam_ = param;
					break;
				}
			}
		}
	}
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