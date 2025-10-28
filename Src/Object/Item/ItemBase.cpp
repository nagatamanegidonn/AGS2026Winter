#include <DxLib.h>
#include "../../Application.h"

#include "ItemBase.h"



ItemBase::ItemBase(int itemId)
{
	count_ = 1;
	switch (itemId)
	{
	case 0:
		name_ = L"çUåÇ";
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item283.png").c_str());
		break;
	case 1:
		name_ = L"âÒïú";
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item489.png").c_str());
		break;
	default:
		name_ = L"âÒïú";
		itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item489.png").c_str());
		break;
	}
}

ItemBase::~ItemBase(void)
{
	DeleteGraph(itemImage_);
}

void ItemBase::Init(void)
{
}

void ItemBase::Update(void)
{
}

void ItemBase::Draw(void)
{
	DrawRotaGraph(Application::SCREEN_SIZE_X - 100, Application::SCREEN_SIZE_Y - 100, 8.0f, 0.0f, itemImage_, true);
	DrawFormatString(Application::SCREEN_SIZE_X - 100, Application::SCREEN_SIZE_Y - 100, 0x0000ff, L"%d", count_);
}

void ItemBase::Release(void)
{
}
