#pragma once
#include <string>
#include <map>
#include <list>
class SceneManager;

class AnimationController
{
	
public :

	// アニメーションデータ
	struct Animation
	{
		int model = -1;
		int attachNo = -1;
		int animIndex = 0;
		float speed = 0.0f;
		float totalTime = 0.0f;
		float step = 0.0f;

		// アニメーション終了後に繰り返すループステップ
		float startStep = 0.0f;
		float endStep = -1.0f;

		// 逆再生
		float switchLoopReverse_ = 1.0f;
		//ブレンドするかどうか
		bool isBlend = false;
		float blendSpeed = 1.0f;

	};

	//ブレンド情報
	struct BlendData {
		int fromAttachNo = -1;
		int animType = -1;
		float blendRate = 0.0f;
	};
	struct BlendState {
		std::list<BlendData> data;
		//int fromAttachNo = -1;
		int toAttachNo = -1;
		//float blendRate = 0.0f;
		bool isBlending = false;
		float blendSpeed = 1.0f; // 1秒で100%に
	};

	// コンストラクタ
	AnimationController(int modelId);
	// デストラクタ
	~AnimationController(void);

	// アニメーション追加
	//sTimeとeTimeを入れるときに再生方向を決めれる
	void Add(int type, const std::wstring& path, float speed, int animNo = 0
		, float sTime = 0.0f, float eTime = -1.0f);
	void SetIsBlend(int type, bool isBlend, float blendSpeed = 3.0f);

	// アニメーション再生
	void Play(int type, bool isLoop = true, 
		bool isStop = false, bool isForce = false);

	void Update(void);

	// アニメーション終了後に繰り返すループステップ
	void SetEndLoop(float startStep, float endStep, float speed);

	// 再生中のアニメーション
	int GetPlayType(void) const;

	// 再生終了
	bool IsEnd(void) const;

	const float GetStepTime(void) const { return playAnim_.step; }
	//アニメーションのモデルローカル位置
	const VECTOR GetPos(void) const { return localPos_; }
	const bool IsBlend(void) const { return blend_.isBlending; }

private :

	std::map<std::wstring, int> animModelCache_;

	// モデルのハンドルID
	int modelId_;

	// 種類別のアニメーションデータ
	std::map<int, Animation> animations_;

	int playType_;
	Animation playAnim_;


	BlendState blend_;
	float blendSpeed = 1.0f;
	int nextAnimType_ = -1; // ブレンド完了後に移行するアニメ（未使用なら -1）


	// アニメーションをループするかしないか
	bool isLoop_;

	// アニメーションを止めたままにする
	bool isStop_;

	// アニメーション終了後に繰り返すループステップ
	float stepEndLoopStart_;
	float stepEndLoopEnd_;
	float endLoopSpeed_;

	// 逆再生
	float switchLoopReverse_;

	VECTOR localPos_;

	int GetAttrchNo(int animType);
};

