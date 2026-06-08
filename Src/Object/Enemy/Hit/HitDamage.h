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

	// コンストラクタ
	HitDamage(int& model, std::wstring boneName, int damage);

	// デストラクタ
	~HitDamage(void);

	// 初期化処理
	void Init(int damage);

	// 更新処理
	void Update(void);

	// 描画処理
	void Draw(void) const;

	// 状態取得のゲッター
	const STATE GetState(void) const { return state_; }

private:

	// モデルの参照
	int& parModel_;

	// 参照ボーンネーム
	std::wstring boneName_;

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

