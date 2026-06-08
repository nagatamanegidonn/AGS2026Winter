#pragma once
#include "ShotBase.h"

class ArrowShot :
    public ShotBase
{

public:

    // コンストラクタ
    ArrowShot(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key);

    // デストラクタ
    ~ArrowShot(void) override;

    // 更新処理
	void Update(void) override;

    // ダメージ量の取得
    int GetDamage(void) const override;

protected:

    // パラメータの設定
    void SetParam(void)override;
};

