#include <DxLib.h>
#include "../../Utility/Utility.h"
#include "../../Manager/SceneManager.h"
#include "../Common/Transform.h"
#include "Planet.h"

namespace
{
	// エリア中心座標
	constexpr VECTOR AREA_POS_0 = { 1200.0f,  0.0f, -5000.0f }; // ベースキャンプ
	constexpr VECTOR AREA_POS_1 = { 0.0f,     0.0f,     0.0f }; // エリア１
	constexpr VECTOR AREA_POS_2 = { 5500.0f,  0.0f,  4250.0f }; // エリア２
	// エリア判定半径 (境界球) 
	constexpr float AREA_RAD_0 = 1000.0f;
	constexpr float AREA_RAD_1 = 3500.0f;
	constexpr float AREA_RAD_2 = 3000.0f;
	// エリア間移動の中継座標
	constexpr VECTOR AREA_MOVE_POS_1to2_0 = { 1600.0f, 0.0f, 1800.0f };
	constexpr VECTOR AREA_MOVE_POS_1to2_1 = { 2700.0f, 0.0f, 3600.0f };
	constexpr VECTOR AREA_MOVE_POS_1to2_2 = { 4300.0f, 0.0f, 4300.0f };
	// エリア検索・マップキー用文字列
	const std::string KEY_ROUTE_1_TO_2 = "1to2";
	const std::string KEY_ROUTE_2_TO_1 = "2to1";
	const std::string KEY_AREA_1 = "1";
	const std::string KEY_AREA_2 = "2";
	// エリアID定義
	constexpr int AREA_ID_0 = 0;
	constexpr int AREA_ID_1 = 1;
	constexpr int AREA_ID_2 = 2;
	constexpr int INVALID_AREA = -1; // どこのエリアにも属していない場合
	// デバッグ表示設定
#ifdef _DEBUG
	constexpr unsigned int COLOR_RED = 0xff0000;
	constexpr int DBG_SPHERE_DIV_NUM = 10;       // 球体描画の細かさ
#endif
}

Planet::Planet(const Stage::NAME& name, const TYPE& type, const Transform& transform)
{
	name_ = name;
	type_ = type;
	transform_ = transform;
}

Planet::~Planet(void)
{
	lerpPosMap_.clear();
}

void Planet::Init(void)
{
	SetArea(AREA_POS_0, AREA_RAD_0, AREA_ID_0);	// ベースキャンプ
	SetArea(AREA_POS_1, AREA_RAD_1, AREA_ID_1);	// エリア１
	SetArea(AREA_POS_2, AREA_RAD_2, AREA_ID_2);	// エリア２

	lerpPosMap_ = {
		// エリア１からエリア２へ
	{KEY_ROUTE_1_TO_2,{
		{0,{MakeLerpPos(AREA_MOVE_POS_1to2_0,0)}},
		{1,{MakeLerpPos(AREA_MOVE_POS_1to2_1,1)}},
		{2,{MakeLerpPos(AREA_MOVE_POS_1to2_2,2)}},
		{3,{MakeLerpPos(AREA_POS_2,3)}}
	}},
		// エリア２からエリア１へ
	{KEY_ROUTE_2_TO_1,{
		{0,{MakeLerpPos(AREA_MOVE_POS_1to2_2,0)}},
		{1,{MakeLerpPos(AREA_MOVE_POS_1to2_1,1)}},
		{2,{MakeLerpPos(AREA_MOVE_POS_1to2_0,2)}},
		{3,{MakeLerpPos(AREA_POS_1,3)}}
	}},
		// エリア１の位置
	{KEY_AREA_1,{
		{0,{MakeLerpPos(AREA_POS_1,0)}}
	}},
		// エリア２の位置
	{KEY_AREA_2,{
		{0,{MakeLerpPos(AREA_POS_2,0)}}
	}}
	};
}

void Planet::Update(void)
{
}

void Planet::Draw(void)
{
    MV1DrawModel(transform_.modelId);

#ifdef _DEBUG
	DrawSphere3D(AREA_POS_0, AREA_RAD_0, DBG_SPHERE_DIV_NUM, COLOR_RED, COLOR_RED, false);
	DrawSphere3D(AREA_POS_1, AREA_RAD_1, DBG_SPHERE_DIV_NUM, COLOR_RED, COLOR_RED, false);
	DrawSphere3D(AREA_POS_2, AREA_RAD_2, DBG_SPHERE_DIV_NUM, COLOR_RED, COLOR_RED, false);
#endif
}

void Planet::SetPosition(const VECTOR& pos)
{
    transform_.pos = pos;
    transform_.Update();
}

void Planet::SetRotation(const Quaternion& rot)
{
	transform_.quaRot = rot;
	transform_.Update();
}

std::weak_ptr<Planet::Area> Planet::GetArea(int areaId) const
{
	return stageArea_.at(areaId);
}

const std::map<int, std::shared_ptr<Planet::Area>>& Planet::GetArea(void) const
{
	return stageArea_;
}

void Planet::SetArea(VECTOR pos, float rad, int areaId)
{
	const auto area = std::make_shared<Area>();
	area->areaId = areaId;
	area->pos = pos;
	area->radius = rad;

	stageArea_.emplace(areaId, area);
}

int Planet::CheckAreaId(const VECTOR& pos)
{
	// エリアの設定
	for (const auto& s : stageArea_)
	{
		// プレイヤーとの衝突判定
		VECTOR diff = VSub(pos, s.second->pos);
		float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (disPow < s.second->radius * s.second->radius)
		{
			return s.second->areaId;
		}
	}
	// エリアに入っていなかったら―１を返す
	return -1;
}

bool Planet::CheckArea(const VECTOR pos)
{
	// エリアの設定
	for (const auto& s : stageArea_)
	{
		// プレイヤーとの衝突判定
		VECTOR diff = VSub(pos, s.second->pos);
		float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (disPow < s.second->radius * s.second->radius)
		{
			return true;
		}
	}
	// エリアに入っていなかったら―１を返す
	return false;
}

bool Planet::CheckLerpPos(std::string text, int id)
{
	auto it = lerpPosMap_.find(text);
	if (it == lerpPosMap_.end())
	{
		return false; // キーがない場合のデフォルト
	}

	const auto& innerMap = it->second;
	auto innerIt = innerMap.find(id);
	if (innerIt == innerMap.end() || !innerIt->second)
	{
		return false; // IDがない or nullポインタ
	}

	return true; // 正常に取得
}

const VECTOR& Planet::GetLerpPos(std::string text, int id) const
{
	auto it = lerpPosMap_.find(text);
	if (it == lerpPosMap_.end())
	{
		return Utility::VECTOR_ZERO; // キーがない場合のデフォルト
	}

	const auto& innerMap = it->second;
	auto innerIt = innerMap.find(id);
	if (innerIt == innerMap.end() || !innerIt->second)
	{
		return Utility::VECTOR_ZERO; // IDがない or nullポインタ
	}

	return innerIt->second->pos; // 正常に取得
}

std::shared_ptr< Planet::LerpPos> Planet::MakeLerpPos(VECTOR pos, int next)
{
	auto Lpos = std::make_shared<LerpPos>(); // メモリ確保
	Lpos->pos = pos;
	Lpos->nextId = next;

	return Lpos;
}

const Planet::TYPE& Planet::GetType(void) const
{
	return type_;
}

const Stage::NAME& Planet::GetName(void) const
{
	return name_;
}
