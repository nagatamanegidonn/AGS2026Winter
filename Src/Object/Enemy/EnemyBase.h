#pragma once
#include <string>
#include "../CharaBase.h"

class AnimationController;
class EffectController;
class SoundController;
// 判定
class Capsule;
// ダメージ関係
class HitPart;
class HitDamage;

class EnemyBase :
    public CharaBase
{

public:

	// コンストラクタ
	EnemyBase(void);

	// デストラクタ
	virtual ~EnemyBase(void);

	// 初期化処理
	virtual void Init(void) override;

	// 更新処理
	virtual void Update(void) override;

	// 描画処理
	virtual void Draw(void) override;

	// 追従対象の設定
	virtual void SetFollow(const Transform* follow);

	// ダメージ関数
	virtual void Damage(int _dama, bool _isConst = false) = 0;

	// 視野内にターゲットがいるか
	bool IsTargetInFOV(const VECTOR& followPos, float fovDeg);

protected:

	// ネット関係
	int key_;		// ユーザー番号
	int createNo_;	// 生成数

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;

	// エフェクトコントローラー
	std::unique_ptr<EffectController> effectController_;

	// サウンドコントローラー
	std::unique_ptr<SoundController> soundController_;

	// 当たり判定（複数）
	std::vector<std::unique_ptr<HitPart>> hitParts_;
	// ダメージ表記用変数
	std::vector<std::unique_ptr<HitDamage>> hitdamages_;

	// アニメーションの追加、設定
	void AddHitPart(int& model, std::wstring boneName, float rad, float rate);

	// 体力
	int hp_;
	int hpMax_;

	// ターゲットプレイヤー
	const Transform* follow_;
	float followTime_;

	// ターゲット回転
	float rotateTimer_ = 0.0f;          // 回転間隔のためのタイマー
	const float rotateInterval_ = 1.6f; // 例：0.2秒ごとに向き直す

	// カプセル
	std::unique_ptr<Capsule> capsule_;

	// 衝突チェック(カプセル)
	VECTOR gravHitPosDown_; // 衝突用線分
	VECTOR gravHitPosUp_;	// 衝突用線分
	VECTOR hitDamePos_;		// 衝突用線分

	// 最終更新
	virtual void CollisionStageCapsule(void) override;	// ステージとの判定
	virtual void CollisionGravity(void) override;		// 重力判定

	// ダメージ描画クラス生成などの更新処理
	virtual int DamageUpdate(void);

	// 目標位置まで回転させる関数
	void TargetRotate(const VECTOR& traPos, float rate = 1.0f);

	// 視覚判定
	void DrawFOV(float fovDeg, float radius, int rayCount, unsigned int color);

};

