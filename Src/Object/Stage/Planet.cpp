#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/SceneManager.h"
#include "../Common/Transform.h"
#include "Planet.h"

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
	SetArea({ 1200.0f,0.0f,-5000.0f }, 1000.0f, 0);//ベースキャンプ
	SetArea({ 0.0f,0.0f,0.0f }, 3500.0f, 1);	   //エリア１
	SetArea({ 5500.0f,0.0f,4250.0f }, 3000.0f, 2); //エリア２

	lerpPosMap_ = {
		// エリア１からエリア２へ
	{"1to2",{
		{0,{MakeLerpPos({ 1600.0f,0.0f,  1800.0f},0)}},
		{1,{MakeLerpPos({ 2700.0f,0.0f,3600.0f},1)}},
		{2,{MakeLerpPos({ 4300.0f,0.0f,4300.0f},2)}},
		{3,{MakeLerpPos({ 5500.0f,0.0f,4250.0f },3)}}
	}},
		// エリア２からエリア１へ
	{"2to1",{
		{0,{MakeLerpPos({ 4300.0f,0.0f,4300.0f},0)}},
		{1,{MakeLerpPos({ 2700.0f,0.0f,3600.0f},1)}},
		{2,{MakeLerpPos({ 1600.0f,0.0f,  1800.0f},2)}},
		{3,{MakeLerpPos({ 0.0f,0.0f,0.0f },3)}}
	}},
		// エリア１の位置
	{"1",{
		{0,{MakeLerpPos({ 0.0f,0.0f,0.0f},0)}}
	}},
		// エリア２の位置
	{"2",{
		{0,{MakeLerpPos({5500.0f,0.0f,4250.0f},0)}}
	}}
	};
}

void Planet::Update(void)
{
}

void Planet::Draw(void)
{
    MV1DrawModel(transform_.modelId);
	//std::lerp()

#ifdef _DEBUG
	DrawSphere3D({1200.0f,0.0f,-5000.0f}, 1000.0f, 10, 0xff0000, 0xff0000, false);
	DrawSphere3D({0.0f,0.0f,0.0f}, 3500.0f, 10, 0xff0000, 0xff0000, false);
	DrawSphere3D({5500.0f,0.0f,4250.0f}, 3000.0f, 10, 0xff0000, 0xff0000, false);
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
std::map<int, std::shared_ptr<Planet::Area>> Planet::GetArea(void) const
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
	//エリアの設定
	for (const auto& s : stageArea_)
	{

		// playerとの衝突判定
		VECTOR diff = VSub(pos, s.second->pos);
		float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (disPow < s.second->radius * s.second->radius)
		{
			return s.second->areaId;
		}

	}
	//エリアに入っていなかったら―１を返す
	return -1;
}

const bool Planet::CheckArea(const VECTOR pos)
{
	//エリアの設定
	for (const auto& s : stageArea_)
	{

		// playerとの衝突判定
		VECTOR diff = VSub(pos, s.second->pos);
		float disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (disPow < s.second->radius * s.second->radius)
		{
			return true;
		}

	}
	//エリアに入っていなかったら―１を返す
	return false;
}

const bool Planet::CheckLerpPos(std::string text, int id)
{
	auto it = lerpPosMap_.find(text);
	if (it == lerpPosMap_.end()) {
		return false; // キーがない場合のデフォルト
	}

	const auto& innerMap = it->second;
	auto innerIt = innerMap.find(id);
	if (innerIt == innerMap.end() || !innerIt->second) {
		return false; // IDがない or nullポインタ
	}

	return true; // 正常に取得
}
const VECTOR Planet::GetLerpPos(std::string text, int id)
{
	auto it = lerpPosMap_.find(text);
	if (it == lerpPosMap_.end()) {
		return AsoUtility::VECTOR_ZERO; // キーがない場合のデフォルト
	}

	const auto& innerMap = it->second;
	auto innerIt = innerMap.find(id);
	if (innerIt == innerMap.end() || !innerIt->second) {
		return AsoUtility::VECTOR_ZERO; // IDがない or nullポインタ
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
