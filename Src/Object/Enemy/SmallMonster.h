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
	// スケールサイズ
	static constexpr float SCALE_SIZE = 2.0f;
	// スピード
	static constexpr float SPEED_MOVE = 4.5f;
	static constexpr float SPEED_FOLLOW = 12.0f;
	static constexpr float SPEED_RUN = 15.0f;
	// 判定用半径
	static constexpr float MOVE_RADIUS = 2250.0f;
	static constexpr float DASH_RADIUS = 1500.0f;
	static constexpr float ATTRCK_RADIUS = 750.0f;
	static constexpr float ATTRCK_BITE_RADIUS = 150.0f;
	static constexpr float ATTRCK_STAMP_RADIUS = 300.0f;
	// 視野関係
	static constexpr float FOV_RADIUS = 30.0f;	// 視野角
	// 最大ＨＰ
	static constexpr int MAX_HP = 75;
	// 攻撃関係
	static constexpr int ATTRCK_STAMP = 34;

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,			// 通常
		BATTLE,			// 戦闘
		FOLLOW,			// 近づく
		ATTRCK_READY,	// 予備動作
		ATTRCK,			// 攻撃
		DAMAGE,			// ダメージ
		DEAD,			// 死亡
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,			// 棒立ち
		RUN,			// 移動
		ATTRCK_READY,	// 予備動作
		ATTRCK,			// 攻撃
		DAMAGE,			// ダメージ
		DEAD,			// 死亡
	};

	// コンストラクタ
	SmallMonster(int key,int createNo);

	// デストラクタ
	~SmallMonster(void);

	// 初期化処理
	virtual void Init(void) override;

	// 更新処理
	virtual void Update(void) override;

	// 描画処理
	virtual void Draw(void) override;

	// 影の描画
	virtual void DrawShadow(void) override;

	// ダメージ関係
	void Damage(int dama, bool isConst = false) override;

	// Stateの判定
	const bool IsState(STATE state)const { return (state_ == state); }

	// 攻撃位置の取得
	const VECTOR GetAttrckPos(void) const { return attrckPos_; }

	// 外部クラストの当たり判定
	const bool CollisionCapsule(std::weak_ptr<Capsule> capsule);
	const bool CollisionAttrck(const int& modelId);

	// 追従対象の設定
	void SetFollow(const Transform* follow) override;

private:

	// 状態管理
	STATE state_;		// 状態
	int animeType_;		// アニメーションタイプ
	int animeAgoType_;	// 前フレームのアニメーション
	float stateTime_;	// 状態時間
	
	//攻撃管理
	float dameRate_;		// 受けるダメージ倍率
	int attrckCount_;		// 連続攻撃の際に使用
	VECTOR attrckPos_;		// 攻撃判定中心位置
	float attrckRadius;		// 攻撃半径

	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;


	// 複数の初期化処理
	void InitAnimation(void);
	void InitEffect(void);
	void InitSound(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateBattle(void);
	void ChangeStateFollow(void);
	void ChangeStateAttrckReady(void);
	void ChangeStateAttrckStamp(void);
	void ChangeStateDamage(void);
	void ChangeStateDead(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateBattle(void);
	void UpdateFollow(void);
	void UpdateAttrckReady(void);
	void UpdateAttrckStamp(void);
	void UpdateDamage(void);
	void UpdateDead(void);

	// 衝突チェック(カプセル)
	void CollisionStageCapsule(void) override;
	void CollisionGravity(void) override;

	// 攻撃無効化
	void AttrckUpdate(void);

	// デバッグ用描画
	void DrawDebug(void);
};

