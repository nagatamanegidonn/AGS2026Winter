#pragma once
#include "Player.h"

class Arrow :
    public Player
{
public:

    // コンストラクタ
    Arrow(int key);

    // デストラクタ
    ~Arrow(void);


private:

    Transform transSubWeapon_;

    void InitPram(void)override;
    void InitAnimation(void) override;
    void InitEffect(void) override;

    void InitAttrckSound(void) override;
    void PlayAttrckSound(void) override;

    void WeaponDraw()override;
    void SyncWeaponPlay()override;
    void SyncWeaponBattle()override;


};

