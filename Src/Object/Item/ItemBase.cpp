

#include <DxLib.h>
#include <typeinfo>

#include "ItemBase.h"
#include "../../Application.h"
#include "../../Manager/ResourceManager.h"

namespace
{
	// アイテム名定義
	const std::wstring ITEM_NAME_ATTACK = L"攻撃";
	const std::wstring ITEM_NAME_RECOVER = L"回復";
	const std::wstring ITEM_NAME_SET = L"設置";
	// アセットパス
	const std::wstring PATH_ITEM_ATTACK = L"Item/items/item283.png";
	const std::wstring PATH_ITEM_RECOVER = L"Item/items/item489.png";
	const std::wstring PATH_ITEM_SET = L"Item/items/item345.png";
	// UI配置・描画パラメータ
	constexpr int UI_OFFSET_X = 100;		// 画面右端からのオフセット
	constexpr int UI_OFFSET_Y = 100;		// 画面下端からのオフセット
	constexpr float ITEM_UI_SCALE = 8.0f;	// アイテムアイコンの描画拡大率
	constexpr float ITEM_UI_ROT = 0.0f;		// 描画回転角
	// 初期値・その他数値
	constexpr int INITIAL_COUNT_ONE = 1;
	constexpr int INVALID_HANDLE_ID = -1;
	// デバッグ表示用設定 
#ifdef _DEBUG
	const std::wstring DBG_FMT_COUNT = L"%d";
	constexpr unsigned int COLOR_BLUE = 0x0000ff;
#endif
}

ItemBase::ItemBase(std::wstring _name)
	:
	selectId_(INVALID_HANDLE_ID),
	itemImage_(INVALID_HANDLE_ID),
	name_(_name),	// アイテム名を設定
	count_(INITIAL_COUNT_ONE)
{
	// アイテム名に応じて画像を読み込む
	if (name_ == ITEM_NAME_ATTACK) {
		itemImage_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::FLASH_ITEM).handleId_;
	}
	else if (name_ == ITEM_NAME_RECOVER) {
		itemImage_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::HEEL_ITEM).handleId_;
	}
	else if (name_ == ITEM_NAME_SET) {
		itemImage_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::BOM_ITEM).handleId_;
	}
	else {
		// 指定名以外はデフォルトで回復アイコンを割り当て
		itemImage_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::HEEL_ITEM).handleId_;
	}
}

ItemBase::~ItemBase(void)
{
	//DeleteGraph(itemImage_);
}

void ItemBase::Init(int _id)
{
	selectId_ = _id;
}

void ItemBase::Update(void)
{
}

void ItemBase::Draw(void)
{
	// アイテムアイコンを画面右下に拡大描画
	DrawRotaGraph(Application::SCREEN_SIZE_X - UI_OFFSET_X, Application::SCREEN_SIZE_Y - UI_OFFSET_Y,
		ITEM_UI_SCALE, ITEM_UI_ROT, itemImage_, true);

#ifdef _DEBUG
	DrawFormatString(Application::SCREEN_SIZE_X - UI_OFFSET_X, Application::SCREEN_SIZE_Y - UI_OFFSET_Y,
		COLOR_BLUE, DBG_FMT_COUNT.c_str(), count_);
#endif // DEBUG
}

void ItemBase::Release(void)
{
}

bool ItemBase::IsSameItem(const ItemBase& _item) const
{
	return typeid(*this) == typeid(_item);
}