#include <DxLib.h>
#include "../../Application.h"

#include "ItemBase.h"


ItemBase::ItemBase(void)
{
	count_ = 1;
	name_ = L"‰ñ•œ";

	itemImage_ = LoadGraph((Application::PATH_IMAGE + L"Item/items/item489.png").c_str());
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
	DrawFormatString(Application::SCREEN_SIZE_X - 100, Application::SCREEN_SIZE_Y - 100, 0xffffff, L"%d", count_);
}

void ItemBase::Release(void)
{
}
