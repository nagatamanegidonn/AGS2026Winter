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

	void Update(void) override;

    int GetDamage(void) const override;

protected:

    // パラメータの設定
    void SetParam(void)override;
};

