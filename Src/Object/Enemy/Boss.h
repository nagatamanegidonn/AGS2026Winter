
#pragma once
#include <functional>

#include "../../Net/NetStructures.h"
//#include "../Common/Vector2F.h"
//#include "../Utility/AsoUtility.h"

#include "EnemyBase.h"

class AnimationController;
class EffectController;
class SoundController;

class Capsule;

class HitDamage;

class SceneBase;
class GameScene;

class Boss : public EnemyBase
{

public:

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;
	static constexpr float TIME_ROT2 = 10.0f;

	static constexpr float SCALE_SIZE = 3.0f;//2.0f;

	// スピード
	static constexpr float SPEED_MOVE = 4.5f;/*3.0f;*/
	static constexpr float SPEED_FOLLOW= 12.0f;/*8.0f;*/
	static constexpr float SPEED_RUN = 15.0f;/*10.0f;*/



	// 加速(accelerator=アクセレレーター)
	static constexpr float MOVE_ACC = 0.25f;
	// 減速(decelerate=ディセラレイト)
	static constexpr float MOVE_DEC = 0.05f;

	//判定用半径
	static constexpr float MOVE_RADIUS = 2250.0f;
	static constexpr float DASH_RADIUS = 1500.0f;
	static constexpr float ATTRCK_RADIUS = 750.0f;
	static constexpr float ATTRCK_BITE_RADIUS = 150.0f;
	static constexpr float ATTRCK_STAMP_RADIUS = 300.0f;
	static constexpr float ATTRCK_DASH_RADIUS = 450.0f;

	//static constexpr float MOVE_RADIUS = 1500.0f;
	//static constexpr float DASH_RADIUS = 1000.0f;
	//static constexpr float ATTRCK_RADIUS = 500.0f;
	//static constexpr float ATTRCK_BITE_RADIUS = 100.0f;
	//static constexpr float ATTRCK_DASH_RADIUS = 300.0f;

	static constexpr float FOV_RADIUS = 30.0f;//視野角
	static constexpr float FOV_RADIUS_FLASH = 180.0f;//視野角

	static constexpr float MAX_LERP_TIME = 20.0f;//在中時間


	// 最大ＨＰ
	//static constexpr int MAX_HP = 200;
	static constexpr int MAX_HP = 20;

	//攻撃関係
	static constexpr int ATTRCK_CLOW = 28;
	//static constexpr int ATTRCK_CLOW = 8;
	static constexpr int ATTRCK_STAMP= 34;
	static constexpr int ATTRCK_DASH = 25;



	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		LERP_MOVE,			//エリア移動
		BATTLE,			//後で消す
		FOLLOW,			//近づく

		ATTRCK_READY,	//予備動作
		ATTRCK_STAMP,	//スタンプ	攻撃
		ATTRCK_L_CLOW,	//爪攻撃
		ATTRCK_R_CLOW,	//爪攻撃
		ATTRCK_DASH,	//攻撃

		HOWLING,		//咆哮
		STUNNED,		//スタン
		DAMAGE,

		DEAD,
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,

		BTLLE_IDLE,
		BTLLE_RUN,

		READY_ATTRCK,		//予備動作
		//ATTRCK_BITE,
		ATTRCK_STAMP,
		ATTRCK_L_CLOW,
		ATTRCK_R_CLOW,
		ATTRCK_DASH,

		STUNNED,
		DEAD,
	};

	// 攻撃種別
	enum class ATTRCK_TYPE
	{
		NONE,
		BITE,
		CLOW_L,
		CLOW_R,
		TACKLE,
	};




	// コンストラクタ
	Boss(int key);

	// デストラクタ
	~Boss(void);

	void Init(void) override;
	void Update(void)override;
	void Draw(void)override;

	// ＨＰの取得
	const int GetHp(void) const { return hp_; }
	//ダメージ関係
	void Damage(int _dama, bool _isConst = false)override;

	const bool IsState(STATE state)const { return (state_ == state); }

	//外部クラストの当たり判定
	const bool CollisionCapsule(std::weak_ptr<Capsule> _capsule);
	const bool CollisionAttrck(const int& modelId);

	// プレイヤー種別(1P or 2P)
	const int GetAnim(void)const { return animeType_; }
	const int GetKey(void) const { return key_; }



	const VECTOR GetAttrckPos(void) const { return attrckPos_; }

	const bool IsHit(void)const { return isHitCheck_; }
	const float GetLerpTime(void)const { return lerpTime_; }
	const bool IsLerp(void)const { return isLerp_; }
	const int GetLerpId(void)const { return lerpId_; }

	const std::vector<std::unique_ptr<HitPart>>& GetHitParts(void) const { return hitParts_; }

	void SetLerpPos(const VECTOR pos);
	// LERP移動開始
	void StartLerp(void);
	// 追従対象の設定
	void SetFollow(const Transform* follow) override;
	// バトル終了
	void SetBattleCancel(void);
	// スタン状態開始
	void StartStunned(void);


	//BGM用バトル常態化の判定
	bool IsBattle(void) const;

private:

	
	// 状態管理
	STATE state_;
	int animeType_;
	int animeAgoType_;

	float stateTime_;	//状態時間

	//攻撃管理
	ATTRCK_TYPE attrckTypeState_;//攻撃種別

	float dameRate_;		//受けるダメージ倍率
	//ボス限定
	int attrckCount_;		//連続攻撃の際に使用
	VECTOR attrckPos_;		//攻撃判定中心位置
	float attrckRadius = 0.0f;

	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	

	//目的移動位置//※小型は範囲外に出たら消すので使わない
	float lerpTime_;
	VECTOR lerpPos_;
	bool isLerp_;
	int lerpId_;


	
	//複数の初期化処理
	void InitAnimation(void);
	void InitEffect(void);
	void InitSound(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateLerpMove(void);
	void ChangeStateBattle(void);
	void ChangeStateFollow(void);
	//攻撃系
	void ChangeStateAttrckReady(void);
	void ChangeStateAttrckStamp(void);
	void ChangeStateAttrckLeftClaw(void);
	void ChangeStateAttrckRightClaw(void);
	void ChangeStateAttrckDash(void);

	void ChangeStateStunned(void);
	void ChangeStateDamage(void);
	void ChangeStateDead(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateLerpMove(void);
	void UpdateBattle(void);
	void UpdateFollow(void);
	void UpdateAttrckReady(void);
	//牙攻撃
	void UpdateAttrckStamp(void);
	//爪攻撃
	void UpdateAttrckLeftClaw(void);
	void UpdateAttrckRightClaw(void);
	//突進攻撃
	void UpdateAttrckDash(void);

	void UpdateStunned(void);
	void UpdateDamage(void);
	void UpdateDead(void);

	//最終更新
	void CollisionStageCapsule(void)override;
	void CollisionGravity(void)override;

	//攻撃無効化
	void AttrckUpdate(void);

	// デバッグ用描画
	void DrawDebug(void);

	
};

