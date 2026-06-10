#pragma once
#include "ShotBase.h"

class EffectController;

class BomObject :
    public ShotBase
{

public:

    // コンストラクタ
    BomObject(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key);

    // デストラクタ
    ~BomObject(void);

    void Update(void) override;

protected:

    std::unique_ptr<EffectController> effectController_;

    // パラメータの設定
    void SetParam(void) override;

    void UpdateShot(void) override;
    void UpdateBlast(void) override;

};

