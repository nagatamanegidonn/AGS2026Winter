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
class ItemPoach;
class Capsule;
class Collider;
class PixelMaterial;
class PixelRenderer;
class SceneBase;
class GameScene;

class Player : 
	public CharaBase
{

public:

	// 腰
	static constexpr VECTOR GSOWRD_SPINE_ROT = { 0.0f, 0.0f, 180.0f };
	static constexpr VECTOR GSOWRD_SPINE_POS = { 0.0f, 50.0f, 15.0f };
	// 右手
	static constexpr VECTOR GSOWRD_HAND_ROT = { 160.0f, 20.0f, 45.0f };
	static constexpr VECTOR GSOWRD_HAND_POS = { 0.0f, 0.0f, 0.0f };
	// ローカル回転
	static constexpr VECTOR WEPON_LOCAL_ROT = { 160.0f, 180.0f,0.0f };
	// 回転完了までの時間
	static constexpr float TIME_ROT = 0.1f;
	static constexpr float TIME_ROT2 = 10.0f;
	// チャージ攻撃
	static constexpr float CHAGE_MAX_TIME = 3.0f;					// チャージの最大時間
	static constexpr float CHAGE_UP_RATE = CHAGE_MAX_TIME / 4.0f;	// チャージ1段階にかかる時間
	static constexpr float INVISIBLE_SMALL_TIME = 0.5f;
	static constexpr float INVISIBLE_BIG_TIME = 2.5f;
	// スピード
	static constexpr float SPEED_MOVE = 5.0f;
	static constexpr float SPEED_RUN = 10.0f;
	static constexpr float SPEED_JUMP = SPEED_RUN * 8;
	static constexpr float SPEED_ROLL = 15.0f;
	// 煙エフェクト発生間隔
	static constexpr float FOOT_SMOKE = 0.3f;
	static constexpr float FAST_FOOT_SMOKE = 0.4f;
	// 最大ＨＰ
	static constexpr int MAX_HP = 100;
	static constexpr int BONUS_HP = 20;
	static constexpr int MAX_STAMINA = 100;
	static constexpr float STAMINA_BREAK = 15.0f;	// 疲労じょうたいのボーダーライン
	static constexpr float UP_TAF = 12.0f;	// スタミナ回復量
	static constexpr float DOWN_TAF = -3.0f;// スタミナ減少量
	static constexpr float ROLL_TAF = 25.0f;// スタミナ消費量（回避）
	// 当たり判定倍率
	static constexpr float COLL_LEG_RATE = 40;	// 脚当たり判定倍率
	static constexpr float DOWN_MAX = 1.2f;		// ダウン最大時間
	// 索敵範囲
	static constexpr float MAX_EAR_RADIUS = 1000.0f;
	// エフェクト関連
	static constexpr int POWER_UP_EFFECT = 0;
	static constexpr int POWER_SLASH_EFFECT = 1;
	static constexpr float PLAYER_EFFECT_SCALE = 30.0f;

	// 状態
	enum class STATE
	{
		NONE,		
		PLAY,		// 通常状態
		BATTLE,		// 戦闘状態
		WEAPON,		// 抜刀、納刀
		ATTACK,		// 攻撃
		ROWLING,	// 回避	
		DAMAGE,		// ダメージ（のけぞり）
		HI_DAMAGE,	// ダメ―ジ（吹っ飛び）
		DEAD,		// 死亡
		GET,		// 採取
		ITEM_PLAY,	// 使用
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		// 通常アニメーション
		IDLE,
		RUN,
		FAST_RUN,
		ROLL,
		// 採取、使用
		GET,
		ITEM_DRINK,
		ITEM_THROW,
		ITEM_THROW_E,
		ITEM_SET,
		ITEM_SET_E,
		// 抜刀
		DRAW,
		BATTLE_DRAW,
		// 納刀
		CLOSE,
		BATTLE_CLOSE,
		// バトル時アニメーション
		BTLLE_IDLE,
		BTLLE_RUN,
		BTLLE_FAST_RUN,
		// ダメージ
		DAMAGE,
		FLYING,
		DOWN,
		// 攻撃
		ATTACK1S,
		ATTACK1STOP,
		ATTACK1E,
		ATTACK2,
		ATTACK3,
		// 死亡
		DEAD,
	};

	// プレイヤー自身から発生されるサウンド
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
		ATTACK1,
		ATTACK2,
		ATTACK3,
		MAX// 音なしfor文用
	};

	// コンストラクタ
	Player(int key, GameScene* scene, PLAYER_TYPE type);

	// デストラクタ
	~Player(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;
	virtual void DrawUI(int i);

	// 解放処理
	void Release(void);

	// 座標の取得
	const Transform& GetTransWeapon(void) const { return transWeapon_; }

	// ユーザー番号の取得
	inline const int GetKey(void) const { return key_; }

	// ＨＰの取得
	inline const int GetHp(void) const { return hp_; }

	// ダメージ関係
	void Damage(int dama,const VECTOR atkPos, const VECTOR mixDir);

	// 衝突しているか
	const bool IsHit(void) const;
	const bool GetHit(void) const;
	const void SetHit(bool flag);

	// カプセルの取得
	std::weak_ptr<Capsule> GetCapsule(void);

	// プレイヤー種別(1P or 2P)
	const PLAYER_TYPE& GetPlayerType(void)const;

	// カプセルとの当たり判定
	bool CollisionCapsule(int& modelId)const;

	// 球体との当たり判定
	const bool CollisionUnderSphere(const VECTOR pos,float r)const;

	// 攻撃アニメーション判定
	const bool IsAttack(void) const;	// 攻撃をしているか
	const bool IsLoopAnim(void) const;	// 再生アニメーションがループするか

	// 自身のプレイヤーかどうか
	const bool IsSelf(void) const;

	// 注目判定
	bool IsAimSet(void);	// 狙い撃ちをしているか
	bool IsTrgAimSet(void);	// 注目をしているか

	// 通信専用の攻撃アニメーション判定
	virtual bool IsSyncAttack(void);

	// 採取の際の情報（仮）
	void SetItemId(int id) { itemId_ = id; }

protected:

	// ゲームシーンのポインタ変数
	GameScene* gameScene_;

	// ユーザー番号
	int key_;

	// モデル制御の基本情報(武器用)
	Transform transWeapon_;

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;
	// 操作コントローラー
	std::unique_ptr<InputController> inputController_;
	// エフェクトコントローラー
	std::unique_ptr<EffectController> effectController_;
	// サウンドコントローラー
	std::unique_ptr<SoundController> soundController_;

	// 状態管理
	STATE state_;
	int animeType_;		// アニメーションの種類
	int animeAgoType_;	// 前フレームのアニメーションの種類

	// 抜刀状態管理
	bool isBattle_;		// 抜刀中か判断する
	bool isDrawWeapon_;	// 抜刀動作中か判断する
	bool isCloseWeapon_;	// 納刀動作中か判断する

	// 攻撃管理
	bool isHit_;		// ダメージが連続で入らないための判定//攻撃中一度ダメージを与えてらtrueに
	float chageCount_ = 0.0f;
	
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// UI
	int freamImg_;		// フレーム画像
	int jobImg_;		// ジョブアイコン
	int hpImg_;			// HP画像（色を変えスタミナにも使用）
	int hpFreamImg_;	// HPフレーム画像」
	int hpMaskImg_;		// HPマスク画像
	int staFreamImg_;	// スタミナフレーム画像
	int staMaskImg_;	// スタミナマスク画像

	// ステータスUI
	std::unique_ptr<PixelMaterial> statusMaterial_;
	std::unique_ptr<PixelRenderer> statusRenderer_;
	// HPバーUI
	std::unique_ptr<PixelMaterial> hpMaterial_;
	std::unique_ptr<PixelRenderer> hpRenderer_;
	// スタミナUI
	std::unique_ptr<PixelMaterial> staMaterial_;
	std::unique_ptr<PixelRenderer> staRenderer_;

	// カプセル（判定用）
	std::unique_ptr<Capsule> capsule_;
	std::shared_ptr<Capsule> capsuleWeapon_;

	// 衝突チェック
	VECTOR gravHitPosDown_; //← 衝突用線分
	VECTOR gravHitPosUp_;	//← 衝突用線分

	// プレイヤー種別
	PLAYER_TYPE type_;

	//キャラごとの固有起動フラグ
	bool isBattleDash_;
	float walkTime_;

#pragma region パラメーター

	// 体力系
	int hp_;	// 現在HP
	int hpAgo_;	// 前フレームのHP
	int hpMax_;	// 最大HP 
	float damage_;	// ダメージ

	// スタミナ
	float stamina_;		// 現在スタミナ
	float staminaMax_;	// 最大スタミナ
	float staminaDir_;	// スタミナの加算値（ダッシュ中はマイナス）
	bool isBreak_;		// 疲労(スタミナが０になったか判定)

#pragma endregion

	// ダメージ関係
	float invisibleTime_;	// 無敵時間
	VECTOR flyigDir_;		// 吹き飛び方向
	float flyigTime_;		// 吹き飛び時間
	float downTime_;		// 起き上がり始めるまでの時間

	// 採取の際の情報（仮）
	// 採取行動の際にどのアイテムがとれるかをId管理（何もないときはー１）
	int itemId_;

	// アイテムポーチ
	std::unique_ptr <ItemPoach> poach_;

	// 複種の初期化処理
	virtual void InitParam(void);
	virtual void InitAnimation(void);
	virtual void InitEffect(void);
	void InitSound(void);				// 通常サウンド
	virtual void InitAttackSound(void);	// 戦闘用サウンド
	virtual void InitShader(void);		// シェーダー初期化
	// 攻撃音再生
	virtual void PlayAttackSound(void) {};

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateWeapon(void);
	void ChangeStateBattle(void);
	void ChangeStateAttack(void);
	void ChangeStateRowling(void);
	void ChangeStateDamage(void);
	void ChangeStateHiDamage(void);
	void ChangeStateDead(void);
	void ChangeStateGet(void);
	void ChangeStateItemUse(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateBattle(void);
	void UpdateWeapon(void);
	void UpdateAttack(void);
	void UpdateRowling(void);
	void UpdateDamage(void);
	void UpdateHiDamage(void);
	void UpdateDead(void);
	void UpdateGet(void);
	void UpdateItemUse(void);
#pragma region アイテムごとの関数
	void UseItem(void);
#pragma endregion

	// 移動
	void ProcessNormal(void);	// 通常操作
	void ProcessBattle(void);	// 戦闘操作

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);
	float CreateRad(const VECTOR& dir);

	// 武器の描画
	virtual void DrawWeapon();
	// 武器の同期
	const void SyncWeapon();
	virtual void SyncWeaponPlay();	// 通常
	virtual void SyncWeaponBattle();// 戦闘中

	/// <summary>
	/// モデルのフレーム追従
	/// </summary>
	/// <param name="frameName">追従フレーム</param>
	/// <param name="offsetRot">角度オフセットR,U,F</param>
	/// <param name="offsetPos">位置オフセットR,U,F</param>
	/// <param name="modelTransform">追従対象</param>
	/// <param name="outWeaponTransform">追従物</param>
	/// <returns></returns>
	const void SyncWeaponToFream(const TCHAR* frameName, const VECTOR& offsetRot, const VECTOR& offsetPos,
		const Transform& modelTransform, Transform& outWeaponTransform);

	// 当たり判定の最終処理
	void CollisionStageCapsule(void)override;	// カプセルとステージの判定
	void CollisionGravity(void)override;		// 重力計算とその後の判定
		
	// モーション終了
	bool IsEndLanding(void);

	// 操作可能か
	const bool IsInputPlay(void)const;

	// 攻撃キャンセル時の処理
	void AttackReset(void);

	// アニメーション終了時の処理
	void ChangeStateAnimeEnd(const ANIM_TYPE anim, const std::function<void(void)> function = {});

	// デバッグ用描画
	void DrawDebug(void);

	// attackDataを基にした更新
	void AttackUpdate(void);

};

