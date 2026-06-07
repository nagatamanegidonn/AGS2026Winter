#pragma once
#include <memory>
#include <functional>
#include <map>

#include "../ActorBase.h"

class AnimationController;

// セレクトシーン、コネクトシーンで使用
class ViewPlayer :
    public ActorBase
{

public:

	// コンストラクタ
	ViewPlayer(void);

	// デストラクタ
	~ViewPlayer(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

	// 解放処理
	void Release(void);

	// キャラの設定
	void SetChar(const int charId);

	// 武器の設定
	void SetWeapon(const int weponId);

	// 位置の設定
	const void SetPos(const VECTOR pos);

	// ローカル回転の設定
	const void SetLocalQua(const VECTOR rot);

private:

	// モデル制御の基本情報(武器用)
	Transform transWeapon_;

	// 状態管理(状態遷移時初期処理)
	std::map<int, std::function<void(void)>> SetWeapon_;
	std::function<void(void)> updateWeapon_;

	// アニメーションコントローラー
	std::unique_ptr<AnimationController> animationController_;

	// 武器ID
	int weponId_;

	// 武器ごとによる一位の同期処理
	void SyncWeaponGreatSowrd();	// 大剣
	void SyncWeaponSowrd();			// 片手剣
	void SyncWeaponBow();			// 弓

};

