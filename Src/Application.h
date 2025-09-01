#pragma once
#include <string>
#include <vector>

class Application
{

public:

	// スクリーンサイズ
	//static constexpr int SCREEN_SIZE_X = 768;
	static constexpr int SCREEN_SIZE_X = 1050;
	//static constexpr int SCREEN_SIZE_Y = 480;
	static constexpr int SCREEN_SIZE_Y = 640;

	// データパス関連
	//-------------------------------------------
	static const std::wstring PATH_IMAGE;
	static const std::wstring PATH_MODEL;
	static const std::wstring PATH_SOUND;
	static const std::wstring PATH_EFFECT;
	static const std::wstring PATH_SHADER;
	static const std::wstring PATH_JSON;

	static const std::wstring PATH_DATA;
	static const std::wstring PATH_MAP_DATA;
	static const std::wstring PATH_IMG_PLAYER1;
	static const std::wstring PATH_IMG_PLAYER2;
	//-------------------------------------------
	
	// ポート番号
	const int PORT = 12345;

	// 受信データを格納する共有リソース
	std::vector<char> recvData_;

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static Application& GetInstance(void);

	// 初期化
	void Init(void);

	// ゲームループの開始
	void Run(void);

	// リソースの解放
	void Destroy(void);

	// エラー判定
	bool IsError(void);

private:

	// 静的インスタンス
	static Application* instance_;

	// コンストラクタ
	Application(void);

	// コピーコンストラクタ
	Application(const Application& ins);

	// デストラクタ
	~Application(void);

	// エラー判定
	bool isError_;

	// Effekseerの初期化
	void InitEffekseer(void);
};