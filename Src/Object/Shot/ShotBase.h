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
        BOM,
    };

    // コンストラクタ
    ShotBase(int damage, const VECTOR& birthPos, const VECTOR& shotVec, int key);

    // デストラクタ
    virtual ~ShotBase(void);

    // 生成関数
    void Create(int damage, const VECTOR& birthPos, const VECTOR& dir, int key);

    // 初期化処理
    virtual void Init(void) override;
    
    // 更新処理
    virtual void Update(void);

    // 描画処理
    void Draw(void);

    // 種類の取得
    const TYPE& GetType(void) const { return type_; }

    // 座標の取得
    const int& GetTransItem(void) const { return transform_.modelId; }

	// 半径の取得
	const float& GetRadius(void) const { return radius_; }

    // 弾を発射したプレイヤーのキー取得
    int GetKey(void)const { return key_; }
    bool IsShot(void)const { return state_ == STATE::SHOT; }
    bool IsBlast(void)const { return state_ == STATE::BLAST; }
    bool IsEnd(void)const { return state_ == STATE::END; }

	// ダメージ量の取得
    virtual int GetDamage(void) const;

	// カプセルコライダの取得
    std::weak_ptr<Capsule> GetCapsule(void);

    // 外部クラストの当たり判定
    bool CollisionCapsule(std::weak_ptr<Capsule> _capsule)const;

	// 状態遷移
    void ChangeState(STATE _state= STATE::END);

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
    float radius_;

    // パラメータの設定
    virtual void SetParam(void);
    void CheckAlive(void);

    virtual void UpdateShot(void);
    virtual void UpdateBlast(void);
};

