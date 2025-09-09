#pragma once
#include <chrono>
#include <memory>
#include <DxLib.h>

#include "../Lib/nlohmann/json.hpp"

// 長いのでnamespaceの省略
using json = nlohmann::json;

class Fader;
class SceneBase;
class Camera;

class SceneManager
{

public:

	static constexpr float DEFAULT_FPS = 60.0f;

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		CONNECT,
		GAME,
		RSULT,
	};

	// ゲーム結果
	enum class GAME_RESULT
	{
		NONE,
		GAME_CLEAR,
		GAME_OVER,
		TIME_OVER,
		DRAW
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	void Init(void);
	void Init3D(void);
	void Update(void);
	void Draw(void);
	void Destroy(void);

	// シーン遷移命令
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;

	float GetTotalGameTime(void);
	void SetTotalGameTime(float time);
	void ForwardGameTime(void);

	// ゲーム結果の取得
	GAME_RESULT GetGameResult(void);

	// ゲーム結果の設定
	void SetGameResult(GAME_RESULT result);

	// カメラの取得
	std::weak_ptr<Camera> GetCamera(void) const;
	//スクリーンの取得
	const int& GetMainScreen(void);

	const int GetControllId(void)const { return ControllerId_; }
	const int GetWeponId(void)const { return weponId_; }
	const void SetWeponId(int weponId) { weponId_ = weponId; }

	//シーンを画像保存
	const void CaptureMainScreen(void);
	const void DrawCapturedScreen(int x, int y);

	/// <summary>
	/// シーンを新しく「積む」
	/// </summary>
	/// <param name="scene"></param>
	void PushScene(std::shared_ptr<SceneBase> scene);

	/// <summary>
	/// 最後に追加したシーンを削除する
	/// </summary>
	void PopScene();

	//接続モードについて
	const bool IsHost(void)const { return IsHost_; }
	void SetHost(bool value);


private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;
	
	//描画スクリーン
	int mainScreen_;
	int capturedScreenGraph_ = -1; // 複製した画像のハンドル

	// フェード
	std::unique_ptr<Fader> fader_;

	// 各種シーン
	std::list<std::shared_ptr<SceneBase>> scene_;

	// カメラ
	std::shared_ptr<Camera> camera_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	// ゲームの総時間
	float totalGameTime_;

	// ゲーム結果
	GAME_RESULT gameResult_;

	//プレイヤー設定
	int charId_;
	int weponId_;

	//接続モード
	bool IsHost_;

	int ControllerId_;

	// コンストラクタ
	SceneManager(void);

	// デストラクタ
	~SceneManager(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード処理
	void Fade(void);

	void Load(void);
	void Save(bool isUse);

};