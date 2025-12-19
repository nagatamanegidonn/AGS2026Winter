#pragma once
#include "Player.h"

#include "../Lib/nlohmann/json.hpp"

// 長いのでnamespaceの省略
using json = nlohmann::json;

class GreatSword :
    public Player
{

public:

    // 腰
    static constexpr VECTOR GSOWRD_SPINE_ROT = { 0.0f, 0.0f, 180.0f };
    static constexpr VECTOR GSOWRD_SPINE_POS = { 0.0f, 50.0f, 15.0f };
    // 右手
    static constexpr VECTOR GSOWRD_HAND_ROT = { 160.0f, 20.0f, 45.0f };
    static constexpr VECTOR GSOWRD_HAND_POS = { 0.0f, 0.0f, 0.0f };

    // コンストラクタ
    GreatSword(int key);

    // デストラクタ
    ~GreatSword(void);

    bool IsSyncAttrck() override;

private:

    // 初期化
    void InitParam(void)override;
    void InitAnimation(void) override;
    void InitEffect(void) override;
    void InitAttrckSound(void) override;
    // 攻撃音再生
    void PlayAttrckSound(void) override;

    void SyncWeaponPlay()override;
    void SyncWeaponBattle()override;
};

