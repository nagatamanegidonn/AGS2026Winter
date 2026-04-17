#include <DxLib.h>
#include "../../Application.h"
#include <typeinfo>

#include "ItemBase.h"



ItemBase::ItemBase(std::wstring _name)
	:
	selectId_(-1),
	itemImage_(-1),
	name_(_name),	// アイテム名を設定
	count_(1)
{

	// アイテム名に応じて画像を読み込む
	if (name_ == L"攻撃") {
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item283.png").c_str());
	}
	else if (name_ == L"回復") {
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item489.png").c_str());
	}
	else if (name_ == L"設置") {
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item345.png").c_str());
	}
	else {
		// "回復" 以外はデフォルトで回復
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item489.png").c_str());
	}
}

ItemBase::~ItemBase(void)
{
	DeleteGraph(itemImage_);
}

void ItemBase::Init( int _id)
{
	selectId_ = _id;
}

void ItemBase::Update(void)
{
}

void ItemBase::Draw(void)
{
	DrawRotaGraph(Application::SCREEN_SIZE_X - 100, Application::SCREEN_SIZE_Y - 100, 8.0f, 0.0f, itemImage_, true);

#ifdef _DEBUG
	DrawFormatString(Application::SCREEN_SIZE_X - 100, Application::SCREEN_SIZE_Y - 100, 0x0000ff, L"%d", count_);
#endif // DEBUG

}

void ItemBase::Release(void)
{
}

bool ItemBase::IsSameItem(const ItemBase& _item) const
{
	return typeid(*this) == typeid(_item);
}
