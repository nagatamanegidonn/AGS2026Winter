#pragma once
#include "Player.h"

#include "../Lib/nlohmann/json.hpp"

// 長いのでnamespaceの省略
using json = nlohmann::json;

class Sword :
    public Player
{

public:

    // 背中
    static constexpr VECTOR SOWRD_SPINE_ROT = { 0.0f, 0.0f, -120.0f };
    static constexpr VECTOR SOWRD_SPINE_POS = { -30.0f, 0.0f, 15.0f };
    // 左手
    static constexpr VECTOR SOWRD_HAND_ROT = { 15.0f, 50.0f, -80.0f };
    static constexpr VECTOR SOWRD_HAND_POS = { 0.0f, 0.0f, -3.0f };

    // コンストラクタ
    Sword(int key);

    // デストラクタ
    ~Sword(void);

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

