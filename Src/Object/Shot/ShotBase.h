#pragma once
#include <memory>
#include "../ActorBase.h"

class Capsule;

class ShotBase :
    public ActorBase
{
public:

    // 状態
    enum class STATE
    {
        NONE,
        SHOT,
        BLAST,
        END,
    };

	// 種類
    enum class TYPE
    {
        NONE,
        ARROW,
        ITEM,
	};

	// コンストラクタ
    ShotBase(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key);
	// デストラクタ
    virtual ~ShotBase(void);

    void Create(int damage,const VECTOR& birthPos, const VECTOR& dir, int key);
    virtual void Init(void) override;
    virtual void Update(void);

    void Draw(void);

	// 種類の取得
	const TYPE& GetType(void) const { return type_; }
    // 座標の取得
    const int& GetTransItem(void) const { return transform_.modelId; }
	// 弾を発射したプレイヤーのキー取得
    const int GetKey(void)const { return key_; }
    const bool IsShot(void)const { return state_ == STATE::SHOT; }
    const bool IsEnd(void)const { return state_ == STATE::END; }

    const int GetDamage(void);
    std::weak_ptr<Capsule> GetCapsule(void);

    void Destroy(void);

protected:
	// 種類
	TYPE type_;
	// ダメージ量
    int damage_;
	// 速度
    float speed_;
	// 生存時間
    float timeAlive_;

    STATE state_;
    VECTOR shotVec_;
    int key_;

    std::shared_ptr<Capsule> capsule_;

    // パラメータの設定
    virtual void SetParam(void);
    void CheckAlive(void);

    // 状態遷移
    void ChangeState(STATE state);

   virtual void UpdateShot(void);
   virtual void UpdateBlast(void);

};

