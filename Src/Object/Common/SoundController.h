#pragma once
#include<memory>
#include<map>
#include"../Manager/Sound.h"

class SoundController
{
public:
	SoundController(void);
	// デストラクタ
	~SoundController(void);

	void Add(int type, std::wstring str, float vol = 1.0f);
	// 初期化
	void Init(void);
	// 解放(シーン切替時に一旦解放)
	void Release(void);
	// リソースの完全破棄
	void Destroy(void);

	/// <summary>
	/// 音の再生
	/// </summary>
	/// <param name="src">出す音</param>
	/// <param name="times">音の出す時間</param>
	/// <param name="isForce"></param>
	/// <returns></returns>
	bool Play(int src, Sound::TIMES times, bool isForce = false);		//二次元音源用
	bool Play(int src, Sound::TIMES times, VECTOR pos, float radius);	//三次元音源用

	void Stop(int src);	// 音源を停止する
	void AllStop(void);	// 音源を停止する

	bool CheckMove(int src);

	void ChengeVolume(int src, float per);	// per 0.0～1.0でパーセントを設定する
private:
	
	std::map<int, std::unique_ptr<Sound>> soundMap_;

};

