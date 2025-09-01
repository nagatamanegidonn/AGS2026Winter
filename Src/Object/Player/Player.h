#pragma once
#include <functional>

#include "../Net/NetStructures.h"
#include "../Common/Vector2F.h"
#include "../Utility/AsoUtility.h"

#include "../CharaBase.h"

class AnimationController;
class InputController;
class EffectController;
class SoundController;

class Capsule;
class Collider;

class PixelMaterial;
class PixelRenderer;

class SceneBase;
class GameScene;

class Player : public CharaBase
{

public:


	// 回転完了までの時間
	//static constexpr float TIME_ROT = 1.0f;
	static constexpr float TIME_ROT = 0.1f;
	static constexpr float TIME_ROT2 = 10.0f;

	static constexpr float CHAGE_MAX_TIME = 3.0f;
	static constexpr float INVISIBLE_SMALL_TIME = 0.5f;
	static constexpr float INVISIBLE_BIG_TIME = 2.5f;

	// スピード
	static constexpr float SPEED_MOVE = 5.0f;
	static constexpr float SPEED_RUN = 10.0f;
	static constexpr float SPEED_JUMP = SPEED_RUN * 8;
	//static constexpr float SPEED_ROLL = 7.5f;
	static constexpr float SPEED_ROLL = 15.0f;


	// 煙エフェクト発生間隔
	static constexpr float FOOT_SMOKE = 0.3f;
	static constexpr float FAST_FOOT_SMOKE = 0.4f;

	// 最大ＨＰ
	static constexpr int MAX_HP = 100;
	static constexpr int MAX_STAMINA = 100;
	static constexpr float UP_TAF = 12.0f;	//スタミナ回復量
	static constexpr float DOWN_TAF = -3.0f;//スタミナ減少量
	static constexpr float ROLL_TAF = 25.0f;//スタミナ消費量（回避）

	static constexpr float COLL_LEG_RATE = 40;
	static constexpr float DOWN_MAX = 1.2f;

	static constexpr float MAX_EAR_RADIUS = 1000.0f;


	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		BATTLE,
		WEPON,

		ATTRCK,

		ROWLING,//回避

		DAMAGE,		//ダメージ
		HI_DAMAGE,	//

		DEAD,
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		BLEND_IDLE,
		RUN,
		FAST_RUN,
		ROLL,

		//抜刀
		DRAW,
		BATTLE_DRAW,
		//納刀
		CLOSE,
		BATTLE_CLOSE,

		BTLLE_IDLE,
		BTLLE_RUN,
		BTLLE_FAST_RUN,

		DAMAGE,
		FLYING,
		DOWN,

		ATTRCK1S,
		ATTRCK1STOP,
		ATTRCK1E,

		ATTRCK2,
		ATTRCK3,

		DEAD,
	};

	enum class SE
	{
		CHAGE,
		DRAW,
		CLOSE,
		WALK,
		RUN,
		ROLL,

		DAMAGE,
		HI_DAMAGE,
		DOWN,

		ATTRCK1,
		ATTRCK2,
		ATTRCK3,
	};

	// キーコンフィグ(1P、2P)
	struct KEY_CONFIG
	{
		int UP;
		int DOWN;
		int LEFT;
		int RIGHT;
		int GAME_END;
		int JUMP;
	};

	// コンストラクタ
	Player(int key);

	// デストラクタ
	~Player(void);

	void Init(GameScene* scene, PLAYER_TYPE type, KEY_CONFIG config);
	// Player.h or Player.cpp に追加
	void Init(void) override 
	{
		// 必要な初期化処理を書く。何もなければ空でもOK。
	}
	void Update(void)override;
	void Draw(void)override;
	virtual void DrawUI(int i);
	void Release(void);

	

	// 座標の取得
	const Transform& GetTransItem(void) const { return transWeapon_; }

	const int GetKey(void) const { return key_; }

	// ＨＰの取得
	const int GetHp(void) const { return hp_; }
	//ダメージ関係
	void Damage(int dama,const VECTOR atkPos, const VECTOR mixDir);

	const bool IsHit(void) const;
	const bool GetHit(void) const;
	const void SetHit(bool flag);

	// プレイヤー種別(1P or 2P)
	PLAYER_TYPE GetPlayerType(void)const;
	bool CollisionCapsule(int& modelId)const;
	const bool CollisionSphere(const VECTOR pos,float r)const;

	const bool IsAttrck(void) const;//通信プレイヤーのことは不明
	const bool IsLoopAnim(void);//通信プレイヤー

	bool IsSelf(void);//このプレイヤーが自身かどうか返す関数

	//注目するか
	bool IsAimSet(void);
	bool IsTrgAimSet(void);

protected:

	VECTOR demoRot_;

	// ゲームシーンのポインタ変数
	GameScene* gameScene_;
	//ユーザー番号
	int key_;

	// モデル制御の基本情報(武器用)
	Transform transWeapon_;

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;
	std::unique_ptr<InputController> inputController_;
	std::unique_ptr<EffectController> effectController_;
	std::unique_ptr<SoundController> soundController_;

	// 状態管理
	STATE state_;
	int animeType_;
	int animeAgoType_;

	//抜刀状態管理
	bool isBattle_;		//抜刀中か判断する
	bool isDrawWepon_;
	bool isCloseWepon_;

	//攻撃管理
	bool isHit_;		//ダメージが連続で入らないための判定//攻撃中一度ダメージを与えてらtrueに
	float chageCount_ = 0.0f;
	
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	//std::map<ANIM_TYPE, std::function<void(void)>> attrckChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//UI
	int freamImg_;//
	int jobImg_;//ジョブアイコン
	int hpImg_;//
	int hpFreamImg_;//
	int hpMaskImg_;//
	int staFreamImg_;//
	int staMaskImg_;//
	std::unique_ptr<PixelMaterial> Material_;
	std::unique_ptr<PixelRenderer> Renderer_;
	std::unique_ptr<PixelMaterial> hpMaterial_;
	std::unique_ptr<PixelRenderer> hpRenderer_;
	std::unique_ptr<PixelMaterial> staMaterial_;
	std::unique_ptr<PixelRenderer> staRenderer_;

	//カプセル
	std::unique_ptr<Capsule> capsule_;

	// 衝突チェック
	VECTOR gravHitPosDown_; //← 衝突用線分
	VECTOR gravHitPosUp_;	//← 衝突用線分


	// プレイヤー種別(1P or 2P)
	PLAYER_TYPE type_;

	// キー設定
	KEY_CONFIG keyConfig_;

	//キャラごとの固有起動フラグ
	bool isBattleDash_;
	float walkTime_;

	// 体力	//ダメージ系
	int hp_;
	int hpAgo_;
	int hpMax_;
	float damage_;
	//スタミナ
	float stamina_;
	float staminaMax_;
	float staminaDir_;
	bool isBreak_;//疲労

	float invisibleTime_;
	VECTOR flyigDir_;
	float flyigTime_;
	float downTime_;

	virtual void InitPram(void);
	virtual void InitAnimation(void);
	virtual void InitEffect(void);
	virtual void InitSound(void);
	virtual void InitAttrckSound(void);
	virtual void PlayAttrckSound(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateWepon(void);
	void ChangeStateBattle(void);
	void ChangeStateAttrck(void);
	void ChangeStateRowling(void);
	void ChangeStateDamage(void);
	void ChangeStateHiDamage(void);
	void ChangeStateDead(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateBattle(void);
	void UpdateWepon(void);
	void UpdateAttrck(void);
	void UpdateRowling(void);
	void UpdateDamage(void);
	void UpdateHiDamage(void);
	void UpdateDead(void);



	// 移動
	void ProcessMove(void);
	void ProcessBattleMove(void);

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);
	float CreateRad(const VECTOR& dir);


	//武器の同期
	const void SyncWeapon();
	virtual void WeaponDraw();
	virtual void SyncWeaponPlay();
	virtual void SyncWeaponBattle();
	const void SyncWeaponToHand(const TCHAR* frameName, const VECTOR& offsetRot, const VECTOR& offsetPos,
		const Transform& modelTransform, Transform& outWeaponTransform);

	//最終更新
	void CollisionStageCapsule(void)override;
	void CollisionGravity(void)override;
	//bool CollisionCapsule(int& modelId);

	
	// モーション終了
	bool IsEndLanding(void);
	//操作可能か
	const bool IsInputPlay(void)const;

	//攻撃キャンセル時の処理
	void AttrckReset(void);
	void ChangeStateAnimeEnd(const ANIM_TYPE anim);

	// デバッグ用描画
	void DrawDebug(void);

	void AttrckUpdate(void);

};

