#pragma once
#include <memory>
#include <map>
#include "../Common/Transform.h"
#include "../ActorBase.h"
class ResourceManager;
class Object;
class Planet;
class Player;
class Boss;

class Stage: 
	public ActorBase
{

public:

	// ステージの切り替え間隔
	static constexpr float TIME_STAGE_CHANGE = 1.0f;

	// ステージ名
	enum class NAME
	{
		MAIN_PLANET,
		FALL_PLANET,
		FLAT_PLANET_BASE,
		FLAT_PLANET_ROT01,
		FLAT_PLANET_ROT02,
		FLAT_PLANET_ROT03,
		FLAT_PLANET_ROT04,
		FLAT_PLANET_FIXED01,
		FLAT_PLANET_FIXED02,
		PLANET10,
		LAST_STAGE,
		SPECIAL_STAGE
	};

	// コンストラクタ
	Stage(Player& player, Boss& boss);

	// デストラクタ
	~Stage(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

	// ステージ変更
	void ChangeStage(NAME type);

	// 対象ステージを取得
	std::weak_ptr<Planet> GetPlanet(NAME type);
	std::weak_ptr<Planet> GetActivePlanet(void);

	void SetPlayers(std::vector < std:: shared_ptr <Player >> players);

	void AddBom(const Transform& _trans);

private:

	// シングルトン参照
	ResourceManager& resMng_;

	std::vector<std::weak_ptr<Player>> players_;
	Player& player_;
	Boss& boss_;

	// ステージアクティブになっている惑星の情報
	NAME activeName_;
	std::weak_ptr<Planet> activePlanet_;

	// 惑星
	std::map<NAME, std::shared_ptr<Planet>> planets_;

	// ワープスター
	std::vector<std::unique_ptr<Object>> objects_;

	// 空のPlanet
	std::shared_ptr<Planet> nullPlanet = nullptr;

	float step_;

	// 最初の惑星
	void MakeMainStage(void);

	// 花の作成
	void MakeFlour(void);


};
