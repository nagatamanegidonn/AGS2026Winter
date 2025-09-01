#pragma once
#include<map>
#include"Sound.h"

class SoundManager
{
public:
	//リソース名
	enum class SRC
	{
		//TITLE_BGM,		//タイトルシーンのBGM
		//SELECT_BGM,		//セレクトシーンのBGM
		//
		//RESULT_BGM,		//リザルトシーンのBGM

		BATTLE_BGM,
		
		SLASH_DAMAGE,
		SHOT_DAMAGE,
		CHAGE,
		
		//コネクトシーンで使用
		CONECT_START,
		ADD,

		//ゲームシーンで使用
		LOCKON,

		//SE
		SELECT,
		ENTER,
		START,
		GAME_START,
		VOICE,

		MOVE,
		REMOVE,
	};

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static SoundManager& GetInstance(void);

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
	/// <param name="isForce">終了前に流すか？</param>
	/// <returns></returns>
	bool Play(SRC src, Sound::TIMES times, bool isForce = false);	//二次元音源用
	bool Play(SRC src, Sound::TIMES times, VECTOR pos, float radius);	//三次元音源用

	void Stop(SRC src);	//音源を停止する
	void AllStop(void);	//音源を停止する

	bool Pause(SRC src, Sound::TIMES times);
	bool Resume(SRC src, Sound::TIMES times);


	bool CheckMove(SRC src);

	void ChengeVolume(SRC src, float per);	//per 0.0～1.0でパーセントを設定する
private:
	// 静的インスタンス
	static SoundManager* instance_;

	std::map<SRC, Sound> soundMap_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SoundManager(void) = default;

	// デストラクタも同様
	~SoundManager(void) = default;
};
