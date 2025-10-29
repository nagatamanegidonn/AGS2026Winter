#include <vector>
#include <map>
#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/ResourceManager.h"
#include "Object.h"
#include "ItemObject.h"
#include "../Player/Player.h"
#include "../Enemy/Boss.h"
#include "Planet.h"
#include "../Common/Collider/Collider.h"
#include "../Common/Transform.h"
#include "Stage.h"

Stage::Stage(Player& player, Boss& boss)
	: resMng_(ResourceManager::GetInstance()), player_(player), boss_(boss)
{
	activeName_ = NAME::MAIN_PLANET;
	step_ = 0.0f;
}

Stage::~Stage(void)
{
	
	// オブジェクト	
	objects_.clear();
	
	// 惑星
	planets_.clear();

}

void Stage::Init(void)
{
	//
	MakeMainStage();
	//
	MakeFlour();

	step_ = -1.0f;
}

void Stage::Update(void)
{

	// オブジェクト
	for (const auto& s : objects_)
	{
		s->Update();
	}

	// 惑星
	for (const auto& s : planets_)
	{
		s.second->Update();
	}

	//エリアの設定
	for (const auto& s : activePlanet_.lock()->GetArea())
	{
		VECTOR diff;
		float disPow;


		for (auto& player : players_)
		{
			// playerとの衝突判定
			auto& pPos = player.lock()->GetTransform().pos;
			diff = VSub(pPos, s.second->pos);
			disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

			if (disPow < s.second->radius * s.second->radius)
			{
				//プレイヤーのエリア位置設定
				player.lock()->SetAreaId(s.second->areaId);
			}
		}

		// bossとの衝突判定
		auto& ePos = boss_.GetTransform().pos;
		diff = VSub(ePos, s.second->pos);
		disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (disPow < s.second->radius * s.second->radius)
		{
			boss_.SetAreaId(s.second->areaId);
		}
	}
}

void Stage::Draw(void)
{


	// 惑星
	for (const auto& s : planets_)
	{
		s.second->Draw();
	}
	
	// オブジェクト
	for (const auto& s : objects_)
	{
		s->Draw();
	}

}

void Stage::ChangeStage(NAME type)
{

	activeName_ = type;

	// 対象のステージを取得する
	activePlanet_ = GetPlanet(activeName_);


	// ステージの当たり判定をプレイヤーに設定
	player_.ClearCollider();
	player_.AddCollider(activePlanet_.lock()->GetTransform().collider);
	player_.AddCollider(boss_.GetTransform().collider);

	boss_.ClearCollider();
	boss_.AddCollider(activePlanet_.lock()->GetTransform().collider);

	for (const auto& s : objects_)
	{
		if (s->GetTransform().collider != nullptr)
		{
			player_.AddCollider(s->GetTransform().collider);
			boss_.AddCollider(s->GetTransform().collider);
		}
	}

	//エリアの設定
	for (const auto& s : activePlanet_.lock()->GetArea())
	{
		VECTOR diff;
		float disPow;

		for (auto& player : players_)
		{
			// playerとの衝突判定
			auto& pPos = player.lock()->GetTransform().pos;
			diff = VSub(pPos, s.second->pos);
			disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

			if (disPow < s.second->radius * s.second->radius)
			{
				player.lock()->SetAreaId(s.second->areaId);
			}
		}

		// bossとの衝突判定
		auto& ePos = boss_.GetTransform().pos;
		diff = VSub(ePos, s.second->pos);
		disPow = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (disPow < s.second->radius * s.second->radius)
		{
			boss_.SetAreaId(s.second->areaId);
		}
	}

	step_ = TIME_STAGE_CHANGE;

}

std::weak_ptr<Planet> Stage::GetPlanet(NAME type)
{
	if (planets_.count(type) == 0)
	{
		return nullPlanet;
	}

	return planets_[type];
}
std::weak_ptr<Planet> Stage::GetActivePlanet(void)
{
	return activePlanet_;
}

void Stage::SetPlayers(std::vector<std::shared_ptr<Player>> players)
{
	for (auto& player : players)
	{
		players_.push_back(player);
	}
}

void Stage::MakeMainStage(void)
{

	// 最初の惑星
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::PLANE));
	planetTrans.scl = AsoUtility::VECTOR_ONE;
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { 0.0f, -100.0f, 0.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);
	planetTrans.Update();

	NAME name = NAME::MAIN_PLANET;
	std::shared_ptr<Planet> planet =
		std::make_shared<Planet>(
			name, Planet::TYPE::GROUND, planetTrans);
	planet->Init();
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

	Transform trans;
	std::unique_ptr<Object> obj;

	// 周囲の岩作成
	//------------------------------------------------------------------------------
	trans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::ROCKS));
	trans.scl = AsoUtility::VECTOR_ONE;
	trans.quaRot = Quaternion();
	trans.pos = { 0.0f, -100.0f, 0.0f };

	// 当たり判定(コライダ)作成
	trans.MakeCollider(Collider::TYPE::WALL);
	trans.Update();

	obj = std::make_unique<Object>(player_, trans, Object::STATE::PLAY);
	obj->Init();
	objects_.push_back(std::move(obj));
	//------------------------------------------------------------------------------
}
//採取ポイントの作成
void Stage::MakeFlour(void)
{

	Transform trans;
	std::unique_ptr<Object> obj;

	// 花作成
	//------------------------------------------------------------------------------
	trans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLOUR));
	trans.scl = AsoUtility::VECTOR_ONE;
	trans.quaRot = Quaternion();
	trans.pos = { 1800.0f, -335.0f, -4700.0f };

	// 当たり判定(コライダ)作成
	trans.MakeCollider(Collider::TYPE::ITEM, trans.pos, 30.0f);
	trans.Update();

	obj = std::make_unique<ItemObject>(player_, trans, Object::STATE::PLAY,1);
	obj->Init();
	objects_.push_back(std::move(obj));
	//------------------------------------------------------------------------------

	// 花作成
	//------------------------------------------------------------------------------
	trans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLOUR));
	trans.scl = AsoUtility::VECTOR_ONE;
	trans.quaRot = Quaternion();
	trans.pos = { 1900.0f, -335.0f, -5000.0f };

	// 当たり判定(コライダ)作成
	trans.MakeCollider(Collider::TYPE::ITEM, trans.pos, 30.0f);
	trans.Update();

	obj = std::make_unique<ItemObject>(player_, trans, Object::STATE::PLAY,1);
	obj->Init();
	objects_.push_back(std::move(obj));
	//------------------------------------------------------------------------------

	// 花作成
	//------------------------------------------------------------------------------
	trans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::ROCK));
	trans.scl = VScale(AsoUtility::VECTOR_ONE, 0.08f);
	trans.quaRot = Quaternion();
	trans.pos = { 700.0f, -335.0f, -4500.0f };

	// 当たり判定(コライダ)作成
	trans.MakeCollider(Collider::TYPE::ITEM, trans.pos, 30.0f);
	trans.Update();

	obj = std::make_unique<ItemObject>(player_, trans, Object::STATE::PLAY,0);
	obj->Init();
	objects_.push_back(std::move(obj));
	//------------------------------------------------------------------------------
}
