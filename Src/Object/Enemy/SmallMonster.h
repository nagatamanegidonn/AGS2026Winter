#pragma once
#include <functional>
#include "EnemyBase.h"

class SmallMonster :
    public EnemyBase
{
public:

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;
	static constexpr float TIME_ROT2 = 10.0f;

	static constexpr float SCALE_SIZE = 2.0f;//2.0f;

	// スピード
	static constexpr float SPEED_MOVE = 4.5f;/*3.0f;*/
	static constexpr float SPEED_FOLLOW = 12.0f;/*8.0f;*/
	static constexpr float SPEED_RUN = 15.0f;/*10.0f;*/

	//判定用半径
	static constexpr float MOVE_RADIUS = 2250.0f;
	static constexpr float DASH_RADIUS = 1500.0f;
	static constexpr float ATTRCK_RADIUS = 750.0f;
	static constexpr float ATTRCK_BITE_RADIUS = 150.0f;
	static constexpr float ATTRCK_STAMP_RADIUS = 300.0f;

	static constexpr float FOV_RADIUS = 30.0f;//視野角

	static constexpr float MAX_LERP_TIME = 20.0f;//在中時間


	// 最大ＨＰ
	static constexpr int MAX_HP = 75;

	//攻撃関係
	static constexpr int ATTRCK_CLOW = 28;
	//static constexpr int ATTRCK_CLOW = 8;
	static constexpr int ATTRCK_STAMP = 34;
	static constexpr int ATTRCK_DASH = 25;

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		BATTLE,			//後で消す
		FOLLOW,			//近づく

		ATTRCK_READY,	//予備動作
		ATTRCK,	//攻撃
	
		DAMAGE,
		DEAD,
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,

		ATTRCK_READY,		//予備動作
		ATTRCK,

		DAMAGE,
		DEAD,
	};

	// コンストラクタ
	SmallMonster(int key,int createNo);

	// デストラクタ
	~SmallMonster(void);

	void Init(void) override;
	void Update(void)override;
	void Draw(void)override;

	//ダメージ関係
	void Damage(int dama);

	const bool IsState(STATE state)const { return (state_ == state); }

	const VECTOR GetAttrckPos(void) const { return attrckPos_; }

	//外部クラストの当たり判定
	const bool CollisionCapsule(std::weak_ptr<Capsule> _capsule);
	const bool CollisionAttrck(const int& modelId);

	//追従対象の設定
	void SetFollow(const Transform* follow) override;


private:

	// 状態管理
	STATE state_;
	int animeType_;
	int animeAgoType_;

	float stateTime_;	//状態時間

	
	//攻撃管理
	//ATTRCK_TYPE attrckTypeState_;//攻撃種別

	float dameRate_;		//受けるダメージ倍率
	//ボス限定
	int attrckCount_;		//連続攻撃の際に使用
	VECTOR attrckPos_;		//攻撃判定中心位置
	float attrckRadius = 0.0f;

	

	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;


	//複数の初期化処理
	void InitAnimation(void);
	void InitEffect(void);
	void InitSound(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateBattle(void);
	void ChangeStateFollow(void);
	//攻撃系
	void ChangeStateAttrckReady(void);
	void ChangeStateAttrckStamp(void);

	void ChangeStateDamage(void);
	void ChangeStateDead(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateBattle(void);
	void UpdateFollow(void);
	//攻撃系
	void UpdateAttrckReady(void);
	void UpdateAttrckStamp(void);

	void UpdateDamage(void);
	void UpdateDead(void);


	//攻撃無効化
	void AttrckUpdate(void);

	// デバッグ用描画
	void DrawDebug(void);
};

