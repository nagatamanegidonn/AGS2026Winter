#pragma once
#include "Player.h"

class Arrow :
    public Player
{

public:

    // 背中
    static constexpr VECTOR BOW_SPINE_ROT = { 0.0f, 0.0f, 30.0f };
    static constexpr VECTOR BOW_SPINE_POS = { 0.0f, 0.0f, 15.0f };    
    // 左手
    static constexpr VECTOR BOW_LHAND_ROT = { 0.0f, 70.0f, -90.0f };
    static constexpr VECTOR BOW_LHAND_POS = { 0.0f, 0.0f, -3.0f };
    // 左手
    static constexpr VECTOR ARROW_RHAND_ROT = { 0.0f, 0.0f, -90.0f };
    static constexpr VECTOR ARROW_RHAND_POS = { 0.0f, 0.0f, -3.0f };

    // コンストラクタ
    Arrow(int key, GameScene* scene, PLAYER_TYPE type);

    // デストラクタ
    ~Arrow(void);

    // 通信専用の攻撃アニメーション判定
	bool IsSyncAttrck() override;

private:

    // 武器モデル（矢）
    Transform transSubWeapon_;

	// 初期化
    void InitParam(void)override;          // パラメータ
    void InitAnimation(void) override;     // アニメーション
    void InitEffect(void) override;        // エフェクト
    void InitAttrckSound(void) override;   // サウンド

    // 攻撃音再生
    void PlayAttrckSound(void) override;

    // 武器の描画
    void DrawWeapon()override;

    // 武器の同期
    void SyncWeaponPlay()override;      // 通常時
    void SyncWeaponBattle()override;    // 戦闘時


};

