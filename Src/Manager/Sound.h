#pragma once
#include<string>
#include<DxLib.h>


class Sound
{
public:

	// サウンドタイプ
	enum class TYPE
	{
		NONE,
		SOUND_2D,	//2D音響
		SOUND_3D,	//3D音響
	};

	enum class TIMES
	{
		ONCE,	//1回
		LOOP,	//繰り返し
	};

	// コンストラクタ
	Sound(void);
	// コンストラクタ
	Sound(TYPE type, const std::wstring& path);

	// デストラクタ
	~Sound(void);

	//3D 座標更新処理
	void Update(VECTOR pos);

	// 読み込み
	void Load(void);

	// 解放
	void Release(void);


	TYPE soundType_;


	// リソースの読み込み先
	std::wstring path_;


	// 音のハンドルID
	int handleId_;


	//音の最大音量
	int maxVolume_;
	
	//3D用
	VECTOR pos_;	//座標
	float radius_;	//有効範囲

	//一時停止用（新規）
	int pauseTime_; // 停止時の再生位置（ミリ秒）
	bool isPaused_;

	bool Play(TIMES times, bool isForce = false);						//2D用音源再生
	bool Play(VECTOR pos, float radius, TIMES times);	//3D用音源再生

	void Stop(void);	//再生中の音源を停止する

	//新規
	void Pause(TIMES times);
	void Resume(TIMES times);



	bool CheckMove(void);	//再生中かを確認

	bool CheckLoad(void);	//ロードしているかを確認

	void ChengeVolume(float per);	//音の大きさを変える

	void ChengeMaxVolume(float per);
};