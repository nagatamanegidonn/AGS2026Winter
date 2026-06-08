#pragma once
#include <memory>
#include <map>
#include <string>
#include "Resource.h"

class ResourceManager
{

public:

	// リソース名
	enum class SRC
	{
		TITLE,
		PUSH_SPACE,
		// オブジェクトモデル
		BOSS,			// ボス
		MONSTER,		// 小型モンスター
		PLAYER_NIGHT,	// プレイヤー
		PLAYER_SHADOW,
		// ステージモデル
		SKY_DOME,
		PLANE,
		ROCKS,
		COLL,
		// アイテムオブジェクト
		FLOUR,
		ROCK,
		BOM,
		SMALL_BAG,

		HEEL,

		FOOT_SMOKE,
		// 武器モデル	
		SWORD,
		SWORD2,
		BOW,
		ARROW,

		CLEAR_LOGO,

		ITEM_FREAM,

	};

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static ResourceManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切替時に一旦解放)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	// リソースのロード
	const Resource& Load(SRC src);

	// リソースの複製ロード(モデル用)
	int LoadModelDuplicate(SRC src);

private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理の対象
	std::map<SRC, std::unique_ptr<Resource>> resourcesMap_;

	// 読み込み済みリソース
	std::map<SRC, Resource&> loadedMap_;

	Resource dummy_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(SRC src);

};
