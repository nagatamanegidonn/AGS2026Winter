#pragma once
#include "../../Common/Quaternion.h"
#include "../ActorBase.h"
#include "Stage.h"

class Planet : public ActorBase
{

public:

	// 重力の強さ
	static constexpr float DEFAULT_GRAVITY_POW = 25.0f;

	// 重力がかかる範囲
	static constexpr float DEFAULT_GRAVITY_RADIUS = 5000.0f;

	// ゲームオーバー範囲
	static constexpr float DEFAULT_DEAD_LENGTH = 1000.0f;

	// 重力種別
	enum class TYPE
	{
		GROUND,
		SPHERE,
		TRANS_ROT,
		TRANS_CAMERA_FIXED,
		ROAD,
	};

	//ステージエリア
	struct Area
	{
		int areaId = 0;			//多分使わない
		VECTOR pos = { 0.0f,0.0f,0.0f };
		float radius = 10.0f;
	};

	//エリア移動用の位置情報
	struct LerpPos
	{
		int nextId = -1;
		VECTOR pos = { 0.0f,0.0f,0.0f };
	};

	// コンストラクタ
	Planet(const Stage::NAME& name, const TYPE& type, const Transform& transform);

	// デストラクタ
	~Planet(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	
	// 惑星種別
	const TYPE& GetType(void) const;

	// 惑星名
	const Stage::NAME& GetName(void) const;

	// 位置の設定
	void SetPosition(const VECTOR& pos);

	// 回転の設定
	void SetRotation(const Quaternion& rot);

	// エリアの取得・設定
	std::weak_ptr<Area> GetArea(int areaId) const;
	std::map<int, std::shared_ptr<Area>> GetArea(void)const;
	void SetArea(VECTOR pos, float rad,int areaId);
	//posがエリアにいるか
	int CheckAreaId(const VECTOR& pos);
	const bool CheckArea(const VECTOR pos);

	const bool CheckLerpPos(std::string text, int id);
	const VECTOR GetLerpPos(std::string text, int id);

private:

	// 惑星種別
	TYPE type_;

	// 惑星名
	Stage::NAME name_;




	std::map<int,std::shared_ptr<Area>> stageArea_;

	using LerpPosMap = std::map<int, std::shared_ptr<LerpPos>>;
	std::map<std::string, LerpPosMap> lerpPosMap_;

	std::shared_ptr<LerpPos> MakeLerpPos(VECTOR pos, int next = -1);

};
