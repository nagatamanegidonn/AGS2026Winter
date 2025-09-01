#pragma once
#include "../ActorBase.h"

class ShotBase :
    public ActorBase
{
public:

    // èÛë‘
    enum class STATE
    {
        NONE,
        SHOT,
        BLAST,
        END,
    };

    ShotBase(int damage,const VECTOR birthPos, const VECTOR shotVec, int key);

    virtual ~ShotBase(void);

    virtual void Create(int damage,const VECTOR birthPos, const VECTOR dir, int key);
    void Init(void);
    void Update(void);
    void Draw(void);

    // ç¿ïWÇÃéÊìæ
    const int& GetTransItem(void) const { return transform_.modelId; }

    const int GetKey(void)const { return key_; }
    const bool IsShot(void)const { return state_ == STATE::SHOT; }
    const bool IsEnd(void)const { return state_ == STATE::END; }

    const int GetDamage(void);

    void Destroy(void);

private:

    int damage_;
    float speed_;
    float timeAlive_;

    STATE state_;
    VECTOR shotVec_;
    int key_;

    void SetParam(void);
    void CheckAlive(void);

    // èÛë‘ëJà⁄
    void ChangeState(STATE state);

    void UpdateShot(void);
    void UpdateBlast(void);

};

