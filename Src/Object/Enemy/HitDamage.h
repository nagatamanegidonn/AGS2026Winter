#pragma once
#include <DxLib.h>
#include <string>


class HitDamage
{

public:

	// ランダム位置補正の範囲
	static constexpr int RAND_RATE = 20;

	enum class STATE
	{
		NONE,
		PLAY,
		END,
	};

	HitDamage(int& model, std::string boneName, int damage);
	~HitDamage(void);

	void Init(int damage);
	void Update(void);
	void Draw(void);

	// 状態取得のゲッター
	const STATE GetState(void) const { return state_; }

private:

	int& parModel_;
	std::string boneName_;

	// 状態変数
	STATE state_;

	// ダメージ表記用変数
	float uiRate_ = 0.0f;
	int uiDame_ = 0;
	VECTOR uiPos_;

	// ランダム位置補正用変数
	int randPosX_;
	int randPosY_;
};

