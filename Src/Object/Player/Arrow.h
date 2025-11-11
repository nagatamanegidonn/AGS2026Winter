#pragma once
#include "Player.h"

class Arrow :
    public Player
{
public:

    //背中
    static constexpr VECTOR BOW_SPINE_ROT = { 0.0f, 0.0f, 30.0f };
    static constexpr VECTOR BOW_SPINE_POS = { 0.0f, 0.0f, 15.0f };    
    //左手
    static constexpr VECTOR BOW_LHAND_ROT = { 0.0f, 70.0f, -90.0f };
    static constexpr VECTOR BOW_LHAND_POS = { 0.0f, 0.0f, -3.0f };
    //左手
    static constexpr VECTOR ARROW_RHAND_ROT = { 0.0f, 0.0f, -90.0f };
    static constexpr VECTOR ARROW_RHAND_POS = { 0.0f, 0.0f, -3.0f };

    // コンストラクタ
    Arrow(int key);

    // デストラクタ
    ~Arrow(void);

	bool IsSyncAttrck() override;

private:

    Transform transSubWeapon_;

    void InitParam(void)override;
    void InitAnimation(void) override;
    void InitEffect(void) override;

    void InitAttrckSound(void) override;
    void PlayAttrckSound(void) override;

    void WeaponDraw()override;
    void SyncWeaponPlay()override;
    void SyncWeaponBattle()override;


};

