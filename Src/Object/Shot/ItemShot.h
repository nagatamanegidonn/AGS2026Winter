#pragma once
#include "ShotBase.h"

class ItemShot :
    public ShotBase
{
public:
    // コンストラクタ
    ItemShot(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key);
    // デストラクタ
	~ItemShot(void);

    void Update(void) override;

protected:

    // パラメータの設定
    void SetParam(void)override;

    void UpdateShot(void)override;
    void UpdateBlast(void)override;
                        
};

