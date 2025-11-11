#pragma once
#include <string>
#include "../CharaBase.h"

class AnimationController;
class EffectController;
class SoundController;

class Capsule;

class HitPart;
class HitDamage;

class EnemyBase :
    public CharaBase
{
public:

	EnemyBase(void);
	virtual ~EnemyBase(void);

	virtual void Init(void) override;
	virtual void Update(void)override;
	virtual void Draw(void)override;

	// 追従対象の設定
	virtual void SetFollow(const Transform* follow);

	// ダメージ関数
	virtual void Damage(int _dama, bool _isConst = false) = 0;

	// 視野内にターゲットがいるか
	bool IsTargetInFOV(const VECTOR& followPos, float fovDeg);

protected:

	//ユーザー番号
	int key_;
	int createNo_;

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;
	std::unique_ptr<EffectController> effectController_;
	std::unique_ptr<SoundController> soundController_;


	//当たり判定（複数）
	std::vector<std::unique_ptr<HitPart>> hitParts_;
	//ダメージ表記用変数
	std::vector<std::unique_ptr<HitDamage>> hitdamages_;

	//アニメーションの追加、設定
	void AddHitPart(int& model, std::wstring boneName, float rad, float rate);


	// 体力
	int hp_;
	int hpMax_;


	//ターゲットプレイヤー
	const Transform* follow_;
	float followTime_;

	//ターゲット回転
	float rotateTimer_ = 0.0f;          // 回転間隔のためのタイマー
	const float rotateInterval_ = 1.6f; // 例：0.2秒ごとに向き直す


	//カプセル
	std::unique_ptr<Capsule> capsule_;

	// 衝突チェック(カプセル)
	VECTOR gravHitPosDown_; //← 衝突用線分
	VECTOR gravHitPosUp_;	//← 衝突用線分
	VECTOR hitDamePos_;	//← 衝突用線分

	//最終更新
	virtual void CollisionStageCapsule(void)override;
	virtual void CollisionGravity(void)override;

	void TargetRotate(const VECTOR& traPos, float rate = 1.0f);
	void DrawFOV(float fovDeg, float radius, int rayCount, unsigned int color);

};

