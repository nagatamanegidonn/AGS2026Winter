#pragma once
#include <vector>
#include <map>
#include <memory>
#include <DxLib.h>
#include "ActorBase.h"

class CharaBase abstract :
	public ActorBase
{

public:

	// アニメーション情報構造体
	struct AnimationInfo
	{
		int type;
		std::wstring name;
		float speed;
		int loopNum;
		float startFrame;
		float endFrame;
	};

	struct SoundData
	{
		int type;			// ID
		std::wstring path;	// リソースパス
		float vol;			// 音量
	};

	struct ShaderData
	{
		std::wstring path;	// リソースパス
		int bufNum;			// バッファ数
		std::vector<FLOAT4> bufs;	// バッファの数値
	};

	struct ActionData
	{
		bool isCharge = false;	// チャージ攻撃か
		int chargeId = -1;		// charge中のナンバー
		float sHitTime = 0.0f;	// 判定発生時間
		float HitTime = 0.0f;
		float sNewTime = 0.0f;	// n入力受付時間
		float NewTime = 0.0f;	// n入力受付終了時間
		int nextAttack = -1;	// 次の攻撃ID 
	};

	// コンストラクタ
	CharaBase(void);

	// デストラクタ
	virtual ~CharaBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;

	// 最終更新
	void Collision(void);
	virtual void CollisionStageCapsule(void);
	virtual void CollisionGravity(void);
	virtual void CollisionMoveEnd(void);

	// 衝突判定に用いられるコライダ制御
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 攻撃量の取得
	const int GetAttrckPow(void) const { return attackDamage_; }
	const float GetAttrckRate(void) const { return attackRate_; }

	// 現エリアの設定：取得
	void SetAreaId(int id);
	const int& GetAreaId(void) const { return areaId_; }

protected:

#pragma region 移動,回転,重力

	// 更新前の座標
	VECTOR prePos_;

	// 移動スピード
	float speed_;

	// 移動方向
	VECTOR moveDir_;

	// 移動量
	VECTOR movePow_;

	// 移動後の座標
	VECTOR movedPos_;

	// 回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// ジャンプ量
	VECTOR jumpPow_;

#pragma endregion

	// 影画像
	int imgShadow_;	

	// 影の描画
	void DrawShadow(void);

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	// エリア管理
	int areaId_;

	// 移動量の計算
	void CalcGravityPow(void);

	// 時間のカウント
	void CountTime(float& time);

	// 種類別のアニメーションデータ
	std::map<int, std::unique_ptr<ActionData>> atkData_;// アタックアニメデータ
	int attackType_ = 0;								// アタックデータ使用ID

	// 攻撃管理
	bool isHitCheck_;			// 攻撃判定が発生するかの判定
	float changeAttackTime_;	// 攻撃チャージ時間
	int attackDamage_;			// 攻撃力
	float attackRate_ = 1.0f;	// 攻撃率

	/// <summary>
	/// アタックアニメデータ設定関数（ActionDataのほうが良い）
	/// </summary>
	/// <param name="nextAtkId">アニメーションが終わった後のアニメ</param>
	/// <param name="sHitTim">判定開始時間</param>
	/// <param name="HitTim">判定終了時間</param>
	/// <param name="sNewTime">操作受付開始時間</param>
	/// <param name="isChage">操作受付終了時間</param>
	/// <param name="chargeId">固定用アニメ</param>
	/// <returns></returns>
	std::unique_ptr<ActionData> SetActionData(int nextAtkId, float sHitTim = -1.0f, float HitTim = -1.0f,
		float sNewTime = -1.0f, bool isChage = false, int chargeId = -1);
	void SetActionData(int id,const ActionData& data);
};

