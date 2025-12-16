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

    ViewPlayer();
    ~ViewPlayer();

	// Player.h or Player.cpp に追加
	void Init(void) override;
	void Update(void)override;
	void Draw(void)override;
	void Release(void);

	void SetChar(const int charId);
	void SetWeapon(const int weponId);

	const void SetPos(const VECTOR pos);
	const void SetLocalQua(const VECTOR rot);

private:

	// モデル制御の基本情報(武器用)
	Transform transWeapon_;

	// 状態管理(状態遷移時初期処理)
	std::map<int, std::function<void(void)>> SetWeapon_;
	std::function<void(void)> updateWeapon_;

	std::unique_ptr<AnimationController> animationController_;

	int weponId_;

	void SyncWeaponGreatSowrd();
	void SyncWeaponAxe();
	void SyncWeaponBow();

};

