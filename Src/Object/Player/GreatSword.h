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
    GreatSword(int key, GameScene* scene, PLAYER_TYPE type);

    // デストラクタ
    ~GreatSword(void);

    bool IsSyncAttrck() override;

private:

    // 初期化
    void InitParam(void)override;          // パラメータ
    void InitAnimation(void) override;     // アニメーション
    void InitEffect(void) override;        // エフェクト
    void InitAttrckSound(void) override;   // サウンド

    // 攻撃音再生
    void PlayAttrckSound(void) override;

    // 武器の同期
    void SyncWeaponPlay()override;      // 通常時
    void SyncWeaponBattle()override;    // 戦闘時
};

