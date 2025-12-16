#include <DxLib.h>
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{

	// 推奨しませんが、どうしても使いたい方は
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::wstring PATH_IMG = Application::PATH_IMAGE;
	static std::wstring PATH_MDL = Application::PATH_MODEL;
	static std::wstring PATH_EFF = Application::PATH_EFFECT;

	std::unique_ptr<Resource> res;

	// タイトル画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + L"TitleRogo.png");
	resourcesMap_.emplace(SRC::TITLE, std::move(res));

	// PushSpace
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + L"PushSpace.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	// ボス
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Enemy/Boss/Boss.mv1");
	resourcesMap_.emplace(SRC::BOSS, std::move(res));
	// 小型モンスター
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Enemy/Monster/SmallMonster.mv1");
	resourcesMap_.emplace(SRC::MONSTER, std::move(res));
	
	// プレイヤー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Player2/Player2.mv1");
	resourcesMap_.emplace(SRC::PLAYER_NIGHT, std::move(res));

	// プレイヤー影
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + L"Shadow.png");
	resourcesMap_.emplace(SRC::PLAYER_SHADOW, std::move(res));

	// スカイドーム
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"SkyDome/SkyDome.mv1");
	resourcesMap_.emplace(SRC::SKY_DOME, std::move(res));

	// 床モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Planet/Plane.mv1");
	resourcesMap_.emplace(SRC::PLANE, std::move(res));
	// 岩モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Planet/Rocks.mv1");
	resourcesMap_.emplace(SRC::ROCKS, std::move(res));
	// 岩モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Planet/Collider.mv1");
	resourcesMap_.emplace(SRC::COLL, std::move(res));

	// 花モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"ItemObject/flour/Chamomile.mv1");
	resourcesMap_.emplace(SRC::FLOUR, std::move(res));
	// 岩モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"ItemObject/Rock/rock1_LOD0.mv1");
	resourcesMap_.emplace(SRC::ROCK, std::move(res));
	// 爆弾モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"ItemObject/Bom/Barrel Dry Highpoly.mv1");
	resourcesMap_.emplace(SRC::BOM, std::move(res));
	// 袋モデル
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"ItemObject/Flash/IncaProps_Bag.mv1");
	resourcesMap_.emplace(SRC::SMALL_BAG, std::move(res));
	
	// ヒールアイテム画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_MDL + L"Item/items/item489.png");
	resourcesMap_.emplace(SRC::HEEL, std::move(res));
	
	// 足煙
	res = std::make_unique<RES>(RES_T::EFFEKSEER, PATH_EFF + L"Smoke/Smoke.efkefc");
	resourcesMap_.emplace(SRC::FOOT_SMOKE, std::move(res));

	// 武器オブジェクト
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Sword/Sword.mv1");
	resourcesMap_.emplace(SRC::SWORD, std::move(res));
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Sword/Sword Two-Hander Base.mv1");
	resourcesMap_.emplace(SRC::SWORD2, std::move(res));

	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Weapon/Bow/Bow.mv1");
	resourcesMap_.emplace(SRC::BOW, std::move(res));	
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + L"Weapon/Bow/Arrow.mv1");
	resourcesMap_.emplace(SRC::ARROW, std::move(res));

	// Clear
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + L"Quest/Clear.png");
	resourcesMap_.emplace(SRC::CLEAR_LOGO, std::move(res));

	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + L"Item/ItemFream.png");
	resourcesMap_.emplace(SRC::ITEM_FREAM, std::move(res));

}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{

	// ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return lPair->second;
	}

	// リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		// 登録されていない
		return dummy_;
	}

	// ロード処理
	rPair->second->Load();

	// 念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;

}
