#include "ItemBase.h"
#include "ItemPoach.h"
#include "../../Application.h"
#include <DxLib.h>

ItemPoach::ItemPoach()
	:
	slectId_(-1),
	Items_(),
	ItemIds_()
{
}

ItemPoach::~ItemPoach()
{
}

void ItemPoach::Draw(int i)
{
	auto it = Items_.find(slectId_);
	if (it != Items_.end()) {
		// 存在する
		Items_[slectId_]->Draw();
		DrawFormatString(Application::SCREEN_SIZE_X - 200, Application::SCREEN_SIZE_Y - 80, 0x0000ff, L"選択中のID=%d", slectId_);
	}
	else {
		// 存在しない
	}
}

//	ここもintではなくItemBaseを引数にする
void ItemPoach::AddItem(int _itemId)
{
	
	auto it = Items_.find(_itemId);
	if (it != Items_.end()) {
		// 存在する
		Items_[_itemId]->Count(1);
	}
	else {
		// 存在しない
		// 
		// ここを変更->ItemBaseを継承させアイテムを作る
		auto item = std::make_shared<ItemBase>(_itemId);
		Items_.emplace(_itemId, std::move(item));

		ItemIds_.push_back(_itemId);
		// 最初のアイテムなら選択中にする
		if (ItemIds_.size() == 1)
		{
			slectId_ = _itemId;
		}
	}
}

void ItemPoach::PlayItem(int _itemId)
{
	auto it = Items_.find(slectId_);
	if (it != Items_.end()) {
		// 存在する
		Items_[slectId_]->Count(-1);
		if (Items_[slectId_]->GetCount() <= 0)
		{
			// map から削除する
			Items_.erase(it);
			// list からも削除する
			// itemIdは値が被らないので可能
			ItemIds_.remove(slectId_);
			CountSelectId();
		}
	}
}
void ItemPoach::CountSelectId(void)
{
	if(ItemIds_.size() == 0)
	{
		slectId_ = -1;
		return;
	}
	slectId_++;
	if (slectId_ + 1 > ItemIds_.size())
	{
		slectId_ = ItemIds_.front();
	}
}
