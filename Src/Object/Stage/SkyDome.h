#pragma once
#include <map>
#include "../Common/Transform.h"
#include "../ActorBase.h"

class SkyDome : 
	public ActorBase
{

public:

	static constexpr float SCALE = 150.0f;
	static constexpr VECTOR SCALES = { SCALE, SCALE, SCALE };

	// 状態
	enum class STATE
	{
		NONE,
		STAY,
		FOLLOW
	};

	// コンストラクタ
	SkyDome(const Transform& syncTransform);

	// デストラクタ
	~SkyDome(void);

	// 初期化処理
	void Init(void) override;
	
	// 更新処理
	void Update(void) override;
	
	// 描画処理
	void Draw(void) override;

private:

	// 自機の情報
	const Transform& syncTransform_;

	// 状態
	STATE state_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStateStay(void);
	void ChangeStateFollow(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdateStay(void);
	void UpdateFollow(void);

};
