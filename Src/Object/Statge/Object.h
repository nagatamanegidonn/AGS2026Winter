#pragma once
#include <memory>
#include <map>
#include <functional>
#include "../Common/Quaternion.h"
#include "../ActorBase.h"
#include "Stage.h"

class Player;
class ModelMaterial;
class ModelRenderer;

class Object :
    public ActorBase
{

public:
	
	// 回転速度
	static constexpr float SPEED_ROT_IDLE = 3.0f;
	static constexpr float SPEED_ROT_RESERVE = 15.0f;

	// エフェクト生成間隔
	static constexpr float TERM_EFFECT = 0.08f;

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		ROLL,
	};

	// コンストラクタ
	Object(
		Player& player, const Transform& transform, STATE state);

	// デストラクタ
	~Object(void);

	virtual void Init(void) override;
	virtual void Update(void) override;
	virtual void Draw(void) override;

protected:

	
	// プレイヤー
	Player& player_;

	// 状態管理
	STATE state_;
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateRoll(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateRoll(void);

};

