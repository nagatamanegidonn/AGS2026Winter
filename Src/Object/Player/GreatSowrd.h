#pragma once
#include "Player.h"

class GreatSowrd :
    public Player
{
public:

    // コンストラクタ
    GreatSowrd(int key);

    // デストラクタ
    ~GreatSowrd(void);

    void InitPram(void)override;
    void InitAnimation(void) override;
    void InitEffect(void) override;

    void InitAttrckSound(void) override;
    void PlayAttrckSound(void) override;

    void SyncWeaponPlay()override;
    void SyncWeaponBattle()override;
};

